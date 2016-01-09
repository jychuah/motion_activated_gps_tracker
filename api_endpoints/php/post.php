<?php
$firebase_url = "https://motorbike-tracker.firebaseio.com/";
$motorbike_tracker_url = "http://webpersistent.com/motorbike-tracker/index.php";
$secret = 'huYU7ZUD5e6AbAk1ML9MsMZekQZ6kbTCRBgv71yx';

function firebase_curl($child, $token, $method, $data) {
    global $firebase_url;
    $url = $firebase_url . $child . ".json?auth=" . $token;
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $method);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    if ($method !== 'GET') {
      $datastring = json_encode($data);
      curl_setopt($ch, CURLOPT_POSTFIELDS, $datastring);
      curl_setopt($ch, CURLOPT_HTTPHEADER, array(
          'Content-Type: application/json', 'Content-Length: ' . strlen($datastring)
      ));
    }
    $result = curl_exec($ch);
    $result_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    if ($result_code != 200) {
        die("ERROR");
    }
    return $result;
}

function sendMail($to, $type, $tracker_name, $sequence) {
    global $motorbike_tracker_url;
    $message = file_get_contents('./emails/' . $type . '.html');
    $message = str_replace('###trip_url###', $motorbike_tracker_url . '?trip=' . $sequence, $message);
    $message = str_replace('###motorbike_tracker_url###', $motorbike_tracker_url, $message);
    $message = str_replace('###tracker_name###', $tracker_name, $message);
    $to = str_replace(',', '.', $to);

    $subject = $tracker_name . ' sending a ' . $type . ' notification';

    $headers = "From: Motorbike Tracker <admin@webpersistent.com>\r\n";
    $headers .= "MIME-Version: 1.0\r\n";
    $headers .= "Content-Type: text/html; charset=ISO-8859-1\r\n";
    mail($to, $subject, $message, $headers);
}

require '../vendor/autoload.php';

use Firebase\Token\TokenException;
use Firebase\Token\TokenGenerator;



// get fields
$post_data = json_decode(file_get_contents('php://input'), true);

$uid = trim($post_data["uid"]);
$imei = trim($post_data["imei"]);
$timestamp = trim($post_data["timestamp"]);
$type = trim($post_data["type"]);
$data = trim($post_data["data"]);
$sequence = trim($post_data["sequence"]);
$location = trim($post_data["location"]);

if (intval($timestamp) == 0) {
    $timestamp = time();
}

http_response_code(200);
// get Firebase login token
try {
    $generator = new TokenGenerator($secret);
    $token = $generator
        ->setOption('admin', true)
        ->create();
} catch (TokenException $e) {
    die("ERROR");
}

// get imei config
$result = firebase_curl('config/imei/' . $imei, $token, 'GET', null);
$imei_config = json_decode($result, true);

// check UID matches post request
if ($imei_config['uid'] !==  $uid ) {
    die("ERROR");
}

$eventjson = array();
$eventjson['location'] = $location;
$eventjson['data'] = $data;
$eventjson['type'] = $type;

foreach($imei_config['notifications'] as $email => $notification_types) {
    if ($notification_types[$type] === 'yes') {
        sendMail($email, $type, $imei_config['name'], $sequence);
    }
}

if ($type === "provision") {
    die($result);
}

if ($type === "boot") {
    $jsondata = array();
    $jsondata['events'] = array();
    $jsondata['imei'] = $imei;
    $jsondata['name'] = 'New trip';
    $jsondata['share'] = true;
    $jsondata['timestamp'] = $timestamp;
    $jsondata['events'][$timestamp] = $eventjson;
    $result = firebase_curl("sequences", $token, 'POST', $jsondata);
    $result_array = json_decode($result, true);
    die($result_array['name']);
}

if ($type === "rate") {
    $result = firebase_curl('config/imei/' . $imei . '/rate', $token, 'GET', null);
    $result = str_replace('"', '', $result);
    die($result);
}

// log all other types of eventjson
firebase_curl('sequences/' . $sequence . '/events/' . $timestamp, $token, 'PUT', $eventjson);
die("OK");
/*
// new sequence
if ($event === "NEW SEQUENCE") {
    // post new sequence with one "tracker activated" event
    $jsondata = array("timestamp" => strval(time()), "events" => array(strval(time()) => array("event" => "Tracker Activated", "data" => "none")));
    $ch = curl_init('https://motorbike-tracker.firebaseio.com/uid/' . $uid . '/imei/' . $imei . '/sequences.json?auth=' . $token);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");

} else {
    if ($event === "TRACKER WAKE") {
        // send alerts
    }
    // put new event in sequence
    $jsondata = array("event" => $event, "data" => $data);
    $ch = curl_init('https://motorbike-tracker.firebaseio.com/uid/' . $uid . '/imei/' . $imei . '/sequences/' . $sequence . '/events/' . strval(time()) . '.json?auth=' . $token);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "PUT");
}

$data_string = json_encode($jsondata);
curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, array(
    'Content-Type: application/json',
    'Content-Length: ' . strlen($data_string))
);

$result = curl_exec($ch);
$result_array = json_decode($result, true);
if ($event === "NEW SEQUENCE") {
    if (array_key_exists('name', $result_array)) {
        echo $result_array['name'];
    } else {
        echo "ERROR";
    }
} else {
    if (array_key_exists("data", $result_array) && array_key_exists("event", $result_array) && $result_array["data"] === $data && $result_array["event"] === $event) {
        echo "OK";
    } else {
        echo $result_array["data"] . " : " . $data;
        echo $result_array["event"] . " : " . $event;
        echo "ERROR";
    }
}
*/
?>
