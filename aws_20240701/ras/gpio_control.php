<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents('php://input'), true);
    $pin = $data['pin'];
    $state = $data['state'];

    // Raspberry Piにリクエストを送信
    $url = 'http://192.168.50.80:5000/control_gpio';
    $postData = json_encode(array('pin' => $pin, 'state' => $state));
    $options = array(
        'http' => array(
            'header' => "Content-type: application/json\r\n",
            'method' => 'POST',
            'content' => $postData,
            'timeout' => 30  // タイムアウトを30秒に設定
        ),
    );
    $context = stream_context_create($options);

    $result = @file_get_contents($url, false, $context);

    if ($result === FALSE) {
        echo json_encode(array('error' => 'Failed to connect to Raspberry Pi'));
    } else {
        echo $result;
    }
} else {
    echo json_encode(array('error' => 'Invalid request'));
}
?>
