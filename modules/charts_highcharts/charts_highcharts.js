/**
 * @file
 * JavaScript integration between Highcharts and Backdrop.
 */
(function ($) {

Backdrop.behaviors.chartsHighcharts = {};
Backdrop.behaviors.chartsHighcharts.attach = function(context, settings) {
  $('.charts-highchart', context).once('charts-highchart', function() {
    if ($(this).attr('data-chart')) {
      var config = $.parseJSON($(this).attr('data-chart'));
      if (typeof $.fn.highcharts === 'function') {
        $(this).highcharts(config);
      }
      else if (typeof Highcharts !== 'undefined' && typeof Highcharts.chart === 'function') {
        Highcharts.chart(this, config);
      }
      else {
        var attempts = ($(this).data('chartsHighchartsAttempts') || 0) + 1;
        $(this).data('chartsHighchartsAttempts', attempts);
        $(this).removeClass('charts-highchart-processed');
        if (attempts <= 50) {
          setTimeout(function() {
            Backdrop.behaviors.chartsHighcharts.attach(document, settings);
          }, 100);
        }
      }
    }
  });
};

})(jQuery);
