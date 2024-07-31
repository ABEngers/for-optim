<?php
session_start();

// RDS接続情報
$host = "database-1.cis1nmdsbnom.us-east-1.rds.amazonaws.com";
$dbname = "PBL_db";  // RDSインスタンス内のデータベース名
$username = "admin";
$password = "sasakilab2024";

// ユーザー入力の取得
$input_username = $_POST['username'];
$input_password = $_POST['password'];

// RDSに接続
$conn = new mysqli($host, $username, $password, $dbname);

// 接続の確認
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// SQLクエリの準備
$sql = "SELECT * FROM users WHERE username = ?";
$stmt = $conn->prepare($sql);
if ($stmt === false) {
    die("Prepare failed: " . $conn->error);
}
$stmt->bind_param("s", $input_username);

// クエリの実行
$stmt->execute();
$result = $stmt->get_result();

// 結果の確認
if ($result->num_rows > 0) {
    // ユーザーが存在する場合
    $user = $result->fetch_assoc();
    echo "デバッグ: ユーザーが見つかりました<br>";
    echo "デバッグ: データベースから取得したユーザー名: " . $user['username'] . "<br>";
    
    // デバッグ: パスワードのハッシュを表示
    echo "デバッグ: データベースから取得したパスワードハッシュ: " . $user['password'] . "<br>";
    
    // パスワードの確認
    if (password_verify($input_password, $user['password'])) {
        echo "デバッグ: パスワードが一致しました<br>";
        // ログイン成功
        $_SESSION['username'] = $input_username;
        header("Location: control.php");
        exit();
    } else {
        // パスワードが間違っている場合
        echo "デバッグ: パスワードが一致しません<br>";
        echo "デバッグ: 入力されたパスワード: " . $input_password . "<br>";
        echo "ユーザー名またはパスワードが違います";
    }
} else {
    // ユーザーが存在しない場合
    echo "デバッグ: ユーザーが存在しません<br>";
    echo "ユーザー名またはパスワードが違います";
}

// 接続を閉じる
$stmt->close();
$conn->close();
?>
