<!DOCTYPE html>
<html>

    <head>
        <title>Motorbike Tracker</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="stylesheet" href="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css">
        <link rel="stylesheet" href="css/motorbike-tracker.css">
        <link href="css/roboto.min.css" rel="stylesheet">
        <link href="css/material.min.css" rel="stylesheet">
        <link href="css/ripples.min.css" rel="stylesheet">
        <link rel="stylesheet" href="css/bootstrap-select.css">
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
        <script src="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>
        <script src="https://cdn.firebase.com/js/client/2.3.1/firebase.js"></script>
        <script src="js/jquery.bootstrap-growl.min.js"></script>
        <script src="js/bootstrap-select.min.js"></script>
        <script src="js/elements.js"></script>
        <script>
<?php
require('vendor/autoload.php');
use Firebase\Token\TokenException;
use Firebase\Token\TokenGenerator;

  if ($_GET['trip']) {

    // Enable anonymous authentication and put in your firebase secret
    $secret = 'your_firebase_secret';

    $sequence = $_GET['trip'];

    try {
        $generator = new TokenGenerator($secret);
        $token = $generator->setOption("admin", true)->create();
        $sequence_share_check_url ='https://motorbike-tracker.firebaseio.com/sequences/' . $sequence . '/share.json?auth=' . $token;
        $ch = curl_init($sequence_share_check_url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
        $result = curl_exec($ch);
        if (strpos($result, "true") >= 0) {
            $generator = new TokenGenerator($secret);
            $expires   = time() + 3600;
            $token = $generator->setData(['uid' => 'uid', 'provider' => 'anonymous'], ['expires' => $expires])->create();
            echo "var token = '" . $token . "'; var shared_trip='" . $sequence . "';";
        }
    } catch (TokenException $e) {
    }
  } else {
  }
?>

$(document).ready(function() {
    logout();
     $('.selectpicker').selectpicker();
    $(function () {
        $('[data-toggle="tooltip"]').tooltip();
    });
    try {
      if (token != undefined && shared_trip != undefined) {
          ref.authWithCustomToken(token, function(error, authData) {
              if (error) {
                  $.bootstrapGrowl('There was an error viewing this trip. Try refreshing the page.', { type : 'danger '});
              } else {
                  events[shared_trip] = { };
                  displaySequence(shared_trip);
              }
          });
      } else {
      }
    } catch (referenceError) {

    }
});

        </script>
    </head>

    <body>
        <nav class="navbar navbar-default navbar-fixed-top">
            <div class="container-fluid">
                <!-- Brand and toggle get grouped for better mobile display -->
                <div class="navbar-header">
                    <button type="button" class="navbar-toggle collapsed" data-toggle="collapse"
                    data-target="#bs-example-navbar-collapse-1" aria-expanded="false">
                        <span class="sr-only">Toggle navigation</span>
                        <span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                    </button>
                    <a class="navbar-brand" href="#">Motorbike Tracker</a>
                </div>
                <!-- Collect the nav links, forms, and other content for toggling -->
                <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
                    <ul class="nav navbar-nav navbar-right">
                        <li>
                            <a href="#">About</a>
                        </li>
                        <li class="dropdown">
                            <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button"
                            aria-haspopup="true" aria-expanded="false"><span id="sign_in_text">Sign in</span> <span class="caret"></span></a>
                            <ul class="dropdown-menu">
                                <li>
                                    <a href="#" role="button" data-toggle="modal" data-target="#loginModal"
                                    id="sign_in_button">Sign in</a>
                                </li>
                                <li>
                                    <a href="#" role="button" class="hidden" data-toggle="modal" data-target="#changePasswordModal"
                                    id="account_option_button">Change password</a>
                                </li>
                                <li role="separator" class="divider"></li>
                                <li>
                                    <a href="#" role="button" data-toggle="modal" data-target="#createAccountModal"
                                    id="create_account_button">Create account</a>
                                </li>
                                <li>
                                    <a href="#" role="button" data-toggle="modal" data-target="#resetPasswordModal"
                                    id="reset_password_button">Reset password</a>
                                </li>
                                <li>
                                    <a href="#" role="button" class="hidden" id="sign_out_button" onClick="logout()">Sign out</a>
                                </li>
                            </ul>
                        </li>
                    </ul>
                </div>
                <!-- /.navbar-collapse -->
            </div>
            <!-- /.container-fluid -->
        </nav>
        <nav class="navbar navbar-default"></nav>
        <div class="container-fluid row" style="height: 90%;">
            <div class="col-md-3" id="leftpanel">
                <div class="panel-group">
                    <div class="panel panel-success">
                        <div class="panel-heading">
                            <h4 class="panel-title">
                                <a data-toggle="collapse" href="#tracker_list">Trackers</a>
                            </h4>
                        </div>
                        <div class="panel-collapse collapse.in">
                            <div class="panel-body">This is a list of Motorbike Trackers that are currently tied to your account.
                                Click one to view its data.</div>
                            <div class="panel-body" id="tracker_list"></div>
                            <div class="panel-footer" id="claim_tracker_button"><a herf="#" data-toggle="modal" data-target="#claimTrackerModal" onclick="claimTrackerClickHandler();">+ Claim a tracker</a></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="col-md-6 container" id="midpanel" style="height: 100%;">Right panel</div>
            <div class="col-md-3" id="rightpanel">
                <div class="panel-group">
                    <div class="panel panel-success">
                        <div class="panel-heading">
                            <h4 class="panel-title">Trip Data</h4>
                        </div>
                        <div class="panel-collapse collapse" id="sequence_panel">
                            <div class="panel-body" id="tracker_list">
                                <div align=center>
                                    <h4 id="sequence_name"></h4>
                                    <img src="img/map-path.png" />
                                    <br />
                                    <i>Events</i>
                                    <br />
                                    <br />
                                </div>
                                <div class="list-group" id="event_list"></div>
                            </div>
                            <div class="panel-footer"><a href="#" id="tripOptions">Trip options</a></div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div id="loginModal" class="modal fade" role="dialog">
            <div class="modal-dialog">
                <!-- Modal content-->
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <h4 class="modal-title">Log in with your e-mail and password</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="usr_email">E-mail address</label>
                            <input type="text" class="form-control" id="usr_email">
                            <label for="pwd">Password</label>
                            <input type="password" class="form-control" id="pwd">
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal" onclick='login()'>Sign in</button>
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    </div>
                </div>
            </div>
        </div>
        <div id="changePasswordModal" class="modal fade" role="dialog">
            <div class="modal-dialog">
                <!-- Modal content-->
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <h4 class="modal-title">Change password</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="currentpwd">Current password</label>
                            <input type="password" class="form-control" id="currentpwd">
                            <label for="setpwd">New password</label>
                            <input type="password" class="form-control" id="setpwd">
                            <label for="setpwdagain">New password again</label>
                            <input type="password" class="form-control"
                            id="setpwdagain">
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal" onclick="changePassword();">Change password</button>
                        <button type="button" class="btn btn-default"
                        data-dismiss="modal">Cancel</button>
                    </div>
                </div>
            </div>
        </div>
        <div id="createAccountModal" class="modal fade" role="dialog">
            <div class="modal-dialog">
                <!-- Modal content-->
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <h4 class="modal-title">Create an account</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="new_usr_email">E-mail address</label>
                            <input type="text" class="form-control" id="new_usr_email">
                            <label for="newpwd">Password</label>
                            <input type="password" class="form-control" id="newpwd">
                            <label for="pwdagain">Password again</label>
                            <input type="password" class="form-control" id="pwdagain">
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal" onClick="createAccount()">Create account</button>
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    </div>
                </div>
            </div>
        </div>
        <div id="resetPasswordModal" class="modal fade" role="dialog">
            <div class="modal-dialog">
                <!-- Modal content-->
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <h4 class="modal-title">Send yourself a password reset e-mail</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="usr_email">E-mail address</label>
                            <input type="text" class="form-control" id="reset_email">
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal" onClick="sendResetEmail();">Send e-mail</button>
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    </div>
                </div>
            </div>
        </div>
        <div id="claimTrackerModal" class="modal fade" role="dialog">
            <div class="modal-dialog">
                <!-- Modal content-->
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <h4 class="modal-title">Claim a tracker</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="new_tracker_name">Tracker Name</label>
                            <input type="text" class="form-control" id="new_tracker_name" value="New tracker">
                            <label for="new_tracker_imei">Tracker IMEI</label>
                            <input type="text" class="form-control" id="new_tracker_imei">
                            <label for="new_tracker_imei_again">Tracker IMEI again</label>
                            <input type="text" class="form-control" id="new_tracker_imei_again">
                            <label for="new_tracker_uid">Paste this UID into your tracker source code</label>
                            <input type="text" class="form-control" id="new_tracker_uid" readonly>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal" onClick="claimTracker();">Claim tracker</button>
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    </div>
                </div>
            </div>
        </div>

        <div id="trackerSettingsModal" class="modal fade" role="dialog">
            <div class="modal-dialog">
                <!-- Modal content-->
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <h4 class="modal-title">Tracker Settings</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="tracker_settings_name">Tracker Name</label>
                            <input type="text" class="form-control" id="tracker_settings_name" value="New tracker">
                            <br />
                            <label for="tracker_settings_gps_interval">GPS Update Interval</label>
                            <br />
                            <select class="selectpicker" id="tracker_settings_gps_interval">
                              <option value="1">Once per minute</option>
                              <option value="5">Once every 5 minutes</option>
                              <option value="15">Once every 15 minutes</option>
                              <option value="60">Once per hour</option>
                              <option value="360">Once every 6 hours</option>
                              <option value="720">Once every 12 hours</option>
                              <option value="1440">Once per day</option>
                            </select>
                            <br /><br />
                            <label for="tracker_settings_table">Tracker notifications</label>
                            <table class="table" id="tracker_settings_table">
                              <thead>
                                <tr>
                                  <th>E-mail address</th>
                                  <th align=center><img src="img/poweron.png" data-toggle="tooltip" data-placement="top" title="Power on" /></th>
                                  <th align=center><img src="img/sleep.png" data-toggle="tooltip" data-placement="top" title="Sleep mode" /></th>
                                  <th align=center><img src="img/wake.png" data-toggle="tooltip" data-placement="top" title="Motion detected" /></th>
                                  <th align=center><img src="img/gps-fix.png" data-toggle="tooltip" data-placement="top" title="GPS location update" /></th>
                                  <th align=center><img src="img/temperature.png" data-toggle="tooltip" data-placement="top" title="Temperature alert" /></th>
                                  <th align=center><img src="img/lowbattery.png" data-toggle="tooltip" data-placement="top" title="Battery alert" /></th>
                                  <th align=center><img src="img/gps-on.png" data-toggle="tooltip" data-placement="top" title="Provisioning data received" /></th>
                                  <th></th>
                                </tr>
                              </thead>
                              <tbody id="table_settings_notifications">
                              </tbody>
                            </table>
                            <a href="#" role="button" onClick="addTrackerNotificationRecipient();">+ Add a notification recipient</a>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal" onClick="saveTrackerSettings();">Save</button>
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    </div>
                </div>
            </div>
        </div>

        <div id="tripOptionsModal" class="modal fade" role="dialog">
            <div class="modal-dialog">
                <!-- Modal content-->
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal">&times;</button>
                        <h4 class="modal-title">Trip Options</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="trip_options_name">Trip Name</label>
                            <input type="text" class="form-control" id="trip_options_name" value="" />
                            <br />
                            <label for="trip_options_name">Enable sharing</label>
                            <input type="checkbox" class="form-control" id="trip_sharing_check" />
                            <br />
                            <label for="trip_share_url">You can share this trip's URL:</label>
                            <input type="text" class="form-control" id="trip_share_url" readonly />
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal" id="saveTripSettingsButton">Save</button>
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    </div>
                </div>
            </div>
        </div>

        <script src="js/ripples.min.js"></script>
        <script src="js/material.min.js"></script>
        <script>
            function showPosition(position) {
              var location = new google.maps.LatLng(position.coords.latitude, position.coords.longitude);
              map.panTo(location);
            }
            function initMap() {
              map = new google.maps.Map(document.getElementById('midpanel'), {
                  center: {
                      lat: -34.397,
                      lng: 150.644
                  },
                  zoom: 8
              });
              if (navigator.geolocation) {
                navigator.geolocation.getCurrentPosition(showPosition);
              }
            }
        </script>
        <script async defer src="https://maps.googleapis.com/maps/api/js?key=AIzaSyDYbxV7J-PbqzdRjYTR7VRhFuf0j-QCz60&callback=initMap"></script>
    </body>

</html>
