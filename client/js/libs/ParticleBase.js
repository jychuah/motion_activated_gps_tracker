define([], function(require) {
  // requires a firebase reference with a child called ParticleBase
  ParticleBase = function(firebaseRef) {
    var ref = this;
    if (firebaseRef == null) {
      throw "Firebase reference must not be null.";
    }
    this.accessToken = null;
    this.accessTokenCallback = null;
    this.firebase = firebaseRef;
    this.firebase.onAuth(function(auth) {
      if (auth) {
        var ref = this;
        var tokenChild = this.firebase.child('ParticleBase').child('users').child('tokens').child(auth.uid);
        tokenChild.on('value', function(dataSnapshot) {
          if (!dataSnapshot) {
            if (ref.accessTokenCallback) {
              ref.accessTokenCallback(ParticleBase.ERROR_PARTICLEBASE_NO_ACCESS_TOKEN);
            }
          } else if (dataSnapshot.exists()) {
            ref.accessToken = dataSnapshot.val();
            if (ref.accessTokenCallback) {
              ref.testToken(function(status) {
                switch(status) {
                  case null : ref.accessTokenCallback(ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN); break;
                  default : ref.accessTokenCallback(status); break;
                };
              });
            }
          } else {
            if (ref.accessTokenCallback) {
              ref.accessTokenCallback(ParticleBase.ERROR_PARTICLEBASE_NO_ACCESS_TOKEN);
            }
          }
        });
      }
    }, this);


    function buildXHR(method, endpoint) {
      var xhr = new XMLHttpRequest();
      xhr.open(method, ParticleBase.ApiUrl + endpoint, true);
      xhr.setRequestHeader("Accept", "*/*");
      xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
      return xhr;
    }

    function firebaseLoggedIn() {
      return ref.firebase.getAuth() != null;
    };

    function getParticleXHRStatus(xhrstatus, success) {
      var status = ParticleBase.ERROR_PARTICLEBASE_UNKNOWN;
      switch(xhrstatus) {
        case 0 : status = ParticleBase.ERROR_PARTICLE_UNREACHABLE; break;
        case 200 : status = success ? success : null; break;
        case 400 : status = ParticleBase.ERROR_PARTICLE_INVALID_CREDENTIALS; break;
        case 401 : {
                      status = ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN;
                      if (this.accessTokenCallback) this.accessTokenCallback(ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN);
                  };
                  break;
        case 500 : status = ParticleBase.ERROR_PARTICLE_SERVER_ERROR; break;
        default : status = ParticleBase.ERROR_PARTICLE_UNKNOWN_ERROR; break;
      };
      return status;
    };

    function sanityCheck(callback) {
      if (!firebaseLoggedIn()) {
        callback(ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN);
        return false;
      }
      if (!hasAccessToken()) {
        callback(ParticleBase.ERROR_PARTICLEBASE_NO_ACCESS_TOKEN);
        return false;
      }
      return true;
    }


    function hasAccessToken() {
      return ref.accessToken != null;
    };

    // Sets an access token event callback which fires if an accessToken
    // is missing, invalid, or acquired.
    // callback will be triggered with
    // ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN or one of the following errors:
    // ParticleBase.ERROR_PARTICLE_INVALID_CREDENTIALS (upon trying to bind a token with a bad username/password)
    // ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN (upon trying to publish an event with an invalid token)
    // ParticleBase.ERROR_PARTICLEBASE_NO_ACCESS_TOKEN (upon logging in but not detecting an access token)
    // This callback could be used to collect a Particle.io
    // username and password, which can be passed to the bindAccessToken function
    this.setAccessTokenCallback = function(callback) {
      this.accessTokenCallback = callback;
    };

    // This function lists "saved" devices. (See saveDevice)
    // callback first parameter will be null on success, or an error message
    // callback second parameter will be a JSON object containing saved devices
    // (saved devices may be an empty JSON object)
    this.getSavedDevices = function(callback) {
      if (!sanityCheck()) {
        return false;
      }
      this.firebase.child('ParticleBase').child('users').child('devices')
        .child(this.firebase.getAuth().uid).once('value', function(dataSnapshot) {
        if (!dataSnapshot) {
          console.log("Device list was empty");
          callback(null, { });
        } else {
          callback(null, dataSnapshot.val());
        }
      }, function(error) {
        callback(ParticleBase.ERROR_PARTICLEBASE_LIST_DEVICES);
      });
    };

    // A client may wish to track only some of a user's devices, instead of
    // every Particle.io device available to the user. This mechanism
    // "saves" devices
    // callback receives null on success or an error message
    this.saveDevice = function(deviceJson, callback) {
      if (!sanityCheck()) {
        return false;
      }
      this.firebase.child('ParticleBase/users/devices/')
        .child(this.firebase.getAuth().uid).child(deviceJson.id)
        .set(deviceJson, function(error) {
          if (error) {
            callback(ParticleBase.ERROR_PARTICLEBASE_SAVE_DEVICE);
          } else {
            callback(null);
          }
        });
    };

    // Lists devices accessible with this user's access token
    // callback first parameter will be passed null on success or
    // one of the following errors:
    // ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN
    // ParticleBase.ERROR_PARTICLE_UNREACHABLE
    // ParticleBase.ERROR_PARTICLEBASE_INVALID_ACCESS_TOKEN
    // ParticleBase.ERROR_PARTICLE_SERVER_ERROR
    // second callback parameter will be a list of devices if no error occurred
    this.listDevices = function(callback) {
      if (!sanityCheck(callback)) {
        return false;
      }
      var xhr = buildXHR("GET", "/v1/devices");
      xhr.setRequestHeader("Authorization", "Bearer " + this.accessToken);
      var ref = this;
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 0 && xhr.status == 0) {
          callback(ParticleBase.ERROR_PARTICLE_UNREACHABLE, null);
          return false;
        };
        if (xhr.readyState == 4) {
          var data = xhr.status == 200 ? JSON.parse(xhr.responseText) : null;
          var status = getParticleXHRStatus(xhr.status);
          callback(status, data);
          return true;
        }
      };
      xhr.send();
      return true;
    };

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
      if (!sanityCheck(callback)) {
        return false;
      }
      var ref = this;
      this.listDevices(function(status, device_list) {
        callback(status);
      });
      return true;
    };


    // callback will be passed null on success, or one of the following statuses:
    // ParticleBase.ERROR_FIREBASE_NOT_LOGGED_IN
    // ParticleBase.ERROR_PARTICLE_UNREACHABLE
    // ParticleBase.ERROR_FIREBASE_COULD_NOT_SET_PARTICLE_TOKEN
    // ParticleBase.ERROR_PARTICLE_BAD_RESPONSE
    // If successful, the supplied Access Token Callback will be triggered
    // with status ParticleBase.SUCCESS_PARTICLEBASE_ACCESS_TOKEN
    this.bindAccessToken = function(particle_username, particle_password, callback) {
      if (!sanityCheck(callback)) {
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

  ParticleBase.ApiUrl = "https://api.particle.io";

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
