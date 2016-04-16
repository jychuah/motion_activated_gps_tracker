define('pbdevicesmodal',
  ['jquery', 'text!./html/pb-devices-modal.html',
    'text!./html/pb-devices-item.html',
    './listgroup',
    'bootstrapgrowl'],
  function($, modalHtml, itemHtml) {
  PBDevicesModal = function(particleBaseInstance) {
    var pb = particleBaseInstance;
    var retrievedDevices = null;
    this.$pbdevicesmodal = $('.pb-devices-modal');

    function device_connect() {
      var selectedDeviceId = $('#device_list').find('li.active').attr('id');
      for (var key in retrievedDevices) {
        var device = retrievedDevices[key];
        if (device.id === selectedDeviceId) {
          pb.saveDevice(device, function(error) {
            if (error) {
              $.bootstrapGrowl("Couldn't connect your device to Firebase", { type : "danger" });
            } else {
              $.bootstrapGrowl("Device " + device.name + " was connected", { type : "success" });
            }
          });
        }
      }
    }

    function populate() {
      $("#device_list").html("");
      pb.getSavedDevices(function(error, data) {
        if (error) {
          $.bootstrapGrowl("A Firebase error occurred, attempting to retrieve your devices", { type : "danger" });
        } else {
          var savedDevices = data;
          pb.listDevices(function(error, data) {
            if (error) {
              $.bootstrapGrowl("Couldn't retrieve your devices from Particle.io", { type : "danger" });
            } else {
              retrievedDevices = data;
              for (var key in data) {
                var device = data[key];
                var obj = $($(itemHtml));
                obj.attr('id', device.id);
                obj.find("#device_name").text(device.name);
                obj.find("#device_id").text(device.id);
                if (savedDevices && device.id in savedDevices) {
                  obj.prop('disabled', true);
                }
                $("#device_list").append(obj);
              }
            }
          });
        }
      });
    }

    function init() {
      this.$pbdevicesmodal.html(modalHtml);
      $('.pb-devices-modal .list-group').listgroup();
      $("#device_connect").click(device_connect);
    }

    $('.pb-devices-modal').addClass('modal fade');
    $('.pb-devices-modal').prop('role', 'dialog');
    $('.pb-devices-modal').on('shown.bs.modal', populate);
    init.apply(this);
  }

  PBDevicesModal.prototype = {
    constructor: PBDevicesModal,
  }

  return PBDevicesModal;
});
