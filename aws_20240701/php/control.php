<?php
session_start();

// エラーメッセージの表示を有効にする
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

// ログインしているか確認
if (!isset($_SESSION['username'])) {
    header("Location: ../index.html");
    exit();
}

// WiFi接続状態の取得
function getWiFiStatus() {
    $output = shell_exec('iwgetid -r');
    $output = "on";
    return !empty($output);
}

$isConnected = getWiFiStatus();
$wifiIcon = $isConnected ? '../img/wifi_connected.png' : '../img/wifi_disconnected.png';

$host = "database-1.cis1nmdsbnom.us-east-1.rds.amazonaws.com";
$dbname = "PBL_db";
$username = "admin";
$password = "sasakilab2024";

// データベースから最新のセンサーデータを取得
function getLatestSensorData() {
    global $host, $username, $password, $dbname;
    $conn = new mysqli($host, $username, $password, $dbname);

    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    $sql = "SELECT temperature, humidity, timestamp FROM sensor_data ORDER BY timestamp DESC LIMIT 1";
    $result = $conn->query($sql);

    $data = $result->fetch_assoc();
    $conn->close();

    return $data;
}

// JSON形式でセンサーデータを返す
if ($_SERVER['REQUEST_METHOD'] === 'GET' && isset($_GET['action']) && $_GET['action'] === 'get_sensor_data') {
    echo json_encode(getLatestSensorData());
    exit();
}

// データベースから最新の電磁弁状態を取得
function getLatestValveState() {
    global $host, $username, $password, $dbname;
    $conn = new mysqli($host, $username, $password, $dbname);

    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    $sql = "SELECT state, timestamp FROM valve_state ORDER BY timestamp DESC LIMIT 1";
    $result = $conn->query($sql);

    $data = $result->fetch_assoc();
    $conn->close();

    return $data;
}

// 電磁弁状態の更新
$debugMessage = "";
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['state'])) {
    $state = $_POST['state'];

    $conn = new mysqli($host, $username, $password, $dbname);

    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    $sql = "INSERT INTO valve_state (state) VALUES (?)";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("s", $state);

    if ($stmt->execute()) {
        $debugMessage = "New record created successfully";
    } else {
        $debugMessage = "Error: " . $stmt->error;
    }

    $stmt->close();
    $conn->close();
}

$sensorData = getLatestSensorData();
$valveState = getLatestValveState();

$valveIcon = $valveState['state'] == "ON" ? '../img/valve_active.png' : '../img/valve_inactive.png';
?>
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="../css/custom_style.css">
    <title>コントロールパネル</title>
    <style>
        .card img {
            float: left;
            margin-right: 10px;
            width: 24px;
            height: 24px;
        }
    </style>
