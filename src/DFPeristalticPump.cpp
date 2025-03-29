#include "DFPeristalticPump.h"

/// @brief Creates a peristaltic pump object
/// @param Name The device name
/// @param Pin The pin to use
/// @param ConfigFile The file name to store settings in
DFPeristalticPump::DFPeristalticPump(String Name, int Pin, String ConfigFile) : Actor(Name) {
	config_path = "/settings/act/" + ConfigFile;
	current_config.pin = Pin;
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
	Description.actions = {{"Dose", 0}};
	pump.setPeriodHertz(50);
	bool result = false;
	// Create settings directory if necessary
	if (!checkConfig(config_path)) {
		// Set defaults
		result = setConfig(R"({"pumpSpeed": 180, "doseTime": 2000, "pin":)" + String(current_config.pin) + "}", true);
	} else {
		// Load settings
		result = setConfig(Storage::readFile(config_path), false);
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
	Logger.println("Dosing pump...");
	pump.write(current_config.pumpSpeed);
	delay(current_config.doseTime);
	pump.write(90);
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DFPeristalticPump::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["Name"] = Description.name;
	doc["pumpSpeed"] = current_config.pumpSpeed;
	doc["doseTime"] = current_config.doseTime;
	doc["pin"] = current_config.pin;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool DFPeristalticPump::setConfig(String config, bool save) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return false;
	}
	// Assign loaded values
	Description.name = doc["Name"].as<String>();
	current_config.pumpSpeed = doc["pumpSpeed"].as<int>();
	current_config.doseTime = doc["doseTime"].as<int>();
	current_config.pin = doc["pin"].as<int>();
	pump.detach();
	pump.attach(current_config.pin, 500, 2500);
	if (save) {
		return saveConfig(config_path, config);
	}
	return true;
}