<?php
/**
 * @file
 * Callbacks and utility functions for rendering C3 charts.
 */

/**
 * Chart render callback.
 */
function _charts_c3_render($chart) {
  if (!isset($chart['#id'])) {
    $chart['#id'] = backdrop_html_id('c3-chart-render');
  }

  $chart_definition = _charts_c3_build_definition($chart);

  if (!empty($chart['#background']) && $chart['#background'] !== 'transparent') {
    $existing_style = isset($chart['#attributes']['style']) ? $chart['#attributes']['style'] : '';
    $chart['#attributes']['style'] = $existing_style . 'background-color:' . $chart['#background'] . ';';
  }

  $chart['#attached']['library'][] = array('charts_c3', 'charts_c3');
  $chart['#attributes']['class'][] = 'charts-c3';
  $chart['#chart_definition'] = $chart_definition;

  return $chart;
}

/**
 * Builds a C3 chart definition from a chart render array.
 */
function _charts_c3_build_definition(array &$chart) {
  $chart_type = !empty($chart['#chart_type']) ? $chart['#chart_type'] : 'line';
  $c3_type = _charts_c3_get_type($chart_type);
  $chart_type_info = chart_get_type($chart_type);
  $single_axis = !empty($chart_type_info['axis']) && $chart_type_info['axis'] === CHARTS_SINGLE_AXIS;

  $definition = array(
    'bindto' => '#' . $chart['#id'],
    'data' => array(
      'type' => $c3_type,
      'columns' => array(),
    ),
    'legend' => array(
      'show' => !empty($chart['#legend']),
      'position' => _charts_c3_legend_position(!empty($chart['#legend_position']) ? $chart['#legend_position'] : ''),
    ),
    'tooltip' => array(
      'show' => !empty($chart['#tooltips']),
    ),
    'color' => array(
      'pattern' => !empty($chart['#colors']) ? array_values($chart['#colors']) : array(),
    ),
  );

  if (!empty($chart['#title'])) {
    $definition['title']['text'] = $chart['#title'];
  }
  if (!empty($chart['#data_labels'])) {
    $definition['data']['labels'] = TRUE;
  }
  if (!empty($chart['#width']) || !empty($chart['#height'])) {
    $definition['size'] = array();
    if (!empty($chart['#width'])) {
      $definition['size']['width'] = is_numeric($chart['#width']) ? (int) $chart['#width'] : $chart['#width'];
    }
    if (!empty($chart['#height'])) {
      $definition['size']['height'] = is_numeric($chart['#height']) ? (int) $chart['#height'] : $chart['#height'];
    }
  }

  $xaxis = array();
  $yaxis = array();
  $stackable_series = array();

  foreach (element_children($chart) as $key) {
    if ($chart[$key]['#type'] === 'chart_xaxis') {
      if (empty($chart[$key]['#defaults_loaded'])) {
        $chart[$key] += element_info('chart_xaxis');
      }
      $xaxis = $chart[$key];
    }
    elseif ($chart[$key]['#type'] === 'chart_yaxis') {
      if (empty($chart[$key]['#defaults_loaded'])) {
        $chart[$key] += element_info('chart_yaxis');
      }
      if (!$yaxis) {
        $yaxis = $chart[$key];
      }
    }
  }

  if ($single_axis) {
    foreach (element_children($chart) as $key) {
      if ($chart[$key]['#type'] !== 'chart_data') {
        continue;
      }
      if (empty($chart[$key]['#defaults_loaded'])) {
        $chart[$key] += element_info('chart_data');
      }

      $series = $chart[$key];
      $series_title = !empty($series['#title']) ? $series['#title'] : t('Series');
      $series_labels = !empty($series['#labels']) ? $series['#labels'] : (!empty($chart['#labels']) ? $chart['#labels'] : array());

      if ($chart_type === 'gauge') {
        $gauge_value = NULL;
        foreach ($series['#data'] as $point) {
          $gauge_value = is_array($point) ? (isset($point[1]) ? $point[1] : NULL) : $point;
          if ($gauge_value !== NULL && $gauge_value !== '') {
            break;
          }
        }
        if ($gauge_value !== NULL && $gauge_value !== '') {
          $definition['data']['columns'][] = array($series_title, $gauge_value);
        }
        continue;
      }

      foreach ($series['#data'] as $delta => $point) {
        if (is_array($point)) {
          $label = isset($point[0]) ? $point[0] : (isset($series_labels[$delta]) ? $series_labels[$delta] : $series_title . ' ' . ($delta + 1));
          $value = isset($point[1]) ? $point[1] : NULL;
        }
        else {
          $label = isset($series_labels[$delta]) ? $series_labels[$delta] : $series_title . ' ' . ($delta + 1);
          $value = $point;
        }

        $label = (string) $label;
        $definition['data']['columns'][] = array($label, $value);
        if (!empty($series['#color'])) {
          $definition['data']['colors'][$label] = $series['#color'];
        }
      }
    }
  }
  else {
    $categories = !empty($xaxis['#labels']) ? array_values($xaxis['#labels']) : array();
    foreach (element_children($chart) as $key) {
      if ($chart[$key]['#type'] !== 'chart_data') {
        continue;
      }
      if (empty($chart[$key]['#defaults_loaded'])) {
        $chart[$key] += element_info('chart_data');
      }

      $series = $chart[$key];
      $series_name = !empty($series['#title']) ? $series['#title'] : $key;
      $stackable_series[] = $series_name;

      if ($c3_type === 'scatter') {
        $x_series_name = $series_name . '__x';
        $x_column = array($x_series_name);
        $y_column = array($series_name);
        foreach ($series['#data'] as $delta => $point) {
          if (is_array($point)) {
            $x_column[] = isset($point[0]) ? $point[0] : $delta;
            $y_column[] = isset($point[1]) ? $point[1] : NULL;
          }
          else {
            $x_column[] = $delta;
            $y_column[] = $point;
          }
        }
        $definition['data']['xs'][$series_name] = $x_series_name;
        $definition['data']['columns'][] = $x_column;
        $definition['data']['columns'][] = $y_column;
      }
      else {
        $column = array($series_name);
        foreach ($series['#data'] as $point) {
          $column[] = is_array($point) ? (isset($point[1]) ? $point[1] : NULL) : $point;
        }
        $definition['data']['columns'][] = $column;
      }

      if (!empty($series['#color'])) {
        $definition['data']['colors'][$series_name] = $series['#color'];
      }
    }

    if (!empty($chart['#stacking']) && count($stackable_series) > 1 && $c3_type !== 'scatter') {
      $definition['data']['groups'][] = $stackable_series;
    }

    $definition['axis']['x'] = array(
      'type' => $c3_type === 'scatter' ? 'indexed' : 'category',
      'categories' => $c3_type === 'scatter' ? NULL : $categories,
    );
    if (!empty($xaxis['#title'])) {
      $definition['axis']['x']['label'] = array('text' => $xaxis['#title']);
    }
    if (!empty($xaxis['#labels_rotation'])) {
      $definition['axis']['x']['tick']['rotate'] = (int) $xaxis['#labels_rotation'];
    }

    $definition['axis']['y'] = array();
    if (!empty($yaxis['#title'])) {
      $definition['axis']['y']['label'] = array('text' => $yaxis['#title']);
    }
    if (isset($yaxis['#min']) && $yaxis['#min'] !== '' && $yaxis['#min'] !== NULL) {
      $definition['axis']['y']['min'] = is_numeric($yaxis['#min']) ? (float) $yaxis['#min'] : $yaxis['#min'];
    }
    if (isset($yaxis['#max']) && $yaxis['#max'] !== '' && $yaxis['#max'] !== NULL) {
      $definition['axis']['y']['max'] = is_numeric($yaxis['#max']) ? (float) $yaxis['#max'] : $yaxis['#max'];
    }

    if ($chart_type === 'bar') {
      $definition['axis']['rotated'] = TRUE;
    }
  }

  if ($chart_type === 'gauge') {
    $definition['gauge'] = array();
    if (isset($yaxis['#min']) && $yaxis['#min'] !== '' && $yaxis['#min'] !== NULL) {
      $definition['gauge']['min'] = is_numeric($yaxis['#min']) ? (float) $yaxis['#min'] : $yaxis['#min'];
    }
    if (isset($yaxis['#max']) && $yaxis['#max'] !== '' && $yaxis['#max'] !== NULL) {
      $definition['gauge']['max'] = is_numeric($yaxis['#max']) ? (float) $yaxis['#max'] : $yaxis['#max'];
    }
  }

  if (!empty($chart['#raw_options'])) {
    $definition = backdrop_array_merge_deep($definition, $chart['#raw_options']);
  }

  charts_trim_array($definition);
  return $definition;
}

/**
 * Maps Charts chart types to C3 chart types.
 */
function _charts_c3_get_type($chart_type) {
  $type_map = array(
    'column' => 'bar',
    'bubble' => 'scatter',
    'spline' => 'spline',
    'donut' => 'donut',
    'gauge' => 'gauge',
  );

  return isset($type_map[$chart_type]) ? $type_map[$chart_type] : $chart_type;
}

/**
 * Maps legend positions to C3 legend placement.
 */
function _charts_c3_legend_position($legend_position) {
  $position_map = array(
    'bottom' => 'bottom',
    'right' => 'right',
    'left' => 'right',
    'top' => 'inset',
  );

  return isset($position_map[$legend_position]) ? $position_map[$legend_position] : 'right';
}
