<?php

$data = json_decode(file_get_contents('php://input'), true);
$date = date("Y-m-d H:i:s");
$errorCode = 0;

$station = $data["station"];
$battery = $data["stats"]["batt"];
$reading = $data["stats"]["read"];
$statstemp = $data["stats"]["temp"];
$rssi = $data["stats"]["rssi"];
$reads = $data["reads"];

$servername = "<SERVERNAME>";
$username = "<USERNAME>";
$password = "<PASSWORD>";
$dbname = "<DB_NAME>";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
  die("Connection failed: " . $conn->connect_error);
}

foreach ($reads as $read) {
  $hive = $read["hive"];
  $temp = $read["temp"];
  $weight = $read["weight"];
  $insertBeehive = "INSERT INTO `beehive` (`station`, `hive`, `datetime`, `temperature`, `weight`) VALUES ('" . $station . "', '" . $hive . "', '" . $date . "', '" . $temp . "', '" . $weight . "');";
  if (isset($data) && $conn->query($insertBeehive) === TRUE) {
    //echo $date . " => new record created successfully";
  } else {
    echo $date . " => error: " . $sql . " => " . $conn->error;
  }
}

$insertStats = "INSERT INTO `stats` (`station`, `datetime`, `temperature`, `battery`, `reading`, `rssi`) VALUES ('" . $station . "', '" . $date . "', '" . $statstemp . "', '" . $battery . "', '" . $reading . "', '" . $rssi . "');";
$updateLastupdate = "INSERT INTO `lastupdate` (`station`, `datetime`) VALUES('" . $station . "', '" . $date . "') ON DUPLICATE KEY UPDATE `datetime`='" . $date . "';";

if (isset($data) && $conn->query($insertStats) === TRUE && $conn->query($updateLastupdate) === TRUE) {
  echo $date . " => new record created successfully";
} else {
  echo $date . " => error: " . $sql . " => " . $conn->error;
}

$conn->close();

?>