/**
 * @file
 * JavaScript integration between C3 and Backdrop.
 */
(function ($) {

Backdrop.behaviors.chartsC3 = {};
Backdrop.behaviors.chartsC3.attach = function(context, settings) {
  $('.charts-c3', context).once('charts-c3', function() {
    var $this = $(this);
    if ($this.attr('data-chart')) {
      if (typeof c3 === 'undefined') {
        var attempts = ($this.data('chartsC3Attempts') || 0) + 1;
        $this.data('chartsC3Attempts', attempts);
        $this.removeClass('charts-c3-processed');
        if (attempts <= 50) {
          setTimeout(function() {
            Backdrop.behaviors.chartsC3.attach(document, settings);
          }, 100);
        }
        return;
      }
      var config = $.parseJSON($this.attr('data-chart'));
      c3.generate(config);
    }
  });
};

})(jQuery);
