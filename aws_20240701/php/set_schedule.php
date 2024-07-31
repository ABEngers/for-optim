<?php
header('Content-Type: application/json');

$host = "database-1.cis1nmdsbnom.us-east-1.rds.amazonaws.com";
$dbname = "PBL_db";
$username = "admin";
$password = "sasakilab2024";

$conn = new mysqli($host, $username, $password, $dbname);
if ($conn->connect_error) {
    http_response_code(500);
    echo json_encode(['error' => 'Connection failed: ' . $conn->connect_error]);
    exit();
}

$data = json_decode(file_get_contents('php://input'), true);

if (isset($data['start_time']) && isset($data['duration']) && isset($data['state'])) {
    $start_time = $data['start_time'];
    $duration = $data['duration'];
    $state = $data['state'];

    $sql = "INSERT INTO irrigation_schedule (start_time, duration, state) VALUES (?, ?, ?)";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("sis", $start_time, $duration, $state);

    if ($stmt->execute()) {
        echo json_encode(["message" => "Schedule added successfully"]);
    } else {
        echo json_encode(["error" => "Error adding schedule: " . $stmt->error]);
    }

    $stmt->close();
} else {
    echo json_encode(["error" => "Invalid input"]);
}

$conn->close();
?>
