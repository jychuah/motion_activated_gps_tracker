/**
* elements.js
*
* js for Motorbike Tracker frontend. Change firebase_url to reflect your
* Firebase URL.
*/
var firebase_url = "https://your-firebase.firebaseio.com";

var ref = new Firebase(firebase_url);
var currentSequence = null;
var map;
var events = { };
var email = "";


function sequenceClickHandler(event) {
    var target = event.target;
    while ($(target).attr('sequence_id') == undefined) {
        target = $(target).parent();
        if (target == null) {
            return false;
        }
    }
    var sequence_id = $(target).attr('sequence_id');
    var children = $(target).parent().children('a');
    for (var i = 0; i < children.length; i++) {
        var inspect = $(children[i]).attr('sequence_id');
        if (!(inspect === sequence_id)) {
            $(children[i]).children('img').attr('src', 'img/map.png');
        }
    }

    $(target).children('img').attr('src', 'img/map-path.png');
    $("#tripOptions").attr('data-toggle', 'modal');
    $("#tripOptions").attr('data-target', '#tripOptionsModal');
    $("#tripOptions").unbind('click');
    $("#tripOptions").attr('sequence_id', sequence_id);
    $("#tripOptions").click(tripOptionsClickHandler);
    $("#saveTripSettingsButton").unbind('click');
    $("#saveTripSettingsButton").click(saveTripSettings);
    $("#saveTripSettingsButton").attr('sequence_id', sequence_id);
    displaySequence(sequence_id);
};

function tripOptionsClickHandler(event) {
    var sequence_id = $(event.target).attr('sequence_id');
    var current_name = $("#sequence_name").html().trim();
    var share_url = "http://webpersistent.com/motorbike-tracker/index.php?trip=" + sequence_id;
    $("#trip_share_url").val(share_url);
    $("#trip_options_name").val(current_name);
    $("#trip_sharing_check").prop('checked', sequence_share_data[sequence_id]);

}

function saveTripSettings(event) {
    var sequence_id = $(event.target).attr('sequence_id');
    ref.child('sequences').child(sequence_id).child('name').set($("#trip_options_name").val().trim());
    ref.child('sequences').child(sequence_id).child('share').set($("#trip_sharing_check").prop('checked'));
}

var Event = function(params, sequence_id) {
    /*
        params = { type : "gps", data : "93, -31", timestamp: 1444091176 }
    */
    var processor = EventProcessors[params.type];
    var a = $('<a></a>').addClass('list-group-item').attr('href', '#').attr('sequence_id', sequence_id).attr('timestamp', params.timestamp);
    if (processor.onClick != undefined) {
        a.click(processor.onClick);
    } else {
        a.click(eventClick);
    }
    $('<img />').attr('src', processor.img).addClass('img_icon').appendTo(a);
    $('<span></span>').html(processor.summarize(params)).appendTo(a);
    $('<span></span>').html(getDateTimeString(params.timestamp)).addClass('badge').appendTo(a);
    if (typeof(processor.process) != undefined && processor.process != null) {
        processor.process(params, sequence_id);
    } else {
        plot(params, sequence_id);
    }
    return a;
};
var Sequence = function(params) {
    /*
        params = { name : "Sequence 1", date : "1/2/2015", sequence_id : "-kk-123456" }
    */
    var a = $('<a></a>');
    a.addClass('list-group-item').attr('href', '#').html("<span>" + params.name + "</span> - <i>" + params.date + "</i>").attr('sequence_id', params.sequence_id);
    a.attr('role', 'button');
    $('<img />').attr('src', 'img/map.png').addClass('img_icon').appendTo(a);
    a.click(sequenceClickHandler);
    return a;
};
var Tracker = function(params) {
    /*
        params  = { imei : 1234, name : "tracker name" }
    */
    var div = $('<div class="panel panel-info" id="imei_' + params.imei + '"/>');
    var panel_heading = $('<div class="panel-heading" />');
    var h4 = $('<h4 class="panel-title" />');
    var a = $('<a data-toggle="collapse" href="#imei_info_' + params.imei + '">' + params.name + '</a>');
    a.appendTo(h4);
    a.attr('imei', params.imei);
    a.click(loadTrackerData);
    h4.appendTo(panel_heading);
    panel_heading.appendTo(div);

    var imei_info = $('<div id="imei_info_' + params.imei + '" class="panel-collapse collapse" />');
    var imei_sequence_count = $('<div class="panel-body" id="imei_panel_body_' + params.imei + '"></div>').html("<i>Loading</i>");
    imei_sequence_count.appendTo(imei_info);
    var imei_sequence_list = $('<div class="list-group" id="imei_sequence_list_' + params.imei + '" />');
    imei_sequence_list.appendTo(imei_info);
    var panel_footer = $('<div class="panel-footer"><a href="#" imei="' + params.imei + '" data-toggle="modal" data-target="#trackerSettingsModal">Settings</a></div>');
    panel_footer.children('a').click(trackerSettingsClickHandler);
    panel_footer.appendTo(imei_info);
    imei_info.appendTo(div);
    div.imei_sequence_count = imei_sequence_count;
    div.a = a;
    div.addSequence = function(s) {
        imei_sequence_list.prepend(s);
        var count = imei_sequence_list.children().length;
        if (count == 0) {
            imei_sequence_count.html("<i>No trips</i>");
        }
        else if (count == 1) {
            imei_sequence_count.html(count + " trip");
        }
        else {
            imei_sequence_count.html(count + " trips");
        }
    }
    return div;
};

