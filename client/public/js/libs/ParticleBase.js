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

    function userChanged(dataSnapshot) {
      if (!dataSnapshot || !dataSnapshot.exists()) {
        // console.log("user has no profile");
        notifyCallbacks(ParticleBase.ERROR_PARTICLEBASE_NO_ACCESS_TOKEN);
      } else {
        ref.profile = dataSnapshot.val();
        ref.testToken(function(status) {
          if (status) {
            notifyCallbacks(ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN);
          } else {
            notifyCallbacks(ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN);
          }
        });
      }
    }

    this.callbacks = new Array();
    this.lastMessage = "";
    this.profile = null;
    this.user = null;
    firebase.auth().onAuthStateChanged(function(user) {
      if (user) {
        ref.user = user;
        // user logged in, get user info from ParticleBase tree
        firebase.database().ref('/ParticleBase/users/').child(user.uid).on('value', userChanged);
      } else {
        // user logged out
        // console.log("logged out of firebase");
        ref.user = null;
      }
    })

    function notifyCallbacks(message) {
      ref.lastMessage = message;
      for (var i = 0; i < ref.callbacks.length; i++) {
        ref.callbacks[i](message);
      }
    }

    function hasAccessToken() {
      return ref.profile.token;
    };

    // Add callback for events
    this.addCallback = function(callback) {
      ref.callbacks.push(callback);
    }

    // Simple device notification hook. Publishes an event with custon eventName,
    // and data will be a JSON.stringify'd dataSnapshot.val()
    // params:
    // firebaseRef - reference to firebase child location for event notification
    // eventType - firebase event type ('value', 'child_changed', 'child_added', etc...)
    // callback - callback on firebase event with dataSnapshot as parameter (as per standard Firebase.on())
    // cancelCallback - callback when the event is removed (as per stadard Firebase.on())
    // context - "this" context for callback and cancelCallback (as per standard Firebase.on())
    this.simplePublishOn = function(firebaseRef, event, eventName, callback, cancelCallback, context) {
      var ref = this;
      firebaseRef.on(eventType, function(dataSnapshot) {
        ref.publishEvent(eventName, JSON.stringify(dataSnapshot.val()));
        if (callback) {
          callback.apply(context, [dataSnapshot]);
        }
      }, cancelCallback, context);
    };

    // device notification hook. Publishes an event called "ParticleBase"
    // with JSON data:
    // {
    //    path : "/event/path",
    //    firebase : "https://my-firebase.firebaseio.com",
    //    eventType : eventType,
    //    dataSnapshot : dataSnapshot.val()
    // }
    // params:
    // firebaseRef - reference to firebase child location for event notification
    // eventType - firebase event type ('value', 'child_changed', 'child_added', etc...)
    // callback - callback on firebase event with dataSnapshot as parameter (as per standard Firebase.on())
    // cancelCallback - callback when the event is removed (as per stadard Firebase.on())
    // context - "this" context for callback and cancelCallback (as per standard Firebase.on())
    this.publishOn = function(firebaseRef, eventType, callback, cancelCallback, context) {
      var ref = this;
      firebaseRef.on(eventType, function(dataSnapshot) {
        var json = { };
        json.path = dataSnapshot.ref().toString().substring(ref.firebase.root().ref().toString().length);
        json.firebase = dataSnapshot.ref().root().toString();
        json.eventType = eventType;
        json.dataSnapshot = dataSnapshot.val();
        ref.publishEvent("ParticleBase",
          JSON.stringify(json),
          function(status) {
          }
        );
        if (callback) {
          callback.apply(context, [dataSnapshot]);
        }
      }, cancelCallback, context);
    };

    // REST publish event wrapper
    // callback will be passed null on success, or one of the following errors:
    // ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN
    // ParticleBase.ERROR_PARTICLE_UNREACHABLE
    // ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN
    // ParticleBase.ERROR_PARTICLE_SERVER_ERROR
    this.publishEvent = function(eventName, data, callback) {
      if (!sanityCheck(callback)) {
        return false;
      }
      var xhr = buildXHR("POST", "/v1/devices/events");
      xhr.setRequestHeader("Authorization", "Bearer " + this.accessToken);
      var ref = this;
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 0 && xhr.status == 0) {
          callback(ParticleBase.ERROR_PARTICLE_UNREACHABLE, null);
          return false;
        };
        if (xhr.readyState == 4) {
          var status = getParticleXHRStatus(xhr.status);
          callback(status);
          return true;
        }
      }
      xhr.send("name=" + eventName + "&data=" + encodeURIComponent(data) + "&private=true&ttl=60");
    };

    // callback will be passed null on success, or one of the following errors:
    // ParticleBase.ERROR_PARTICLE_UNREACHABLE
    // ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN
    // ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN
    // ParticleBase.ERROR_PARTICLE_SERVER_ERROR
    this.testToken = function(callback) {
      // console.log("testing token", this.profile.token);
      var token = this.profile.token;
      var devicesPr = this.particle.listDevices({ auth: token });
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

    this.saveAccessToken = function(token) {
      if (!this.user) {
        throw "Not logged in to Firebase.";
      }
      firebase.database().ref('/ParticleBase/users/').child(this.user.uid).child("token").set(token);
    }

    // TODO: Make this function create a permanent access token and return it
    // callback will be passed null on success, or one of the following statuses:
    // ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN
    // ParticleBase.ERROR_PARTICLE_UNREACHABLE
    // ParticleBase.ERROR_FIREBASE_COULD_NOT_SET_PARTICLE_TOKEN
    // ParticleBase.ERROR_PARTICLE_BAD_RESPONSE
    // If successful, the supplied Access Token Callback will be triggered
    // with status ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN
    this.bindAccessToken = function(particle_username, particle_password, callback) {
      if (!firebaseLoggedIn()) {
        callback(ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN);
        return false;
      }
      var xhr = buildXHR("POST", "/oauth/token");
      xhr.setRequestHeader ("Authorization", "Basic " + btoa("particle:particle"));
      var ref = this;
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 0 && xhr.status == 0) {
          callback(ParticleBase.ERROR_PARTICLE_UNREACHABLE);
          return false;
        };
        if (xhr.readyState == 4) {
          var data = xhr.status == 200 ? JSON.parse(xhr.responseText) : null;
          var status = getParticleXHRStatus(xhr.status);
          if (status === null) {
            if ("access_token" in data) {
              var token = data.access_token;
              ref.firebase.child('ParticleBase').child('users').child('tokens').child(ref.firebase.getAuth().uid).set(token, function(error) {
                if (error) {
                  callback(ParticleBase.ERROR_FIREBASE_COULD_NOT_SET_PARTICLE_TOKEN);
                } else {
                  callback(null);
                }
              });
            } else {
              callback(ParticleBase.ERROR_PARTICLE_BAD_RESPONSE);
              return false;
            }
          } else {
            callback(status);
            return false;
          }
          return true;
        }
      };
      xhr.send("grant_type=password&expires_in=0&username=" +
          encodeURIComponent(particle_username) + "&password=" +
          encodeURIComponent(particle_password));
      return true;
    };
  };


  ParticleBase.ERROR_PARTICLE_UNREACHABLE = "Particle.io was unreachable.";
  ParticleBase.ERROR_PARTICLE_INVALID_CREDENTIALS = "The supplied username/password for Particle.io was invalid.";
  ParticleBase.ERROR_PARTICLE_SERVER_ERROR = "Particle.io had some type of server error. Please try again later.";
  ParticleBase.ERROR_PARTICLE_UNKNOWN_ERROR = "There was an unknown error while contacting Particle.io.";
  ParticleBase.ERROR_PARTICLE_BAD_RESPONSE = "Particle.io was successfully contacted, but didn't return an Authorization token.";
  ParticleBase.ERROR_PARTICLEBASE_UNKNOWN = "Hmm... Unhandled ParticleBase error.";
  ParticleBase.ERROR_FIREBASE_COULD_NOT_SET_PARTICLE_TOKEN = "Could not set Particle.io authorization token.";
  ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN = "You are not logged in to Firebase.";
  ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN = "ParticleBase has acquired a valid access token.";
  ParticleBase.ERROR_PARTICLEBASE_NO_ACCESS_TOKEN = "The logged in Firebase user does not yet have an access token from Particle.io.";
  ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN = "The access token for this Firebase user is currently invalid.";
  ParticleBase.ERROR_PARTICLEBASE_LIST_DEVICES = "ParticleBase could not retrieve a list of saved devices.";
  ParticleBase.ERROR_PARTICLEBASE_SAVE_DEVICE = "ParticleBase could not save a device JSON.";
  ParticleBase.ERROR_CANCEL = "ParticleBase action cancelled.";

  ParticleBase.prototype = {
    constructor: ParticleBase,
  };

  return ParticleBase;
});
