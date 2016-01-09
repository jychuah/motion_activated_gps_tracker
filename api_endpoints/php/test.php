<?php

$json = '{ "uid" : "d76db2b8-be35-477c-a428-2623d523fbfd", "imei" : "865067020757418", "sequence" : "-K1Wd7U69hneS_u8oyjK", "location" : "-180.123456, -180.123456", "timestamp" : "SERVER", "type" : "sleep     ","data" : "                                        " }';
echo $json;
$ch = curl_init('http://webpersistent.com/motorbike-tracker/helper/post.php');
curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
curl_setopt($ch, CURLOPT_POSTFIELDS, $json);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, array(
    'Content-Type: application/json',
    'Content-Length: ' . strlen($json))
);
echo curl_exec($ch);
?>
