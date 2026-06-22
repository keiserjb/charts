/**
 * @file
 * JavaScript integration between Billboard.js and Backdrop.
 */
(function ($) {

Backdrop.behaviors.chartsBillboard = {};
Backdrop.behaviors.chartsBillboard.attach = function(context, settings) {
  $('.charts-billboard', context).once('charts-billboard', function() {
    var $this = $(this);
    if ($this.attr('data-chart')) {
      if (typeof bb === 'undefined') {
        var attempts = ($this.data('chartsBillboardAttempts') || 0) + 1;
        $this.data('chartsBillboardAttempts', attempts);
        $this.removeClass('charts-billboard-processed');
        if (attempts <= 50) {
          setTimeout(function() {
            Backdrop.behaviors.chartsBillboard.attach(document, settings);
          }, 100);
        }
        return;
      }
      var config = $.parseJSON($this.attr('data-chart'));
      bb.generate(config);
    }
  });
};

})(jQuery);
