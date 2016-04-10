define(['jquery',
        'particlebase',
        'pbdevicesmodal', 'pbloginmodal',
        'fbloginmodal', 'fbchangepwmodal',
        'fbcreatemodal', 'fbresetmodal',
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
          ref.pb.listDevices(function(status, data) {
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
          });
        } else {
          $("#pb-login-modal").modal('show');
//          alert("Login to particle");
        }
      },
      fb_logout : function() {
        this.firebase.unauth();
      },

      initialize: function() {
          this.pbdevicesmodal = new PBDevicesModal(this.pb);
          this.pbloginmodal = new PBLoginModal(this.pb, function(status) {
            console.log("pb-login-modal callback: ", status);
          });
          this.fbloginmodal = new FBLoginModal(this.firebase, function(error, auth) {
            if (error) {
              console.log("Firebase login error: ", error);
            } else {
              console.log("Firebase login auth: ", auth);
            }
          });
          this.fbchangepwmodal = new FBChangePwModal(this.firebase, function(error) {
            if (error) {
              console.log("Firebase change password error: ", error);
            } else {
              console.log("Firebase changed password");
            }
          });
          this.fbcreatemodal = new FBCreateModal(this.firebase, function(error, authData) {
            if (error) {
              console.log("Firebase create user error: ", error);
            } else {
              console.log("Firebase user created: ", authData);
            }
          });
          this.fbresetmodal = new FBResetModal(this.firebase, function(error) {
            if (error) {
              console.log("Firebase user reset error: ", error);
            } else {
              console.log("Firebase user password reset requested.");
            }
          })
          $("#fb_logout").click($.proxy(this.fb_logout, this));
//          $("#particle_login").click($.proxy(this.particle_login, this));
      }
  };
  return App;
});
