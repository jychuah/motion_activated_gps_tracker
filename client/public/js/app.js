define(['jquery',
        'particle',
        'particlebase',
        'particleaccesstokenmodal',
        'pbdevicedropdown',
        'bootstrapgrowl',
        'bootstrap'], function($, Particle) {
  function App() {
      var ref = this;
      this.particle = new Particle();
      this.pb = new ParticleBase();
      $(document).ready(this.initialize.apply(this));

  };
  App.prototype = {
      constructor: App,

      pbLoginCallback: function(profile) {
        if (profile) {
          if (!profile.token) {
            //  console.log("access token error");
            $("#particle-accesstoken-modal").modal('show');
            $("#particleConnectLink").removeClass('hidden');
            $("#pbdevicedropdown").addClass('hidden');
          } else {
            $("#particleConnectLink").addClass('hidden');
            $("#pbdevicedropdown").removeClass('hidden');
            if (!profile.devices) {
              $("#addDeviceLink").removeClass('hidden');
              $("#pbdevicedropdown").addClass('hidden');
            } else {
              $("#addDeviceLink").addClass('hidden');
              $("#pbdevicedropdown").removeClass('hidden');
            }
          }
        }
      },

      deviceSelectCallback: function(device) {
        console.log(device);
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
        this.pbdevicedropdown = new PBDeviceDropdown(this.pb, $.proxy(this.deviceSelectCallback, this));
      }
  };
  return App;
});