</head>
<body>
    <div class="container" id="mainContainer">
        <div class="header">
            <h1>コントロールパネル</h1>
        </div>
        <div class="status-bar">
            <div class="current-time">
                <img src="<?php echo $wifiIcon; ?>" alt="WiFi Icon" class="wifi-icon">
                <span id="currentTime"></span>
            </div>
        </div>
        <div class="grid-container">
            <div class="card" onclick="showDetails('valveDetails')">
                <img src="<?php echo $valveIcon; ?>" alt="Valve Icon" class="valve-icon">
                <h3>現在の電磁弁の状態</h3>
            </div>
            <div class="card" onclick="showDetails('sensorDetails')">
                <img src="../img/sensors.png" alt="Sensor">
                <h3>センサの値</h3>
            </div>
            <div class="card" onclick="showDetails('connectionDetails')">
                <img src="../img/connection.png" alt="Connection">
                <h3>通信状況</h3>
            </div>
            <div class="card" onclick="showDetails('toggleDetails')">
                <img src="../img/toggle.png" alt="Toggle">
                <h3>ON/OFF切替ボタン</h3>
            </div>
            <div class="card" onclick="showDetails('timerDetails')">
                <img src="../img/timer.png" alt="Timer">
                <h3>タイマー機能</h3>
            </div>
            <div class="card" onclick="showDetails('manualAutoDetails')">
                <img src="../img/manual_auto.png" alt="Manual/Auto">
                <h3>手動/自動切換え</h3>
            </div>
            <div class="card span-2-rows" onclick="showDetails('cameraDetails')">
                <img src="../img/camera.png" alt="Camera">
                <h3>カメラ映像</h3>
            </div>
            <div class="card span-2-cols" onclick="showDetails('manualLinkDetails')">
                <img src="../img/manual.png" alt="Manual">
                <h3>取扱説明書リンク</h3>
            </div>
            <div class="card span-2-cols" onclick="showDetails('faqDetails')">
                <img src="../img/faq.png" alt="FAQ">
                <h3>エラーが発生したときは</h3>
            </div>
        </div>
    </div>
    <div id="detailContainer" class="detail-container">
        <div id="valveDetails" class="detail-content">
            <h2>電磁弁の詳細</h2>
            <div class="valve-status">
                <p>電磁弁の状態: <?php echo $valveState['state']; ?></p>
                <p>最終更新: <?php echo $valveState['timestamp']; ?></p>
            </div>
            <p><?php echo $debugMessage; ?></p>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <div id="sensorDetails" class="detail-content">
            <h2>センサの詳細</h2>
            <p id="temperature">温度: <?php echo $sensorData['temperature']; ?> °C</p>
            <p id="humidity">湿度: <?php echo $sensorData['humidity']; ?> %</p>
            <p id="timestamp">最終更新: <?php echo $sensorData['timestamp']; ?></p>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <div id="connectionDetails" class="detail-content">
            <h2>通信状況の詳細</h2>
            <p>通信状況の詳細情報...</p>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <div id="toggleDetails" class="detail-content">
            <h2>ON/OFF切替ボタン</h2>
            <p>ここでON/OFFの切替を行います。</p>
            <form method="POST" action="">
                <button type="submit" name="state" value="ON">ON</button>
                <button type="submit" name="state" value="OFF">OFF</button>
            </form>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <div id="timerDetails" class="detail-content">
            <h2>タイマー機能の詳細</h2>
            <form id="scheduleForm">
                <label for="start_time">開始時間:</label>
                <input type="datetime-local" id="start_time" name="start_time" required>
                <label for="duration">継続時間（分）:</label>
                <input type="number" id="duration" name="duration" required>
                <input type="hidden" name="state" value="ON">
                <button type="submit">スケジュールを追加</button>
            </form>
            <button onclick="hideDetails()">閉じる</button>
            <h3>潅水スケジュール</h3>
            <ul id="scheduleList">
                <!-- スケジュール一覧をここに表示 -->
            </ul>
            <h3>潅水履歴</h3>
            <ul id="historyList">
                <!-- 履歴一覧をここに表示 -->
            </ul>
        </div>
        <div id="manualAutoDetails" class="detail-content">
            <h2>手動/自動切換えの詳細</h2>
            <p>手動/自動切換えの詳細情報...</p>
            <p>（今回は実装なし）</p>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <div id="cameraDetails" class="detail-content">
            <h2>カメラ映像の詳細</h2>
            <p>カメラ映像の詳細情報...</p>
            <p>（今回は実装なし）</p>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <div id="manualLinkDetails" class="detail-content">
            <h2>取扱説明書リンク</h2>
            <p>当社のリンク お客様係連絡番号などの情報です。</p>
            <p>（今回は実装なし）</p>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <div id="faqDetails" class="detail-content">
            <h2>エラーが発生したときは</h2>
            <p>エラーの対処方法などのFAQ的な要素...</p>
            <p>（今回は実装なし）</p>
            <button onclick="hideDetails()">閉じる</button>
        </div>
        <script src="../js/index.js"></script>
        <script>
            function showDetails(detailId) {
                document.getElementById('mainContainer').style.display = 'none';
                document.getElementById('detailContainer').style.display = 'flex';
                const details = document.querySelectorAll('.detail-content');
                details.forEach(detail => detail.style.display = 'none');
                document.getElementById(detailId).style.display = 'block';
            }

            function hideDetails() {
                document.getElementById('mainContainer').style.display = 'block';
                document.getElementById('detailContainer').style.display = 'none';
            }

            function updateTime() {
                const currentTimeElement = document.querySelector('.current-time span');
                if (currentTimeElement) {
                    const now = new Date();
                    currentTimeElement.textContent = now.toLocaleString('ja-JP', { year: 'numeric', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit', second: '2-digit' });
                }
            }

            document.addEventListener('DOMContentLoaded', function() {
                updateTime();
                setInterval(updateTime, 1000);
                updateSensorData();
                setInterval(updateSensorData, 30000); // 30秒ごとにセンサーデータを更新
                updateHistory(); // ページロード時に履歴を更新
                updateSchedules(); // ページロード時にスケジュールを更新
            });

            function updateSensorData() {
                fetch('?action=get_sensor_data')
                .then(response => {
                    if (!response.ok) {
                        throw new Error('Network response was not ok');
                    }
                    return response.json();
                })
                .then(data => {
                    document.getElementById('temperature').textContent = '温度: ' + data.temperature + ' °C';
                    document.getElementById('humidity').textContent = '湿度: ' + data.humidity + ' %';
                    document.getElementById('timestamp').textContent = '最終更新: ' + data.timestamp;
                })
                .catch(error => {
                    console.error('Error fetching sensor data:', error);
                });
            }

            function updateSchedules() {
                fetch('./get_schedule.php')
                .then(response => response.json())
                .then(data => {
                    const scheduleList = document.getElementById('scheduleList');
                    scheduleList.innerHTML = '';
                    data.forEach(schedule => {
                        const li = document.createElement('li');
                        li.innerHTML = `開始時間: ${schedule.start_time}, 継続時間: ${schedule.duration}分, 状態: ${schedule.state}
                        <button onclick="deleteSchedule(${schedule.id})">削除</button>`;
                        scheduleList.appendChild(li);
                    });
                })
                .catch(error => {
                    console.error('Error fetching schedule data:', error);
                });
            }

            function deleteSchedule(id) {
                fetch('./get_schedule.php', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({ action: 'delete', id: id })
                })
                .then(response => response.json())
                .then(data => {
                    alert(data.message || data.error);
                    updateSchedules();
                })
                .catch(error => {
                    console.error('Error deleting schedule:', error);
                });
            }

            function updateHistory() {
                fetch('./control.php?action=get_history')
                .then(response => response.json())
                .then(data => {
                    const historyList = document.getElementById('historyList');
                    historyList.innerHTML = '';
                    data.forEach(history => {
                        const li = document.createElement('li');
                        li.textContent = `開始時間: ${history.start_time}, 終了時間: ${history.end_time}, 状態: ${history.state}`;
                        historyList.appendChild(li);
                    });
                })
                .catch(error => {
                    console.error('Error fetching history data:', error);
                });
            }

            document.getElementById('scheduleForm').addEventListener('submit', function(event) {
                event.preventDefault();
                const formData = new FormData(this);
                const jsonData = JSON.stringify(Object.fromEntries(formData.entries()));

                fetch('./set_schedule.php', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: jsonData
                })
                .then(response => response.json())
                .then(data => {
                    console.log(data);  // レスポンスデータをコンソールに出力
                    alert(data.message || data.error);
                    updateSchedules();
                })
                .catch(error => {
                    console.error('Error setting schedule:', error);
                });
            });
        </script>
</body>
</html>
