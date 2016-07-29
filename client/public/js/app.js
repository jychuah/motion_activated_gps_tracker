define(['jquery',
        'particle',
        'particlebase',
        'particleaccesstokenmodal',
        'bootstrapgrowl',
        'bootstrap'], function($, Particle) {
  function App() {
      var ref = this;
      this.particle = new Particle();
      this.pb = new ParticleBase();
//      this.pb.addAccessTokenCallback($.proxy(this.accessTokenModalListener, this));
      $(document).ready(this.initialize.apply(this));

  };
  App.prototype = {
      constructor: App,

      pbLoginCallback: function(status) {
        var ref = this;
        if (status === ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN ||
          status === ParticleBase.ERROR_PARTICLEBASE_NO_ACCESS_TOKEN) {
          //  console.log("access token error");
          $("#particle-accesstoken-modal").modal('show');
        }
      },

      deviceSelectListener: function(device_id) {
        this.pb.getDeviceTreeFirebase().child(device_id).once('value', function(dataSnapshot) {
          var deviceInfo = dataSnapshot.val();
          $("#device_name").html(deviceInfo.name);
          var deviceDetails = "<table border=0>";
          deviceDetails += "<tr><td>Device ID:&nbsp;&nbsp;</td><td>" + deviceInfo.id + "</td></tr>";
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

      firebaseLogin : function() {
        var provider = new firebase.auth.GoogleAuthProvider();
        firebase.auth().signInWithPopup(provider).then(function(result) {
          var token = result.credential.accessToken;
          var user = result.user;
        }).catch(function(error) {
          var errorCode = error.code;
          var errorMessage = error.message;
          var email = error.email;
          var credential = error.credential;
          $.bootstrapGrowl("Error logging in: " + errorMessage, { type : "warning" });
        });
      },

      firebaseLogout : function() {
        firebase.auth().signOut();
      },

      initialize: function() {
        $("#loginLink").click(this.firebaseLogin);
        $("#logoutLink").click(this.firebaseLogout);
        var ref = this;

        this.pb.addCallback(this.pbLoginCallback);
        this.particleaccesstokenmodal = new ParticleAccessTokenModal(function(result) {
          if (result) {
            ref.pb.saveAccessToken(result);
            $.bootstrapGrowl("Successfully logged in to Particle.io", {type : "success"});
          } else {
            $.bootstrapGrowl("Couldn't log in to Particle.io", { type : "warning" });
          }
        });
        /*
          this.pbdevicesmodal = new PBDevicesModal(this.pb);
          this.pbloginmodal = new PBLoginModal(this.pb, function(status) {
            if (status === ParticleBase.ERROR_CANCEL) {
              $("#pb-login-modal").modal('show');
            }
          });
          this.fbaccountdropdown = new FBAccountDropdown(this.firebase);
          this.pbdevicedropdown = new PBDeviceDropdown(this.pb, $.proxy(this.deviceSelectListener, this));
          */
      }
  };
  return App;
});
