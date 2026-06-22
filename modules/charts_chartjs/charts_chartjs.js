/**
 * @file
 * JavaScript integration between Chart.js and Backdrop.
 */
(function ($) {

Backdrop.behaviors.chartsChartjs = {};
Backdrop.behaviors.chartsChartjs.attach = function(context, settings) {
  $('.charts-chartjs', context).once('charts-chartjs', function() {
    var $this = $(this);
    if ($this.attr('data-chart')) {
      if (typeof Chart === 'undefined') {
        // Library may load after AJAX insertion; allow reprocessing.
        var attempts = ($this.data('chartsChartjsAttempts') || 0) + 1;
        $this.data('chartsChartjsAttempts', attempts);
        $this.removeClass('charts-chartjs-processed');
        if (attempts <= 50) {
          setTimeout(function() {
            Backdrop.behaviors.chartsChartjs.attach(document, settings);
          }, 100);
        }
        return;
      }

      var config = $.parseJSON($this.attr('data-chart'));
      var chartId = $this.attr('id');

      // Recursively process the configuration to replace placeholders with functions.
      function processConfig(obj) {
        for (var key in obj) {
          if (typeof obj[key] === 'object' && obj[key] !== null) {
            processConfig(obj[key]);
            if (obj[key].callback === '###TICK_CALLBACK###') {
              (function(ticksConfig) {
                var prefix = ticksConfig._prefix || '';
                var suffix = ticksConfig._suffix || '';
                ticksConfig.callback = function(value, index, ticks) {
                  // If it's a category axis, value might be a string (label) or an index.
                  // Chart.js v4 callback for category axis: value is the index.
                  // For linear axis: value is the numeric value.
                  var label = value;
                  if (this.getLabelForValue) {
                    label = this.getLabelForValue(value);
                  }

                  if (typeof label === 'number') {
                    // Use a more standard formatting if toLocaleString is not enough.
                    label = label.toLocaleString(undefined, {
                      minimumFractionDigits: 0,
                      maximumFractionDigits: 20
                    });
                  }
                  return prefix + label + suffix;
                };
              })(obj[key]);
            }
          }
        }
      }
      processConfig(config);

      // Views preview containers can have no computed height; provide a
      // fallback so responsive charts remain visible.
      if ($this.height() < 20) {
        $this.css('min-height', '240px');
      }

      // Chart.js requires a canvas element.
      var $canvas = $('<canvas></canvas>').attr('id', chartId + '-canvas');
      $this.append($canvas);

      var ctx = $canvas[0].getContext('2d');
      new Chart(ctx, config);
    }
  });
};

})(jQuery);
