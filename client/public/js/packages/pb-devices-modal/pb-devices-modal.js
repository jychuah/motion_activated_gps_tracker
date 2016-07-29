define('pbdevicesmodal',
  ['jquery', 'particle','text!./html/pb-devices-modal.html',
    'text!./html/pb-devices-item.html',
    './listgroup',
    'bootstrapgrowl'],
  function($, Particle, modalHtml, itemHtml) {
  PBDevicesModal = function(particleBaseInstance, callback) {
    var particle = new Particle();
    var cb = callback;
    var pb = particleBaseInstance;
    var retrieved = null;
    var ref = this;
    this.$pbdevicesmodal = $('.pb-devices-modal');

    function device_connect() {
      var selectedId = $('[data-devices-modal="devices"]').find('li.active').attr('data-device-id');
      var device = retrieved[selectedId];
      firebase.database().ref('/ParticleBase/users')
        .child(firebase.auth().currentUser.uid)
        .child('devices')
        .child(selectedId).set(device)
        .then(function() {
          $.bootstrapGrowl(device.name + " was connected", { type : "success" });
          cb(device);
        });
    }

    function createDevice(device) {
      var existing = pb.getDevices();
      var obj = $($(itemHtml));
      obj.addClass("disabled");
      obj.attr('data-device-id', device.id);
      obj.find('[data-device-li="device_name"]').html(device.name);
      obj.find('[data-device-li="device_id"]').html(device.id);
      if (existing && device.id in existing) {
        obj.find('[data-device-li="added"]').removeClass('hidden');
      }
      return obj;
    }

    function populate() {
      $("[data-devices-modal='devices']").html("");
      var token = pb.getAccessToken();
      if (!token) {
        // console.log("No access token!");
        cb(null);
        return false;
      }
      retrieved = { };
      var promise = particle.listDevices({ auth : token });
      promise.then(
        function(data) {
          for (var index in data.body) {
            retrieved[data.body[index].id] = data.body[index];
            var obj = createDevice(data.body[index]);
            ref.$pbdevicesmodal.find('[data-devices-modal="devices"]').append(obj);
          }
        }
      );
    }

    function init() {
      this.$pbdevicesmodal.html(modalHtml);
      $('.pb-devices-modal .list-group').listgroup();
      $("[data-devices-modal='connect']").click(device_connect);
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
