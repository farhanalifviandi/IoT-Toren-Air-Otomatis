CREATE DATABASE toren_air

USE toren_air

CREATE TABLE sensor_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    water_level FLOAT NOT NULL,
    flow_rate FLOAT NOT NULL,
    total_liters FLOAT NOT NULL,
    pump_status VARCHAR(10) NOT NULL,
    TIMESTAMP DATETIME DEFAULT CURRENT_TIMESTAMP
);