function singleInfoWindow(sequence_id, timestamp) {
    for (var event in events[sequence_id]) {
        if (events[sequence_id][event].marker != undefined &&
            events[sequence_id][event].marker != null) {
            events[sequence_id][event].infowindow.close();
        }
    }
    if (events[sequence_id][timestamp].marker != undefined &&
        events[sequence_id][timestamp].marker != null) {
        events[sequence_id][timestamp].infowindow.open(map, events[sequence_id][timestamp].marker);
    }
}


var NotificationRecipient = function(params) {
    var key = Object.keys(params)[0];
    var settings = params[key];
    var email = key.replace(/,/g,".");
    var tr = $("<tr key='" + key + "'></tr>");
    $("<td></td>").append($("<input type='email' value='" + email + "' />")).appendTo(tr);
    $("<td align=center></td>").append($("<input type='checkbox' value='boot'" + (settings.boot === "yes" ? "checked" : "") + " />")).appendTo(tr);
    $("<td align=center></td>").append($("<input type='checkbox' value='sleep'" + (settings.sleep === "yes" ? "checked" : "") + " />")).appendTo(tr);
    $("<td align=center></td>").append($("<input type='checkbox' value='wake'" + (settings.wake === "yes" ? "checked" : "") + " />")).appendTo(tr);
    $("<td align=center></td>").append($("<input type='checkbox' value='gps'" + (settings.gps === "yes" ? "checked" : "") + " />")).appendTo(tr);
    $("<td align=center></td>").append($("<input type='checkbox' value='temperature'" + (settings.temperature === "yes" ? "checked" : "") + " />")).appendTo(tr);
    $("<td align=center></td>").append($("<input type='checkbox' value='battery'" + (settings.battery === "yes" ? "checked" : "") + " />")).appendTo(tr);
    $("<td align=center></td>").append($("<input type='checkbox' value='provision'" + (settings.provision === "yes" ? "checked" : "") + " />")).appendTo(tr);
    $("<td align=center></td>").append($("<a href='#'><strong>remove</strong></a>").click(removeRecipient)).appendTo(tr);
    return tr;
}

function removeRecipient(event) {
    var target = $(event.target);
    var key = $(event.target).attr('key');
    while (key == undefined && target != undefined) {
        target = target.parent();
        key = target.attr('key');
    }
    target.remove();
}

