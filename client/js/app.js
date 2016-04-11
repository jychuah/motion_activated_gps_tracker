define(['jquery',
        'particlebase',
        'pbdevicesmodal', 'pbloginmodal',
        'fbaccountnavbar',
        'firebase', 'bootstrap'], function($, ParticleBase, PBDevicesModal) {
  function App() {
      var Firebase = require('firebase');
      this.firebase = new Firebase("https://lighting-controller.firebaseio.com");
      this.pb = new ParticleBase(this.firebase);
      this.pb.setAccessTokenCallback($.proxy(this.accessTokenCallback, this));
      $(document).ready(this.initialize.apply(this));
  };
  App.prototype = {
      constructor: App,

      accessTokenCallback : function(status) {
        console.log("Access token callback: ", status);
        var ref = this;
        if (status === ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN) {
          ref.pb.listDevices(function(error, data) {
            if (error) {
              console.log("Couldn't retrieve devices");
            } else {
              
            }
            /*
            console.log("List devices status: ", status);
            console.log("Device list: ", data);
            for (var key in data) {
              ref.pb.saveDevice(data[key], function(error) {
                if (!error) {
                  ref.pb.getSavedDevices(function(error, data) {
                    if (!error) {
                      console.log("Saved devices: ", data);
                    } else {
                      console.log("Error retrieving saved devices: ", error);
                    }
                  });
                } else {
                  console.log("Error saving device: ", error);
                }
              });
            }
            */
          });
        } else {
          $("#pb-login-modal").modal('show');
//          alert("Login to particle");
        }
      },


      initialize: function() {
          this.pbdevicesmodal = new PBDevicesModal(this.pb);
          this.pbloginmodal = new PBLoginModal(this.pb, function(status) {
            console.log("pb-login-modal callback: ", status);
          });
          this.fbaccountnavbar = new FBAccountNavbar(this.firebase);
//          $("#particle_login").click($.proxy(this.particle_login, this));
      }
  };
  return App;
});
