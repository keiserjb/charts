/**
 * @file
 * JavaScript integration between Google Charts and Backdrop.
 */
(function ($) {

Backdrop.behaviors.chartsGoogle = {};
Backdrop.behaviors.chartsGoogle.attach = function(context, settings) {
  // Google API script may not be ready on first AJAX attach.
  if (typeof google === 'undefined' || typeof google.load !== 'function') {
    Backdrop.behaviors.chartsGoogle._attempts = (Backdrop.behaviors.chartsGoogle._attempts || 0) + 1;
    if (!Backdrop.behaviors.chartsGoogle._retryPending && Backdrop.behaviors.chartsGoogle._attempts <= 50) {
      Backdrop.behaviors.chartsGoogle._retryPending = true;
      setTimeout(function() {
        Backdrop.behaviors.chartsGoogle._retryPending = false;
        Backdrop.behaviors.chartsGoogle.attach(document, settings);
      }, 100);
    }
    return;
  }
  Backdrop.behaviors.chartsGoogle._attempts = 0;
  google.load('visualization', '1', { callback: renderCharts, packages: ['corechart', 'table', 'gauge'] });

  // Redraw charts on window resize.
  var debounce;
  if (!Backdrop.behaviors.chartsGoogle._resizeBound) {
    Backdrop.behaviors.chartsGoogle._resizeBound = true;
    $(window).on('resize', function() {
      clearTimeout(debounce);
      debounce = setTimeout(function() {
        $('.charts-google').each(function() {
          var wrap = $(this).data('chartsGoogleWrapper');
          if (wrap) {
            wrap.draw(this);
          }
        });
      }, 75);
    });
  }

  function renderCharts() {
    $('.charts-google', context).once('charts-google', function() {
      if ($(this).attr('data-chart')) {
        var config = $.parseJSON($(this).attr('data-chart'));
        var wrap = new google.visualization.ChartWrapper();
        wrap.setChartType(config.visualization);
        wrap.setDataTable(config.data);
        wrap.setOptions(config.options);

        // Apply data formatters. This only affects tooltips. The same format is
        // already applied via the hAxis/vAxis.format option.
        var dataTable = wrap.getDataTable();
        if (config.options.series) {
          for (var n = 0; n < config.options.series.length; n++) {
            if (config.options.series[n]['_format']) {
              var format = config.options.series[n]['_format'];
              if (format['dateFormat']) {
                var formatter = new google.visualization.DateFormat({ pattern: format['dateFormat'] });
              }
              else {
                var formatter = new google.visualization.NumberFormat({ pattern: format['format'] });
              }
              formatter.format(dataTable, n + 1);
            }
          }
        }

        // Apply individual point properties, by adding additional "role"
        // columns to the data table. So far this only applies "tooltip"
        // properties to individual cells. Ideally, this would support "color"
        // also. Feature request:
        // https://code.google.com/p/google-visualization-api-issues/issues/detail?id=1267
        var columnsToAdd = {};
        var rowCount = dataTable.getNumberOfRows();
        var columnCount = dataTable.getNumberOfColumns();
        for (var rowIndex in config._data) {
          if (config._data.hasOwnProperty(rowIndex)) {
            for (var columnIndex in config._data[rowIndex]) {
              if (config._data[rowIndex].hasOwnProperty(columnIndex)) {
                for (var role in config._data[rowIndex][columnIndex]) {
                  if (config._data[rowIndex][columnIndex].hasOwnProperty(role)) {
                    if (!columnsToAdd[columnIndex]) {
                      columnsToAdd[columnIndex] = {};
                    }
                    if (!columnsToAdd[columnIndex][role]) {
                      columnsToAdd[columnIndex][role] = new Array(rowCount);
                    }
                    columnsToAdd[columnIndex][role][rowIndex] = config._data[rowIndex][columnIndex][role];
                  }
                }
              }
            }
          }
        }
        // Add columns from the right-most position.
        for (var columnIndex = columnCount; columnIndex >= 0; columnIndex--) {
          if (columnsToAdd[columnIndex]) {
            for (var role in columnsToAdd[columnIndex]) {
              if (columnsToAdd[columnIndex].hasOwnProperty(role)) {
                dataTable.insertColumn(columnIndex + 1, {
                  type: 'string',
                  role: role,
                  p: {html: config.options.tooltip.isHtml}
                });
                for (var rowIndex in columnsToAdd[columnIndex][role]) {
                  dataTable.setCell(parseInt(rowIndex) - 1, columnIndex + 1, columnsToAdd[columnIndex][role][rowIndex]);
                }
              }
            }
          }
        }

        wrap.draw(this);
        $(this).data('chartsGoogleWrapper', wrap);
      }
    });
  }
};

})(jQuery);