function trackerSettingsClickHandler(event) {
    $("#table_settings_notifications").empty();
    var imei = $(event.target).attr('imei');
    $("#tracker_settings_name").attr('imei', imei);
    $("#tracker_settings_name").val(trackers[imei].name);
    $("#tracker_settings_gps_interval").selectpicker('val', trackers[imei].rate);
    for (var recipient in trackers[imei].notifications) {
        var data = { };
        data[recipient] = trackers[imei].notifications[recipient];
        var n = new NotificationRecipient(data);
        $("#table_settings_notifications").append(n);
    }
}

function addTrackerNotificationRecipient() {
    var shim = {
      "some@email,com" : {
        battery : "yes",
        boot: "yes",
        gps: "no",
        provision: "yes",
        sleep: "yes",
        wake: "yes",
        temperature: "yes",
      }
    }
    var n = new NotificationRecipient(shim);
    $("#table_settings_notifications").append(n);
}

function saveTrackerSettings() {
    var imei = $("#tracker_settings_name").attr('imei');
    ref.child('users').child(ref.getAuth().uid).child('imei').child(imei).set($("#tracker_settings_name").val().trim());
    ref.child('config/imei').child(imei).child('rate').set(parseInt($("#tracker_settings_gps_interval").val()));
    ref.child('config/imei').child(imei).child('name').set($("#tracker_settings_name").val().trim());
    var notifications = { };
    var rows = $("#table_settings_notifications").children('tr');
    for (var i = 0; i < rows.length; i++) {
        var tr = $(rows[i]);
        var email = tr.find("input[type='email']").val();
        email = email.trim().toLowerCase().replace(/\./g,",");
        notifications[email] = { };
        var checkboxes = tr.find("input[type!='email']");
        for (var j = 0; j < checkboxes.length; j++) {
            var input = $(checkboxes[j]);
            var key = input.val();
            notifications[email][key] = (input.is(":checked") ? "yes" : "no");
        }
    }
    ref.child('config/imei').child(imei).child('notifications').set(notifications, function(error) {
        if (error) {
            $.bootstrapGrowl("There was a problem saving your settings", { type : 'danger' });
        }
    });
}

var currentLocationMarker = null;
var currentLocationInfowindow = null;

function showLastLocation() {
    if (currentLocationMarker == null) {
        currentLocationMarker = new google.maps.Marker();
        currentLocationInfowindow = new google.maps.InfoWindow();
        currentLocationMarker.setZIndex(10);
        currentLocationInfowindow.setContent("Last known location");
        currentLocationMarker.addListener('click', function() {
            currentLocationInfowindow.open(map, currentLocationMarker);
        });
    }

    currentLocationMarker.setMap(null);
    currentLocationInfowindow.close();
    var keys = Object.keys(events[currentSequence]).sort();
    for (var i = keys.length - 1; i > 0; i--) {
        var event = events[currentSequence][keys[i]];
        if (event.marker != undefined &&
            event.marker != null) {
            currentLocationMarker.setPosition(event.marker.getPosition());
            currentLocationMarker.setMap(map);
            currentLocationInfowindow.open(map, currentLocationMarker);
        //    map.panTo(event.marker.getPosition());
            return;
        }
    }
}

var minLat = null;
var maxLat = null;
var minLng = null;
var maxLng = null;

