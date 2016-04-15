define('pbcontrolpanel',
  ['jquery',
    'text!pbcontrolpanel/html/pb-control-panel.html',
    'particlebase',
    'bootstrapgrowl'],
  function($, panelHtml) {
  PBControlPanel = function(particleBase, device_id) {
    var pb = particleBase;
    var id = device_id
    this.$pbcontrolpanel = $('.pb-control-panel');
    function init() {
      this.$pbcontrolpanel.html(panelHtml);
    }


    pb.addAccessTokenCallback(
      $.proxy(function(status) {
      }, this));

    $('.pb-control-panel').addClass('panel panel-default');
    init.apply(this);
  }

  PBControlPanel.prototype = {
    constructor: PBControlPanel,
  }
  return PBControlPanel;
});
