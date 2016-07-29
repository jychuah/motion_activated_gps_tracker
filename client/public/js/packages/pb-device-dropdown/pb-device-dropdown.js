define('pbdevicedropdown',
  ['jquery',
    'text!./html/pb-device-dropdown.html',
    'text!./html/pb-device-li.html',
    'particlebase',
    'pbdevicesmodal',
    'bootstrapgrowl'],
  function($, dropdownHtml, liHtml) {
  PBDeviceDropdown = function(particleBase, callback) {
    var pb = particleBase;
    var cb = callback;
    var ref = this;
    var selected = null;
    var watching = false;
    var retrieved = null;
    this.$pbdevicedropdown = $('.pb-device-dropdown');
    function init() {
      this.$pbdevicedropdown.html(dropdownHtml);
      this.pbdevicesmodal = new PBDevicesModal(pb);
    }

    pb.addCallback(
      function(status) {
        if (status === ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN && !watching) {
          firebase.database().ref('ParticleBase/users')
            .child(firebase.auth().currentUser.uid)
            .child('devices')
            .on('value', populate);
        }
      }
    );

    function deviceClick(source) {
      var device = ref.retrieved[$(source.currentTarget).parent('[data-device-dropdown="item"]').attr('device-id')];
      setCurrent(device);
      cb(device);
    }

    function createLi(device) {
      var obj = $($(liHtml));
      obj.attr('device-id', device.id);
      var a = obj.find('[data-device-dropdown="name"]');
      a.html(device.name);
      a.click(deviceClick);
      return obj;
    }

    function setCurrent(device) {
      ref.$pbdevicedropdown.find('[data-device-dropdown="current"]').html(device.name);
    }

    function populate(snapshot) {
      var devices = snapshot.val();
      var pickNew = false;
      ref.retrieved = devices;
      if (!devices) {
        ref.$pbdevicedropdown.find('[data-device-dropdown="item"]').remove();
        pickNew = true;
      } else {
        for (var id in devices) {
          if (!ref.$pbdevicedropdown.find('[device-id=" + id + "]').length) {
            var obj = createLi(devices[id]);
            ref.$pbdevicedropdown.find('[data-device-dropdown="dropdown"]').prepend(obj);
            if (!selected) {
              selected = id;
              setCurrent(ref.retrieved[id]);
            }
          }
        }
        var existing = ref.$pbdevicedropdown.find('[data-device-dropdown="item"]');
        for (var i = 0; i < existing.length; i++) {
          var itemId = $(existing[i]).attr('device-id');
          if (!(itemId in devices)) {
            existing[i].remove();
            if (selected === itemId) {
              pickNew = true;
            }
          }
        }
      }
      if (pickNew) {
        if (!ref.retrieved) {
          selected = null;
          setCurrent({ name : "Devices" });
          cb(null);
        } else {
          var newCurrent = ref.retrieved(ref.retrieved.keys[0]);
          selected = newCurrent.id;
          setCurrent(newCurrent);
          cb(newCurrent);
        }
      }
    }

    $('.pb-device-dropdown').addClass('dropdown');
    init.apply(this);
  }

  PBDeviceDropdown.prototype = {
    constructor: PBDeviceDropdown,
  }
  return PBDeviceDropdown;
});
