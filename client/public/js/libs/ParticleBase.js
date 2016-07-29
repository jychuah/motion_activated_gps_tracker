define(['particle'], function(Particle) {
  // requires a firebase reference with a child called ParticleBase
  ParticleBase = function() {
    this.particle = new Particle();
    var ref = this;
    try {
      firebase.app();
    } catch (error) {
      throw "Firebase has not been initialized yet";
    }

    // callback will be passed null on success, or one of the following errors:
    function testToken(callback) {
      // console.log("testing token", this.profile.token);
      var token = ref.profile.token;
      var devicesPr = ref.particle.listDevices({ auth: token });
      devicesPr.then(
        function(devices){
          // console.log("Successfully listed devices", devices);
          callback(devices);
        },
        function(err) {
          // console.log('List devices call failed: ', err);
          callback(null);
        }
      );
    };

    function userChanged(dataSnapshot) {
      if (!dataSnapshot || !dataSnapshot.exists()) {
        // console.log("user has no profile");
        ref.profile = null;
        notifyCallbacks();
      } else {
        var data = dataSnapshot.val();
        ref.profile.devices = data.devices;
        ref.profile.token = data.token;
        testToken(function(status) {
          if (!status) {
            ref.profile.token = null;
          }
          notifyCallbacks();
        });
      }
    }

    this.callbacks = new Array();
    this.lastMessage = "";
    this.profile = null;

    firebase.auth().onAuthStateChanged(function(user) {
      if (user) {
        ref.profile = { };
        ref.profile.user = user;
        // user logged in, get user info from ParticleBase tree
        firebase.database().ref('/ParticleBase/users/').child(user.uid).on('value', userChanged);
      } else {
        // user logged out
        // console.log("logged out of firebase");
        ref.profile = null;
      }
    })

    function notifyCallbacks() {
      for (var i = 0; i < ref.callbacks.length; i++) {
        ref.callbacks[i](ref.profile);
      }
    }

    this.getProfile = function() {
      return ref.profile;
    }

    this.getAccessToken = function() {
      return ref.profile ? (ref.profile.token ? ref.profile.token : null) : null;
    };

    this.getDevices = function() {
      return ref.profile.devices ? (ref.profile.devices ? ref.profile.devices : null) : null;
    }

    // Add callback for events
    this.addCallback = function(callback) {
      ref.callbacks.push(callback);
    }

    this.saveAccessToken = function(token) {
      if (!ref.profile.user) {
        throw "Not logged in to Firebase.";
      }
      firebase.database().ref('/ParticleBase/users/').child(this.user.uid).child("token").set(token);
    }

  };

  ParticleBase.prototype = {
    constructor: ParticleBase,
  };

  return ParticleBase;
});