function plot(params, sequence_id) {
    // check before
    if (params.location == undefined || params.location == null) {
        return;
    }
    var split = params.location.split(',');
    var location = new google.maps.LatLng(split[0], split[1]);

    if (minLat == null) {
        minLat = parseFloat(split[0]);
        maxLat = parseFloat(split[0]);
        minLng = parseFloat(split[1]);
        maxLng = parseFloat(split[1]);
        map.panTo(location);
    } else {
        var lat = parseFloat(split[0]);
        var lng = parseFloat(split[1]);
        minLat = Math.min(minLat, lat);
        maxLat = Math.max(maxLat, lat);
        minLng = Math.min(minLng, lng);
        maxLng = Math.min(maxLng, lng);
        var southwest = new google.maps.LatLng(minLat, minLng);
        var northeast = new google.maps.LatLng(maxLat, maxLng);
        var bounds = new google.maps.LatLngBounds(southwest, northeast);
        map.fitBounds(bounds);
        map.panToBounds(bounds);
    }

    var keys = Object.keys(events[sequence_id]).sort();
    var position = keys.indexOf(params.timestamp);

    var previous_exists = false;

    if (position > 0 &&
        events[sequence_id][keys[position - 1]].marker != undefined &&
        events[sequence_id][keys[position - 1]].marker != null) {
        var previous = events[sequence_id][keys[position - 1]];
        if (previous.marker.getPosition().equals(location)) {
            // append to previous
            events[sequence_id][params.timestamp].marker = previous.marker;
            events[sequence_id][params.timestamp].infowindow = previous.infowindow;
            previous_exists = true;
        } else {
            // TODO draw line
            var points =[ previous.marker.getPosition(), location];
            var path = new google.maps.Polyline({
                path : points,
                geodesic : true,
                strokeColor: "#8888FF",
                strokeOpacity: 0.6,
                strokeWeight: 5
            });
            path.setMap(map);

        }
    }
    if (!previous_exists) {
        var infowindow = new google.maps.InfoWindow({
            content: "<div class='list-group'></div>",
        });
        var marker = new google.maps.Marker({
            position: location,
            map: map,
            zIndex : 5,
            icon: {
                path : google.maps.SymbolPath.CIRCLE,
                scale: 7,
                strokeColor: "#FFFFFF",
                fillColor: "#8888FF",
                fillOpacity: 1,
                strokeOpacity: 0.8,
                strokeWeight: 3
            },
            title: getDateTimeString(params.timestamp)
        });

        marker.addListener('click', function() {
            singleInfoWindow(sequence_id, params.timestamp);
        });
        // Tag gMap objects and store in events
        events[sequence_id][params.timestamp].marker = marker;
        events[sequence_id][params.timestamp].infowindow = infowindow;
    }
    var processor = EventProcessors[params.type];
    var a = $('<div></div>').addClass('list-group-item');
    $('<img />').attr('src', processor.img).addClass('img_icon').appendTo(a);
    $('<span></span>').html(processor.summarize(params) + "&nbsp;&nbsp;").appendTo(a);
    $('<span></span>').html(getDateTimeString(params.timestamp)).addClass('badge').appendTo(a);
    var tempdiv = $('<div></div>');
    var realdiv = $(events[sequence_id][params.timestamp].infowindow.getContent());
    realdiv.appendTo(tempdiv);
    realdiv.append(a);
    events[sequence_id][params.timestamp].infowindow.setContent(tempdiv.html());

    showLastLocation();
}

function eventClick(event) {
    var target = event.target;
    while ($(target).attr('sequence_id') == undefined) {
        target = $(target).parent();
        if (target == null) {
            return false;
        }
    }
    var sequence_id = $(target).attr('sequence_id');
    var timestamp = $(target).attr('timestamp');

    if (events[sequence_id][timestamp].marker != undefined &&
        events[sequence_id][timestamp].marker != null) {
          map.setCenter(events[sequence_id][timestamp].marker.getPosition());
    }
    singleInfoWindow(sequence_id, timestamp);
}

var EventProcessors = {
    'provision' : {
        img: 'img/gps-on.png',
        summarize: function(params) {
            return "Tracker provisioned";
        },
    },
    'boot': {
        img: 'img/poweron.png',
        summarize: function(params) {
            return "Power on";
        },
    },
    'gps': {
        img: 'img/gps-fix.png',
        summarize: function(params) {
            return "Location fix";
        },
    },
    'sleep': {
        img: 'img/sleep.png',
        summarize: function(params) {
            return "Going to sleep";
        }
    },
    'wake': {
        img: 'img/wake.png',
        summarize: function(params) {
            return "Wake! Tracker disturbed!";
        }
    },
    'battery': {
        img: 'img/lowbattery.png',
        summarize: function(params) {
            var percent = parseInt(params.data);
            return "Battery at " + percent + "%";
        }
    },
    'temp': {
        img: 'img/temperature.png',
        summarize: function(params) {
            var value = parseInt(params.data);
            return "Battery at " + (value / 10) + "F";
        }
    }
};



