<?php
// データベースに接続
$host = "database-1.cis1nmdsbnom.us-east-1.rds.amazonaws.com";
$dbname = "PBL_db";
$username = "admin";
$password = "sasakilab2024";
$conn = new mysqli($host, $username, $password, $dbname);

// 接続の確認
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// すべてのユーザーのパスワードをハッシュ化して更新
$sql = "SELECT username, password FROM users";
$result = $conn->query($sql);

while ($row = $result->fetch_assoc()) {
    $username = $row['username'];
    $password = $row['password'];
    
    // パスワードが既にハッシュ化されているかを確認
    if (!password_get_info($password)['algo']) {
        // パスワードをハッシュ化
        $hashed_password = password_hash($password, PASSWORD_DEFAULT);
        
        // データベースを更新
        $update_sql = "UPDATE users SET password = ? WHERE username = ?";
        $update_stmt = $conn->prepare($update_sql);
        $update_stmt->bind_param("ss", $hashed_password, $username);
        $update_stmt->execute();
        
        echo "ユーザー " . $username . " のパスワードがハッシュ化されました。<br>";
    }
}

// 接続を閉じる
$conn->close();
?>
