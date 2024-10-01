/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman and Garth Johnson
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/ 
* ESP32Servo: https://github.com/madhephaestus/ESP32Servo
*
* DFRobot Peristaltic Pump: https://www.dfrobot.com/product-1698.html
*
* Contributors: Sam Groveman, Garth Johnson
*/

#pragma once
#include <Arduino.h>
#include <Actor.h>
#include <SensorManager.h>
#include <Storage.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

/// @brief Allows for control of a peristaltic pump
class DFPeristalticPump : public Actor {
	protected:
		/// @brief Holds pump configuration
		struct {
			/// @brief Speed at which pump runs
			int pumpSpeed;

			/// @brief Time in ms for each dose
			int doseTime;

			/// @brief Pin for pump
			int pin;
			
		} current_config;

		/// @brief Full path to config file
		String config_path;
		
		/// @brief Servo object
		Servo pump;

		void dose();
	public:
		DFPeristalticPump(int Pin, String ConfigFile = "DFPump.json");
		bool begin();
		std::tuple<bool, String> receiveAction(int action, String payload = "");
		String getConfig();
		bool setConfig(String config, bool save);
};