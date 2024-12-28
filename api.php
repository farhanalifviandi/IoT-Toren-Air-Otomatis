<?php
$dataFile = 'data.json';

$host = 'localhost';
$dbname = 'toren_air';
$username = 'root';
$password = '';

try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    die("Database connection failed: " . $e->getMessage());
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $input = file_get_contents('php://input');
    $data = json_decode($input, true);

    if ($data) {
        file_put_contents($dataFile, json_encode($data, JSON_PRETTY_PRINT));
        $pumpStatus = $data['waterLevel'] <= 5 ? 'ON' : 'OFF';

        $stmt = $pdo->prepare("INSERT INTO sensor_data (water_level, flow_rate, total_liters, pump_status) VALUES (:water_level, :flow_rate, :total_liters, :pump_status)");
        $stmt->execute([
            ':water_level' => $data['waterLevel'],
            ':flow_rate' => $data['flowRate'],
            ':total_liters' => $data['totalLiters'],
            ':pump_status' => $pumpStatus
        ]);

        echo json_encode(["message" => "Data received successfully"]);
    } else {
        http_response_code(400);
        echo json_encode(["error" => "Invalid data"]);
    }
} elseif ($_SERVER['REQUEST_METHOD'] === 'GET') {
    if (file_exists($dataFile)) {
        echo file_get_contents($dataFile);
    } else {
        echo json_encode(["waterLevel" => 0, "flowRate" => 0, "totalLiters" => 0]);
    }
} else {
    http_response_code(405);
    echo json_encode(["error" => "Method not allowed"]);
}
?>