function login() {
    email = $("#usr_email").val();
    var password = $("#pwd").val();
    ref.authWithPassword({
        email: email,
        password: password
    }, function(error, authData) {
        if (error) {
            $.bootstrapGrowl("Login failed", {
                type: 'danger'
            });
        }
        else {
            completeLogin();
        }
    });
}

function validateEmail(email) {
    var re = /^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,6}(?:\.[a-z]{2})?)$/i;
    return re.test(email);
}

function sendResetEmail() {
    var reset_email = $("#reset_email");
    if (!validateEmail(reset_email)) {
        $.bootstrapGrowl("Enter a valid e-mail address", {
            type: 'danger'
        });
        return false;
    }
    ref.resetPassword({
        email: reset_email
    }, function(error) {
        $.bootstrapGrowl("Reset e-mail sent", {
            type: 'info'
        });
    });
}

function createAccount() {
    var new_usr_email = $("#new_usr_email").val();
    var newpwd = $("#newpwd").val();
    var pwdagain = $("#pwdagain").val();

    $("#new_usr_email").val("");
    $("#newpwd").val("");
    $("#pwdagain").val("");
    if (!validateEmail(new_usr_email)) {
        $.bootstrapGrowl("Enter a valid e-mail address", {
            type: 'danger'
        });
        return false;
    }
    if (!(newpwd === pwdagain)) {
        $.bootstrapGrowl("Passwords must match", {
            type: 'danger'
        });
        return false;
    }
    ref.createUser({
        email: new_usr_email,
        password: newpwd
    }, function(error, userData) {
        if (error) {
            $.bootstrapGrowl("Couldn't create account", {
                type: 'danger'
            });
        }
        else {
            $.bootstrapGrowl("Account created", {
                type: 'info'
            });
            completeLogin();
        }
    });
}

function changePassword() {
    var currentpwd = $("#currentpwd").val();
    var setpwd = $("#setpwd").val();
    var setpwdagain = $("#setpwdagain").val();

    $("#currentpwd").val("");
    $("#setpwd").val("");
    $("#setpwdagain").val("");
    if (!(setpwd === setpwdagain)) {
        $.bootstrapGrowl("New passwords must match", {
            type: 'danger'
        });
        return false;
    }
    var email = ref.getAuth().password.email;
    ref.changePassword({
        email: email,
        oldPassword: currentpwd,
        newPassword: setpwd
    }, function(error) {
        if (error) {
            switch (error.code) {
            case "INVALID_PASSWORD":
                $.bootstrapGrowl("Incorrect current password", {
                    type: 'danger'
                });
                break;
            case "INVALID_USER":
                $.bootstrapGrowl("Invalid user", {
                    type: 'danger'
                });
                break;
            default:
                $.bootstrapGrowl("Password couldn't be changed", {
                    type: 'danger'
                });
            }
        }
        else {
            $.bootstrapGrowl("Password changed", {
                type: 'info'
            });
        }
    });
}

