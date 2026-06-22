/**
 * @file
 * Scripting for administrative interfaces of Charts module.
 */
(function ($) {

Backdrop.behaviors.chartsAdmin = {};
Backdrop.behaviors.chartsAdmin.attach = function(context, settings) {
  var libraryTypeMap = settings.chartsAdmin && settings.chartsAdmin.libraryTypeMap ? settings.chartsAdmin.libraryTypeMap : {};

  function filterTypesByLibrary($radios) {
    var $form = $radios.closest('form');
    var $librarySelect = $form.find('.chart-library-select');
    if (!$librarySelect.length) {
      return;
    }

    var selectedLibrary = $librarySelect.val();
    var supportedTypes = libraryTypeMap[selectedLibrary];
    if (!supportedTypes || !supportedTypes.length) {
      return;
    }

    var checkedAndSupported = false;
    $radios.find('input:radio').each(function() {
      var $radio = $(this);
      var isSupported = $.inArray($radio.val(), supportedTypes) !== -1;
      $radio.prop('disabled', !isSupported);
      $radio.closest('.form-item').toggle(isSupported);
      if (isSupported && $radio.is(':checked')) {
        checkedAndSupported = true;
      }
    });

    if (!checkedAndSupported) {
      var $firstSupported = $radios.find('input:radio:not(:disabled):first');
      if ($firstSupported.length) {
        $firstSupported.prop('checked', true);
      }
    }
  }

  // Change options based on the chart type selected.
  $(context).find('.form-radios.chart-type-radios').once('charts-axis-inverted', function() {
    var $radios = $(this);
    var $form = $radios.closest('form');
    var xAxisLabel = $form.find('fieldset.chart-xaxis .fieldset-title').html();
    var yAxisLabel = $form.find('fieldset.chart-yaxis .fieldset-title').html();

    // Manually attach collapsible fieldsets first.
    if (Backdrop.behaviors.collapse) {
      Backdrop.behaviors.collapse.attach(context, settings);
    }

    $radios.find('input:radio').change(function() {
      if ($(this).is(':checked')) {
        var groupingField = $form.find('.charts-grouping-field').val();

        // Flip X/Y axis fieldset labels for inverted chart types.
        if ($(this).attr('data-axis-inverted')) {
          $form.find('fieldset.chart-xaxis .fieldset-title').html(yAxisLabel);
          $form.find('fieldset.chart-xaxis .axis-inverted-show').closest('.form-item').show();
          $form.find('fieldset.chart-xaxis .axis-inverted-hide').closest('.form-item').hide();
          $form.find('fieldset.chart-yaxis .fieldset-title').html(xAxisLabel);
          $form.find('fieldset.chart-yaxis .axis-inverted-show').closest('.form-item').show();
          $form.find('fieldset.chart-yaxis .axis-inverted-hide').closest('.form-item').hide();
        }
        else {
          $form.find('fieldset.chart-xaxis .fieldset-title').html(xAxisLabel);
          $form.find('fieldset.chart-xaxis .axis-inverted-show').closest('.form-item').hide();
          $form.find('fieldset.chart-xaxis .axis-inverted-hide').closest('.form-item').show();
          $form.find('fieldset.chart-yaxis .fieldset-title').html(yAxisLabel);
          $form.find('fieldset.chart-yaxis .axis-inverted-show').closest('.form-item').hide();
          $form.find('fieldset.chart-yaxis .axis-inverted-hide').closest('.form-item').show();
        }

        // Show color options for single axis settings.
        if ($(this).attr('data-axis-single')) {
          $form.find('fieldset.chart-xaxis').hide();
          $form.find('fieldset.chart-yaxis').hide();
          $form.find('th.chart-field-color, td.chart-field-color').hide();
          $form.find('div.chart-colors').show();
        }
        else {
          $form.find('fieldset.chart-xaxis').show();
          $form.find('fieldset.chart-yaxis').show();
          if (groupingField) {
            $form.find('th.chart-field-color, td.chart-field-color').hide();
            $form.find('div.chart-colors').show();
          }
          else {
            $form.find('th.chart-field-color, td.chart-field-color').show();
            $form.find('div.chart-colors').hide();
          }
        }
      }
    });

    $form.find('.chart-library-select').once('charts-library-type-filter').change(function() {
      var $currentForm = $(this).closest('form');
      $currentForm.find('.form-radios.chart-type-radios').each(function() {
        filterTypesByLibrary($(this));
        $(this).find('input:radio:checked').triggerHandler('change');
      });
    });

    filterTypesByLibrary($radios);

    // Set the initial values.
    $radios.find('input:radio:checked').triggerHandler('change');
  });

  // React to the setting of a group field.
  $(context).find('.charts-grouping-field').once('charts-grouping', function() {
    $(this).change(function() {
      var $form = $(this).closest('form');

      // Hide the entire grouping field row, since no settings are applicable.
      var value = $(this).val();
      $form.find('#chart-fields tr').show();
      if (value) {
        var $labelField = $form.find('.chart-label-field input[value="' + value + '"]');
        $labelField.closest('tr').hide();
        if ($labelField.is(':checked')) {
          $form.find('input[name="style_options[label_field]"][value=""]').attr('checked', 'checked').triggerHandler('change');
        }
      }
      // Restripe the table after hiding/showing rows.
      $form.find('#chart-fields tr:visible')
        .removeClass('odd even')
        .filter(':even').addClass('odd').end()
        .filter(':odd').addClass('even');

      // Recalculate shown color fields by triggering the chart type change.
      $form.find('.form-radios.chart-type-radios input:radio:checked').triggerHandler('change');
    }).triggerHandler('change');
  });

  // Disable the data checkbox when a field is set as a label.
  $(context).find('td.chart-label-field input').once('charts-axis-inverted', function() {
    var $radio = $(this);
    $radio.change(function() {
      if ($radio.is(':checked')) {
        var $form = $radio.closest('form');
        $form.find('.chart-data-field input').show();
        $form.find('.chart-field-color input').show();
        $form.find('input.chart-field-disabled').remove();
        $radio.closest('tr').find('.chart-data-field input').hide().after('<input type="checkbox" name="chart_field_disabled" disabled="disabled" class="chart-field-disabled" />');
        $radio.closest('tr').find('.chart-field-color input').hide();
      }
    });
    $radio.triggerHandler('change');
  });

};

})(jQuery);
