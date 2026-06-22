<?php
/**
 * @file
 * Rendering callbacks for Chart.js library.
 */

/**
 * Render a chart using Chart.js.
 */
function _charts_chartjs_render($element) {
  // Determine chart type.
  $chart_type = $element['#chart_type'];
  $chart_type_info = chart_get_type($chart_type);
  $single_axis = !empty($chart_type_info['axis']) && $chart_type_info['axis'] === CHARTS_SINGLE_AXIS;
  $chartjs_type = _charts_chartjs_type($chart_type);
  $chart_definition = array(
    'type' => $chartjs_type,
    'data' => array(
      'labels' => array(),
      'datasets' => array(),
    ),
    'options' => array(
      'responsive' => TRUE,
      'maintainAspectRatio' => TRUE,
      'plugins' => array(
        'title' => array(
          'display' => !empty($element['#title']),
          'text' => $element['#title'],
          'color' => !empty($element['#title_color']) ? $element['#title_color'] : NULL,
        ),
        'legend' => array(
          'display' => (bool) $element['#legend'],
          'position' => $element['#legend_position'] ? $element['#legend_position'] : 'right',
          'labels' => array(
            'color' => !empty($element['#legend_font_color']) ? $element['#legend_font_color'] : (!empty($element['#legend_labels_color']) ? $element['#legend_labels_color'] : NULL),
          ),
        ),
        'tooltip' => array(
          'enabled' => (bool) $element['#tooltips'],
        ),
      ),
    ),
  );

  if ($chart_type === 'bar') {
    $chart_definition['options']['indexAxis'] = 'y';
  }
  if (!empty($element['#stacking'])) {
    $chart_definition['options']['scales']['x']['stacked'] = TRUE;
    $chart_definition['options']['scales']['y']['stacked'] = TRUE;
  }

  // Handle axes and labels.
  foreach (element_children($element) as $key) {
    if ($element[$key]['#type'] === 'chart_xaxis' || $element[$key]['#type'] === 'chart_yaxis') {
      $axis_type = ($element[$key]['#type'] === 'chart_xaxis') ? 'x' : 'y';

      // Use labels from the axis if present.
      if (!empty($element[$key]['#labels'])) {
        $chart_definition['data']['labels'] = $element[$key]['#labels'];
      }

      // If this is a horizontal bar chart, we need to ensure the labels go to the Y-axis scale if that's where they belong.
      // However, Chart.js global labels usually work.

      if (!empty($element[$key]['#axis_type'])) {
        $chart_definition['options']['scales'][$axis_type]['type'] = ($element[$key]['#axis_type'] === 'labels') ? 'category' : $element[$key]['#axis_type'];
      }

      if (!empty($element[$key]['#title'])) {
        $chart_definition['options']['scales'][$axis_type]['title']['display'] = TRUE;
        $chart_definition['options']['scales'][$axis_type]['title']['text'] = $element[$key]['#title'];
      }
      if (!empty($element[$key]['#labels_color'])) {
        $chart_definition['options']['scales'][$axis_type]['ticks']['color'] = $element[$key]['#labels_color'];
      }
      if (!empty($element[$key]['#title_color'])) {
        $chart_definition['options']['scales'][$axis_type]['title']['color'] = $element[$key]['#title_color'];
      }

      if (isset($element[$key]['#min']) && $element[$key]['#min'] !== '') {
        $chart_definition['options']['scales'][$axis_type]['min'] = (float) $element[$key]['#min'];
      }
      if (isset($element[$key]['#max']) && $element[$key]['#max'] !== '') {
        $chart_definition['options']['scales'][$axis_type]['max'] = (float) $element[$key]['#max'];
      }

      // Apply prefix/suffix to ticks.
      $prefix = isset($element[$key]['#prefix']) ? $element[$key]['#prefix'] : '';
      $suffix = isset($element[$key]['#suffix']) ? $element[$key]['#suffix'] : '';
      if ($prefix !== '' || $suffix !== '') {
        $chart_definition['options']['scales'][$axis_type]['ticks']['callback'] = '###TICK_CALLBACK###';
        $chart_definition['options']['scales'][$axis_type]['ticks']['_prefix'] = $prefix;
        $chart_definition['options']['scales'][$axis_type]['ticks']['_suffix'] = $suffix;
      }
    }
    elseif ($element[$key]['#type'] === 'chart_data') {
      $dataset_data = $element[$key]['#data'];
      if ($single_axis) {
        $labels = array();
        $values = array();
        foreach ($dataset_data as $delta => $point) {
          if (is_array($point)) {
            $labels[] = isset($point[0]) ? $point[0] : $delta;
            $values[] = isset($point[1]) ? $point[1] : NULL;
          }
          else {
            $labels[] = $delta;
            $values[] = $point;
          }
        }
        if (!empty($labels)) {
          $chart_definition['data']['labels'] = $labels;
        }
        $dataset_data = $values;
      }
      $series_chart_type = !empty($element[$key]['#chart_type']) ? $element[$key]['#chart_type'] : $chart_type;
      $series_chartjs_type = _charts_chartjs_type($series_chart_type);

      if (!$single_axis && ($series_chartjs_type === 'scatter' || $series_chartjs_type === 'bubble')) {
        $normalized = array();
        foreach ($dataset_data as $delta => $point) {
          if (is_array($point)) {
            $normalized[] = array(
              'x' => isset($point[0]) ? $point[0] : $delta,
              'y' => isset($point[1]) ? $point[1] : NULL,
            );
          }
          else {
            $normalized[] = array(
              'x' => $delta,
              'y' => $point,
            );
          }
        }
        $dataset_data = $normalized;
      }

      $dataset = array(
        'label' => $element[$key]['#title'],
        'data' => $dataset_data,
      );
      if ($series_chartjs_type !== $chart_definition['type']) {
        $dataset['type'] = $series_chartjs_type;
      }
      if (!empty($element[$key]['#color'])) {
        $dataset['backgroundColor'] = $element[$key]['#color'];
        $dataset['borderColor'] = $element[$key]['#color'];
      }

      if ($series_chart_type === 'area') {
        $dataset['fill'] = TRUE;
      }
      if ($series_chart_type === 'spline') {
        $dataset['tension'] = 0.4;
      }

      $chart_definition['data']['datasets'][] = $dataset;
    }
  }

  // Add the library and class.
  $element['#attached']['library'][] = array('charts_chartjs', 'chartjs');
  $element['#attributes']['class'][] = 'charts-chartjs';

  // When explicit dimensions are provided, allow flexible aspect ratio and set
  // wrapper dimensions so Chart.js has a measurable render area.
  if (!empty($element['#width']) || !empty($element['#height'])) {
    $chart_definition['options']['maintainAspectRatio'] = FALSE;
    $style = isset($element['#attributes']['style']) ? rtrim($element['#attributes']['style']) : '';
    if (!empty($element['#width'])) {
      $style .= ($style ? ' ' : '') . 'width:' . $element['#width'] . 'px;';
    }
    if (!empty($element['#height'])) {
      $style .= ($style ? ' ' : '') . 'height:' . $element['#height'] . 'px;';
    }
    $element['#attributes']['style'] = $style;
  }

  // Apply background color to the wrapper.
  if (!empty($element['#background']) && $element['#background'] !== 'transparent') {
    $existing_style = isset($element['#attributes']['style']) ? $element['#attributes']['style'] : '';
    $element['#attributes']['style'] = $existing_style . 'background-color: ' . $element['#background'] . ';';
  }

  if (!empty($element['#raw_options'])) {
    $chart_definition = backdrop_array_merge_deep($chart_definition, $element['#raw_options']);
  }

  $element['#chart_definition'] = $chart_definition;

  return $element;
}

/**
 * Maps Charts chart types to Chart.js chart types.
 */
function _charts_chartjs_type($chart_type) {
  $type_map = array(
    'area' => 'line',
    'column' => 'bar',
    'donut' => 'doughnut',
    'polarArea' => 'polarArea',
    'spline' => 'line',
  );

  return isset($type_map[$chart_type]) ? $type_map[$chart_type] : $chart_type;
}