function loadTrackerData(event) {
    var imei = $(event.target).attr('imei');
    ref.child('config/imei/' + imei + '/trips').on('value', function(dataSnapshot) {
        if (dataSnapshot.val() == null) {
            $(trackers[imei].element.imei_sequence_count).html('<i>No trips</i>');
        }
    });
    ref.child('config/imei/' + imei).child('notifications').on('value', function(dataSnapshot) {
        trackers[imei].notifications = dataSnapshot.val();
    });
    ref.child('config/imei/' + imei).child('rate').on('value', function(dataSnapshot) {
        trackers[imei].rate = dataSnapshot.val();
    });
    ref.child('config/imei/' + imei + '/trips').orderByKey().on('child_added', function(dataSnapshot) {
        // sequence added to tracker
        var timestamp = dataSnapshot.key();
        var sequence_id = dataSnapshot.val();
        trackers[imei].sequences[timestamp] = sequence_id;
        ref.child('sequences/' + sequence_id + "/name").once('value', function(dataSnapshot) {
            var name = dataSnapshot.val();
            var date = getDateString(timestamp);
            if (events[sequence_id] == undefined || events[sequence_id] == null) {
                events[sequence_id] = { };
            }
            trackers[imei].element.addSequence(
                new Sequence({ name : name, date : date, sequence_id : sequence_id })
            );
        });
        ref.child('sequences/' + sequence_id + "/name").on('value', function(dataSnapshot) {
            //  sequence name change event
            var new_sequence_name = dataSnapshot.val();
            if (currentSequence === dataSnapshot.ref().parent().key()) {
                $("#sequence_name").html(new_sequence_name);
            }
            var selector = 'a[sequence_id="' + sequence_id + '"] span';
            $(trackers[imei].element).find(selector).html(new_sequence_name);
        });
        ref.child('sequences').child(sequence_id).child('share').on('value', function(dataSnapshot) {
            sequence_share_data[sequence_id] = dataSnapshot.val();
        });
    });
    $(event.target).unbind("click");

}

var sequence_share_data = { };

var trackers = {};

function addTracker(dataSnapshot) {
    var imei = dataSnapshot.key();
    var name = dataSnapshot.val();
    var t = new Tracker({ imei : imei, name : name});
    trackers[imei] = { imei : imei, name : name, element : t, sequences : { } };
    $("#tracker_list").append(t);
    $.bootstrapGrowl("Found tracker: " + name, { type: 'info'});
}

function claimTrackerClickHandler() {
    $("#new_tracker_uid").val(ref.getAuth().uid);
}

function getUser() {
    var userRef = ref.child('users').child(ref.getAuth().uid);
    userRef.once('value', function(dataSnapshot) {
    }, function(err) {
        // code to handle read error
        $.bootstrapGrowl("No trackers found", { type: 'info' });
    });
    userRef.child('imei').on('child_changed', function(dataSnapshot) {
        var imei = dataSnapshot.key();
        var new_name = dataSnapshot.val();
        $(trackers[imei].element.a).html(new_name);
        trackers[imei].name = new_name;
    });
    userRef.child('imei').on('child_added', function(dataSnapshot) {
        // imei added
        addTracker(dataSnapshot);
    });
    userRef.child('imei').on('child_removed', function(dataSnapshot) {
        // TODO user data update
        console.log("user data child_removed - key: " + dataSnapshot.key());
        console.log(dataSnapshot.val());
    });
}

function completeLogin() {
    $.bootstrapGrowl("Sign in complete", {
        type: 'info'
    });
    $("#sign_in_text").html(ref.getAuth().password.email);
    $("#sign_in_button").addClass('hidden');
    $("#account_option_button").removeClass('hidden');
    $("#create_account_button").addClass('hidden');
    $("#reset_password_button").addClass('hidden');
    $("#sign_out_button").removeClass('hidden');
    $("#claim_tracker_button").removeClass('hidden');
    $("#usr_email").val("");
    $("#pwd").val("");
    getUser();
}

function logout() {
    ref.unauth();
    $("#sign_in_text").html("Sign in");
    $("#sign_in_button").removeClass('hidden');
    $("#account_option_button").addClass('hidden');
    $("#create_account_button").removeClass('hidden');
    $("#reset_password_button").removeClass('hidden');
    $("#sign_out_button").addClass('hidden');
    $("#tracker_list").empty();
    $("#claim_tracker_button").addClass('hidden');
    $("#usr_email").val("");
    $("#pwd").val("");
}

