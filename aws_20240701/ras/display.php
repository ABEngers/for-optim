// display_data.php
<?php
header('Content-Type: text/html; charset=UTF-8');

// データベース接続情報
$host = "database-1.cis1nmdsbnom.us-east-1.rds.amazonaws.com";
$dbname = "PBL_db";  // RDSインスタンス内のデータベース名
$username = "admin";
$password = "sasakilab2024";

// データベース接続
$conn = new mysqli($servername, $username, $password, $dbname);

// 接続確認
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// データの取得
$sql = "SELECT * FROM sensor_data ORDER BY timestamp DESC";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    echo "<table border='1'><tr><th>ID</th><th>Temperature</th><th>Humidity</th><th>Timestamp</th></tr>";
    while($row = $result->fetch_assoc()) {
        echo "<tr><td>" . $row["id"]. "</td><td>" . $row["temperature"]. "°C</td><td>" . $row["humidity"]. "%</td><td>" . $row["timestamp"]. "</td></tr>";
    }
    echo "</table>";
} else {
    echo "0 results";
}

// 接続を閉じる
$conn->close();
?>
