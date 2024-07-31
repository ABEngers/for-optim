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

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents('php://input'), true);
    if ($data['action'] === 'delete' && isset($data['id'])) {
        $id = $data['id'];
        $sql = "DELETE FROM irrigation_schedule WHERE id = ?";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("i", $id);
        if ($stmt->execute()) {
            echo json_encode(['message' => 'Schedule deleted successfully']);
        } else {
            echo json_encode(['error' => 'Failed to delete schedule']);
        }
        $stmt->close();
        $conn->close();
        exit();
    }
}

$sql = "SELECT id, start_time, duration, state FROM irrigation_schedule";
$result = $conn->query($sql);

$schedules = [];
if ($result->num_rows > 0) {
    while($row = $result->fetch_assoc()) {
        $schedules[] = $row;
    }
}

echo json_encode($schedules);
$conn->close();
?>
