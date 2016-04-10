define('pbdevicesmodal',
  ['jquery', 'text!pbdevicesmodal/html/pb-devices-modal.html', 'text!pbdevicesmodal/html/pb-devices-item.html', './listgroup'],
  function($, modalHtml, itemHtml) {
  PBDevicesModal = function(particleBaseInstance) {
    var pb = particleBaseInstance;
    this.$pbdevicesmodal = $('.pb-devices-modal');

    function init() {
      console.log("PB Devices Modal Hello World");
      this.$pbdevicesmodal.html(modalHtml);
      $('.pb-devices-modal .list-group').listgroup();
    }

    $('.pb-devices-modal').addClass('modal fade');
    $('.pb-devices-modal').prop('role', 'dialog');
    init.apply(this);
  }

  PBDevicesModal.prototype = {
    constructor: PBDevicesModal,
  }

  return PBDevicesModal;
});
