var Firebase = require('firebase');
var FirebaseTokenGenerator = require("firebase-token-generator");
var aws = require('aws-sdk');
var ref = new Firebase('https://motorbike-tracker.firebaseio.com/');
var fs = require('fs');
var config = require('./config.json');
var global_context = null;
var global_event = null;
var task_count = 0;
var succeed = "";
var ses = null;
function checkTaskCount() {
    if (task_count <= 0) {
        global_context.succeed(succeed);
    }
}


function sendEmail(recipient, message, subject) {
    task_count++;
    var charset = "UTF-8";
    var params = {
        Destination : {
            ToAddresses : [recipient]
        },
        Message : {
            Body : {
                Html : {
                    Data : message,
                    Charset : charset
                }
            },
            Subject : {
                Data : subject,
                Charset : charset
            }
        },
        Source : "jychuah@gmail.com",
        ReplyToAddresses: ["jychuah@gmail.com"]
    };
    ses.sendEmail(params, function(error, data) {
        if (error) {
            console.log(error);
        }
        task_count--;
        checkTaskCount();
    });
}

function checkNotification() {
    task_count++;
    var event = global_event;
    aws.config.loadFromPath('awsconfig.json');
    ses = new aws.SES({apiVersion: '2010-12-01'});
    ref.child('config').child('imei').child(event.imei).once('value', function(dataSnapshot) {
        var tracker_name = dataSnapshot.val().name;
        var recipients = dataSnapshot.val().notifications;
        for (var recipient in recipients) {
            if (recipients[recipient][global_event.type] && recipients[recipient][global_event.type] === "yes") {
                var address = recipient.replace(/,/g, ".");
                task_count++;
                fs.readFile('./messages/' + event.type + '.html', 'utf8', function(error, data) {
                    if (error) {
                        console.log("No message for type " + event.type);
                        task_count--;
                        checkTaskCount();
                    } else {
                        var message = data.replace("###motorbike_tracker_url###", config.motorbike_tracker_url);
                        message = message.replace('###trip_url###', config.motorbike_tracker_url + "?trip=" + event.sequence);
                        message = message.replace('###tracker_name###', tracker_name);
                        sendEmail(address, message, config.subjects[event.type]);
                    }
                    task_count--;
                    checkTaskCount();
                });
            }
        }
        task_count--;
        checkTaskCount();
    });
}

function authHandler(error, result) {
    var event = global_event;
    if (error) {
        console.log("Auth Error");
        console.log(error);
        global_context.succeed("ERROR");
    } else {
        ref.child('config').child('imei').child(event.imei).child('uid').once('value',
            function(dataSnapshot) {
                if (dataSnapshot.val().trim() === event.uid) {
                    eventProcessor();
                } else {
                    console.log("UID and IMEI do not match records");
                    console.log(event);
                    console.log("Expected " + dataSnapshot.val());
                    console.log("Received " + event.uid);
                    global_context.succeed("ERROR");
                }
            },
            function (error) {
                if (error) {
                    console.log("Could not read config data");
                    console.log(error);
                    console.log(event);
                    global_context.succeed("ERROR");
                }
            })
    }
}

function eventProcessor() {
    var event = global_event;
    var timestamp = parseInt(event.timestamp);
    if (isNaN(timestamp)) {
        timestamp = Math.floor(Date.now() / 1000);
    }
    var tracker_event = {
        location : event.location,
        type : event.type,
        data : event.data
    };
    switch (event.type) {
      case 'provision' :
        task_count++;
        checkNotification();
        ref.child('config').child('imei').child(event.imei).child('uid').once('value',
                  function(dataSnapshot) {
                      if (dataSnapshot.val() == null) {
                          console.log("Provision data empty");
                          console.log(event);
                          global_context.succeed('ERROR');
                      } else {
                          task_count--;
                          succeed = dataSnapshot.val();
                          checkTaskCount();
                      }
                  },
                  function(error) {
                      console.log("Could not read provisioning data");
                      console.log(error);
                      console.log(event);
                      global_context.succeed('ERROR');
                  });
        break;
      case 'boot' :
        task_count++;
        var data = {
            events : { },
            imei : event.imei,
            name : "New trip",
            share : true,
            timestamp : timestamp
        }
        data.events[timestamp] = tracker_event;
        var new_sequence = ref.child('sequences').push();
        new_sequence.set(data, function(error) {
            if (error) {
                console.log("Could not write new sequence");
                console.log(error);
                console.log(event);
                global_context.succeed("ERROR");
            } else {
                event.sequence = new_sequence.key();
                checkNotification();
                task_count--;
                succeed = new_sequence.key();
                checkTaskCount();
            }
        });
        task_count++;
        ref.child('config').child('imei').child(event.imei).child('trips').child(timestamp).set(new_sequence.key(), function(error) {
            task_count--;
            checkTaskCount();
        });
        break;
      default :
        task_count++;
        checkNotification();
        ref.child('sequences').child(event.sequence).child('events').child(timestamp + '').set(tracker_event,
            function(error) {
                if (error) {
                    console.log("Error logging event");
                    console.log(event);
                    global_context.succeed("ERROR");
                } else {
                    succeed = "OK";
                }
                task_count--;
                checkTaskCount();
            }
        );
    }
}



exports.handler = function(event, context) {
    event.uid = event.uid.trim();
    event.imei = event.imei.trim();
    event.type = event.type.trim().toLowerCase();
    event.timestamp = event.timestamp.trim();
    event.location = event.location.trim();
    global_context = context;
    global_event = event;
    ref.authWithCustomToken(config.key, authHandler);

}