function displaySequence(sequence_id) {
    if (currentSequence != null) {
        ref.child('sequences/' + currentSequence + "/events").off();
    }
    currentSequence = sequence_id;
    $("#event_list").empty();
    $('#sequence_panel').collapse('show');
    ref.child('sequences/').child(sequence_id).child("name").on('value', function(dataSnapshot) {
        $('#sequence_name').html(dataSnapshot.val());
    });
    ref.child('sequences/' + sequence_id + "/events").orderByKey().on('child_added', function(dataSnapshot) {
        var event = dataSnapshot.val();
        var params = { timestamp : dataSnapshot.key(), type : event["type"] };
        if (event.location != undefined && event.location != null) {
            params["location"] = event.location;
        }
        if (event.data != undefined && event.data != null) {
            params["data"] = event["data"];
        }
        events[sequence_id][params.timestamp] = { };
        events[sequence_id][params.timestamp].params = params;
        var e = new Event(params, sequence_id);
        events[sequence_id][params.timestamp].element = e;
        $("#event_list").prepend(e);
    });
};
function isNumeric(n) {
    return !isNaN(parseFloat(n)) && isFinite(n);
}

function claimTracker() {
    if ($("#new_tracker_imei").val().trim() !== $("#new_tracker_imei_again").val().trim()) {
        $.bootstrapGrowl("IMEIs don't match!", { type : 'danger'} );
        return;
    }
    var imei = $("#new_tracker_imei").val().replace(/-|\s/g,"").trim();
    var sum = 0;

    for (var i = 0; i < imei.length - 1; i++) {
        if (!isNumeric(imei.charAt(0))) {
            $.bootstrapGrowl("IMEI must be numeric!", { type : 'danger'});
            return;
        }
        if (i % 2 == 1) {
            var digitdoubled = (parseInt(imei.charAt(i) + "") * 2) + "";
            for (var j = 0; j < digitdoubled.length; j++) {
                sum += parseInt(digitdoubled.charAt(j));
            }
        } else {
            sum += parseInt(imei.charAt(i));
        }
    }
    if ((sum + parseInt(imei.charAt(imei.length - 1))) % 10 == 0) {
        ref.child('config/imei/' + imei).child('uid').set(ref.getAuth().uid, function(error) {
            if (error) {
                $.bootstrapGrowl("That IMEI has already been claimed.", { type : 'danger '});
            } else {
                var value = {};
                value[imei] = $("#new_tracker_name").val().trim();
                ref.child('users/' + ref.getAuth().uid + '/imei').child(imei).set($("#new_tracker_name").val().trim(), function(error) {
                    if (error) {
                        $.bootstrapGrowl("Provisioned the tracker, but unable to lay claim to it. Contact technical support.", { type : 'danger'});
                    } else {
                        $("#new_tracker_name").val('');
                        $("#new_tracker_imei").val('');
                        $("#new_tracker_imei_again").val('');
                        $("#new_tracker_uid").val('');
                        $.bootstrapGrowl("Provisioned IMEI " + imei + " - turn it on now to test it.", { type : 'info' });
                        var fbemail = email.replace(/\./g,",");
                        var settings = {
                            battery : "no",
                            boot : "yes",
                            location : "no",
                            provision : "yes",
                            sleep : "yes",
                            wake : "yes",
                            temperature: "no"
                        };
                        ref.child('config/imei/' + imei).child('notifications').child(fbemail).set(settings, function(error) {
                            if (error) {
                                $.bootstrapGrowl("Unable to set initial notifications for tracker", { type : 'success'});
                            }
                        });
                        ref.child('config/imei/' + imei).child('rate').set(600, function(error) {
                            if (error) {
                                $.bootstrapGrowl("Unable to set rate for tracker", { type : 'success'});
                            }
                        });
                    }
                });
            }
        });
    } else {
        $.bootstrapGrowl("IMEI invalid. Check the digits please.", { type : 'danger'});
    }
}


function getDateString(unixtime) {
    var d = new Date(unixtime * 1000);
    return d.getMonth() + "/" + d.getDate() + "/" + d.getFullYear();
};

function getTimeString(unixtime) {
    var d = new Date(unixtime * 1000);
    return d.getHours() + ":" + d.getMinutes();
};

function getDateTimeString(unixtime) {
    return getDateString(unixtime) + " - " + getTimeString(unixtime);
};
