define(['jquery',
        'particle',
        'particlebase',
        'pbdevicesmodal', 'pbloginmodal',
        'fbaccountdropdown',
        'pbdevicedropdown',
        'particlevariablepanel',
        'particlefunctionpanel',
        'bootstrapgrowl',
        'firebase', 'bootstrap'], function($, Particle, ParticleBase, PBDevicesModal) {
  function App() {
      var Firebase = require('firebase');
      this.particle = new Particle();
      this.firebase = new Firebase("https://lighting-controller.firebaseio.com");
      this.pb = new ParticleBase(this.firebase);
      this.pb.addAccessTokenCallback($.proxy(this.accessTokenModalListener, this));
      $(document).ready(this.initialize.apply(this));

  };
  App.prototype = {
      constructor: App,

      accessTokenModalListener: function(status) {
        var ref = this;
        if (status === ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN) {
          $("#pb-login-modal").modal('show');
        }
      },

      deviceSelectListener: function(device_id) {
        this.pb.getDeviceTreeFirebase().child(device_id).once('value', function(dataSnapshot) {
          var deviceInfo = dataSnapshot.val();
          $("#device_name").html(deviceInfo.name);
          var deviceDetails = "<table border=0>";
          deviceDetails += "<tr><td>Device ID:&nbsp;&nbsp;</td><td>" + deviceInfo.id + "</td></tr>";
          deviceDetails += "<tr><td>Connected:&nbsp;&nbsp;</td><td>" + (deviceInfo.connected ? "yes" : "no") + "</td></tr>";
          deviceDetails += "<tr><td>Last heard:&nbsp;&nbsp;</td><td>" + deviceInfo.last_heard + "</td></tr>";
          deviceDetails += "<tr><td>Status:&nbsp;&nbsp;</td><td>" + deviceInfo.status + "</td></tr>";
          deviceDetails += "</table>";
          $("#device_info").html(deviceDetails);
        });
        if (this.particlevariablepanel) {
          $(this.particlevariablepanel).html("");
          this.particlevariablepanel = null;
        }
        if (device_id) {
          this.particlevariablepanel = new ParticleVariablePanel(device_id, this.pb.accessToken);
        }
        if (this.particlefunctionpanel) {
          $(this.particlefunctionpanel).html("");
          this.particlefunctionpanel = null;
        }
        if (device_id) {
          this.particlefunctionpanel = new ParticleFunctionPanel(device_id, this.pb.accessToken);
        }
      },

      initialize: function() {
          this.pbdevicesmodal = new PBDevicesModal(this.pb);
          this.pbloginmodal = new PBLoginModal(this.pb, function(status) {
            if (status === ParticleBase.ERROR_CANCEL) {
              $("#pb-login-modal").modal('show');
            }
          });
          this.fbaccountdropdown = new FBAccountDropdown(this.firebase);
          this.pbdevicedropdown = new PBDeviceDropdown(this.pb, $.proxy(this.deviceSelectListener, this));
      }
  };
  return App;
});
