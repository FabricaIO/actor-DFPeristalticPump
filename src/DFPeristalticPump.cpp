#include "DFPeristalticPump.h"

/// @brief Creates a peristaltic pump object
/// @param Pin The pin to use
/// @param ConfigFile The file name to store settings in
DFPeristalticPump::DFPeristalticPump(int Pin, String ConfigFile) {
	config_path = "/settings/sig/" + ConfigFile;
	// Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
}

/// @brief Starts a peristaltic pump object
/// @return True on success
bool DFPeristalticPump::begin() {
	// Set description
	Description.actionQuantity = 1;
	Description.type = "pump";
	Description.name = "Peristaltic Pump";
	Description.actions = {{"Dose", 0}};
	Description.id = 3;
	pump.setPeriodHertz(50);
	bool result = false;
	// Create settings directory if necessary
	if (!checkConfig(config_path)) {
		// Set defaults
		result = setConfig(R"({"pumpSpeed": 180, "doseTime": 2000, "pin":)" + String(current_config.pin) + R"(, "threshold": 50, "autoParameter": "", "autoEnabled": false, "activeLow": true, "taskName": "AutoPump", "taskPeriod": 10000})");
	} else {
		// Load settings
		result = setConfig(Storage::readFile(config_path));
	}
	return result;
}

/// @brief Receives an action
/// @param action The action to process (only option is 0 for get data)
/// @param payload Not used
/// @return A plaintext response with the data
std::tuple<bool, String> DFPeristalticPump::receiveAction(int action, String payload) {
	dose();
	return { true, R"({"success": true})" };
}

/// @brief Activates the pump for the configured amount of time
void DFPeristalticPump::dose() {
	pump.write(current_config.pumpSpeed);
	delay(current_config.doseTime);
	pump.write(90);
}

/// @brief Runs the auto pump task
/// @param elapsed The amount of time, in ms, since this was last called
void DFPeristalticPump::runTask(long elapsed) {
	totalElapsed += elapsed;
	if (totalElapsed > TaskDescription.taskPeriod) {
		totalElapsed = 0;
		for (const auto& m : SensorManager::measurements) {
			if (m.parameter == current_config.autoParameter) {
				if (current_config.activeLow) {
					if (m.value < current_config.threshold) {
						dose();
						return;
					}
				} else {
					if (m.value > current_config.threshold) {
						dose();
						return;
					}
				}
			}
		}
	}
}

/// @brief Enables the auto pump
/// @param enable True to enable, false to disable 
/// @return True on success
bool DFPeristalticPump::enableAuto(bool enable) {
	current_config.autoEnabled = enable;
	return enableTask(enable);
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DFPeristalticPump::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["pumpSpeed"] = current_config.pumpSpeed;
	doc["autoParameter"] = current_config.autoParameter;
	doc["doseTime"] = current_config.doseTime;
	doc["threshold"] = current_config.threshold;
	doc["pin"] = current_config.pin;
	doc["autoEnabled"] = current_config.autoEnabled;
	doc["activeLow"] = current_config.activeLow;
	doc["taskName"] = TaskDescription.taskName;
	doc["taskPeriod"] = TaskDescription.taskPeriod;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config The JSON config to use
/// @return True on success
bool DFPeristalticPump::setConfig(String config) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("Deserialization failed: "));
		Serial.println(error.f_str());
		return false;
	}
	// Assign loaded values
	current_config.pumpSpeed = doc["pumpSpeed"].as<int>();
	current_config.autoParameter = doc["autoParameter"].as<String>();
	current_config.doseTime = doc["doseTime"].as<int>();
	current_config.threshold = doc["threshold"].as<int>();
	current_config.pin = doc["pin"].as<int>();
	current_config.autoEnabled = doc["autoEnabled"].as<bool>();
	current_config.activeLow = doc["activeLow"].as<bool>();
	TaskDescription.taskName = doc["taskName"].as<std::string>();
	TaskDescription.taskPeriod = doc["taskPeriod"].as<long>();
	pump.detach();
	pump.attach(current_config.pin, 500, 2500);
	if (!saveConfig(config_path, config)) {
		return false;
	}
	return enableAuto(current_config.autoEnabled);
}