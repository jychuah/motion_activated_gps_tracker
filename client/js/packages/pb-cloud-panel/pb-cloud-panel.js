define('pbcloudpanel',
  ['jquery',
    'text!pbcloudpanel/html/pb-cloud-panel.html',
    'particlebase',
    'bootstrapgrowl'],
  function($, panelHtml) {
  PBCloudPanel = function(particleBase, device_id) {
    var pb = particleBase;
    var id = device_id
    this.$pbcloudpanel = $('.pb-cloud-panel');
    function init() {
      this.$pbcloudpanel.html(panelHtml);
    }


    pb.addAccessTokenCallback(
      $.proxy(function(status) {
      }, this));

    $('.pb-cloud-panel').addClass('panel panel-default');
    init.apply(this);
  }

  PBCloudPanel.prototype = {
    constructor: PBCloudPanel,
  }
  return PBCloudPanel;
});
