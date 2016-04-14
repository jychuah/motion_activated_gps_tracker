define(['jquery',
        'particlebase',
        'pbdevicesmodal', 'pbloginmodal',
        'fbaccountdropdown',
        'pbdevicedropdown',
        'bootstrapgrowl',
        'firebase', 'bootstrap'], function($, ParticleBase, PBDevicesModal) {
  function App() {
      var Firebase = require('firebase');
      this.firebase = new Firebase("https://lighting-controller.firebaseio.com");
      this.pb = new ParticleBase(this.firebase);
      this.pb.addAccessTokenCallback($.proxy(this.accessTokenCallback, this));
      $(document).ready(this.initialize.apply(this));
  };
  App.prototype = {
      constructor: App,

      accessTokenCallback : function(status) {
        console.log("main app access token callback: ", status);
        var ref = this;
        /*
        if (status === ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN) {
          ref.pb.listDevices(function(error, data) {
            if (error) {
            } else {
              // populate device
              ref.pbdevicedropdown.populate();
            }
          });
        } else {
        //  ref.pbdevicedropdown.init();
          $("#pb-login-modal").modal('show');
        }
        */
      },

      initialize: function() {
          this.pbdevicesmodal = new PBDevicesModal(this.pb);
          this.pbloginmodal = new PBLoginModal(this.pb, function(status) {
            if (status === ParticleBase.ERROR_CANCEL) {
              $("#pb-login-modal").modal('show');
            }
          });
          this.fbaccountdropdown = new FBAccountDropdown(this.firebase);
          this.pbdevicedropdown = new PBDeviceDropdown(this.pb, function(device_id) {
            console.log("Device selected: ", device_id);
          });
      }
  };
  return App;
});
