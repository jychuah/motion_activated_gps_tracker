define('pbdevicedropdown',
  ['jquery',
    'text!./html/pb-device-dropdown.html',
    'text!./html/pb-device-li.html',
    'particlebase',
    'bootstrapgrowl'],
  function($, dropdownHtml, liHtml) {
  PBDeviceDropdown = function(particleBase, callback) {
    var pb = particleBase;
    var cb = callback;
    this.$pbdevicedropdown = $('.pb-device-dropdown');
    function init() {
      this.$pbdevicedropdown.html(dropdownHtml);
    }

    function select_device(source) {
      var id = $(source.target).attr('device_id');
      var device_name = $(source.target).html();
      $("#current_device").text(device_name);
      if (cb) {
        cb(id);
      }
    }

    pb.addAccessTokenCallback(
      $.proxy(function(status) {
        if (status === ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN) {
          populate.apply(this);;
        } else {
          init.apply(this);
          cb(null);
        }
      }, this));

    function populate() {
      pb.getSavedDevices(function(error, data) {
        if (error) {
        } else {
          if (data) {
            $('[device-dropdown="device-controller"]').removeClass('disabled');
            var first = true;
            for (var key in data) {
              li = $($(liHtml));
              li.find('a').text(data[key].name);
              li.find('a').attr('device_id', data[key].id);
              li.find('a').click(select_device);
              $('[device-dropdown="device-dropdown"]').prepend(li);
              if (first) {
                first = false;
                li.find('a').click();
              }
            }
          }
        }
      });
    }

    $('.pb-device-dropdown').addClass('dropdown');
    init.apply(this);
  }

  PBDeviceDropdown.prototype = {
    constructor: PBDeviceDropdown,
  }
  return PBDeviceDropdown;
});