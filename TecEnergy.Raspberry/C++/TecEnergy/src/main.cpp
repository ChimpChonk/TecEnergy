#include <Arduino.h>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <algorithm> 
#include <iterator>   
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>

// Variable to keep track of whether the first pulse has been detected
bool first_pulse_detected = false;

// File path for storing the pulse log
const String log_file_path = "/PulseLog.json";

// DynamicJsonDocument for storing pulse log data (with an initial capacity of 1024 bytes)
DynamicJsonDocument pulse_log(1024);

// Global variable to keep track of the last time a post was requested
std::chrono::system_clock::time_point last_post_time;

// Structure to represent an energy meter
struct Meter {
    int pin;          // GPIO pin associated with the meter
    int count;        // Pulse count for the meter
    String bimayler_id; // Unique identifier for the meter
};

// Global variable to store the current meter's pin
int currentMeterPin;

// Map of energy meters, each identified by a color (Blue, White, Red, Green)
std::map<String, Meter> meters = {
    {"Blue", {12, 0, "184BDB2B-8355-4D7D-0E62-08DBE5B8372C"}},
    {"White", {27, 0, "BB7750A1-60E8-409B-3134-08DBE5CA4CCD"}},
    {"Red", {4, 0, "270C991B-9FA9-47FA-3135-08DBE5CA4CCD"}},
    {"Green", {5, 0, "5C03AAC4-779B-4AEB-3136-08DBE5CA4CCD"}}
};

// Method for posting data to the API
void postToAPI() {
    HTTPClient http;

    // API URL for posting data
    String api_url = "http://10.233.134.112:2050/api/EnergyData";
    
    // Set content type header for JSON data
    http.addHeader("Content-Type", "application/json");

    // Serialize JSON data from pulse_log to post_data
    String post_data;
    serializeJson(pulse_log, post_data);

    // Perform HTTP POST request
    int http_code = http.POST(post_data);

    // Check the result of the HTTP POST request
    if (http_code > 0) {
        Serial.printf("[HTTP] POST request to %s succeeded with HTTP code %d\n", api_url.c_str(), http_code);
        pulse_log.clear(); // Clear the pulse log after successful posting
    } else {
        Serial.printf("[HTTP] POST request to %s failed with HTTP code %d\n", api_url.c_str(), http_code);
    }

    // End the HTTP client session
    http.end();
}

// Callback function for handling pulse interrupts
void pulse_callback() {
    int gpio = currentMeterPin;

    // Find the meter associated with the current GPIO pin
    std::map<String, Meter>::iterator meter_it = std::find_if(meters.begin(), meters.end(), [gpio](const std::pair<const String, Meter>& meter_pair) {
        return meter_pair.second.pin == gpio;
    });

    // If the meter is found
    if (meter_it != meters.end()) {
        meter_it->second.count++; // Increment pulse count for the meter

        // Get the current time and format it
        auto current_time = std::chrono::system_clock::now();
        auto current_time_str = std::chrono::system_clock::to_time_t(current_time);
        auto tm_time = std::localtime(&current_time_str);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()).count();
        String current_time_formatted = String("");
        current_time_formatted += String(tm_time->tm_year + 1900) + "-";
        current_time_formatted += String(tm_time->tm_mon + 1) + "-";
        current_time_formatted += String(tm_time->tm_mday) + "T";
        current_time_formatted += String(tm_time->tm_hour) + ":";
        current_time_formatted += String(tm_time->tm_min) + ":";
        current_time_formatted += String(tm_time->tm_sec) + ".";
        current_time_formatted += String(milliseconds % 1000) + "Z";

        // Create a JSON log entry for the pulse_log
        JsonObject logEntry = pulse_log.createNestedObject();
        logEntry["EnergyMeterID"] = meter_it->second.bimayler_id;
        logEntry["DateTime"] = current_time_formatted;
        logEntry["AccumulatedValue"] = meter_it->second.count;

        // Print the current data to Serial
        Serial.print(current_time_formatted);
        Serial.print(" - Socket ");
        Serial.print(meter_it->first);
        Serial.print(" Pulse: ");
        Serial.println(meter_it->second.count);
    }

    // Check if it's time to post data to the API (every 10 seconds)
    auto now = std::chrono::system_clock::now();
    if (now >= last_post_time + std::chrono::seconds(10)) {
        postToAPI();
        last_post_time = now;
    }
}

// Wrapper function for the pulse interrupt callback
void pulseInterruptWrapper() {
    pulse_callback();
}

// Method to configure pins and set up interrupts
void setPins() {
    // Initialize SPIFFS (SPI Flash File System) for data storage
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }

    // Loop through each meter and configure its associated pin and interrupt
    for (const auto& meter : meters) {
        pinMode(meter.second.pin, INPUT); // Set pin mode to INPUT
        currentMeterPin = meter.second.pin; // Set the current meter's pin
        attachInterrupt(digitalPinToInterrupt(meter.second.pin), pulseInterruptWrapper, RISING);
        // Attach the pulseInterruptWrapper function to handle interrupts on the rising edge
    }
}

// Setup function runs once at the beginning of the program
void setup() {
    // Start serial communication with a baud rate of 115200
    Serial.begin(115200);

    // Initialize SPIFFS (SPI Flash File System) for data storage
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }

    // Call the setPins function to configure GPIO pins and interrupts
    setPins();
}

// Loop function runs repeatedly in a continuous loop
void loop() {
    // Introduce a delay of 1000 milliseconds (1 second)
    delay(1000);

    // Open or create a file on SPIFFS with the specified path in write mode ("w")
    File file = SPIFFS.open(log_file_path, "w");

    // Check if the file was successfully opened or created
    if (file) {
        // Serialize JSON data from pulse_log and write it to the file
        serializeJson(pulse_log, file);

        // Close the file to ensure data is written and resources are released
        file.close();
    }
}