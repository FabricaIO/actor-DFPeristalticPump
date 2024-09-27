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
#include <PeriodicTask.h>

/// @brief Allows for control of a peristaltic pump
class DFPeristalticPump : public Actor, public PeriodicTask {
	private:
		/// @brief Holds pump configuration
		struct {
			/// @brief Speed at which pump runs
			int pumpSpeed;

			/// @brief Time in ms for each dose
			int doseTime;

			/// @brief Pin for pump
			int pin;

		    /// @brief threshold below which to deliver a dose
		    int threshold;
			
			/// @brief Name of the sensor to use for auto-triggering
			String autoParameter;

			/// @brief Whether to enable auto triggering or not
			bool autoEnabled;

			/// @brief Whether the pump should be active on the above the threshold (false) or below (true)
			bool activeLow; 
			
		} current_config;

		/// @brief Full path to config file
		String path;
		
		/// @brief Servo object
		Servo pump;

		void dose();
		bool enableAuto(bool enabled);
	public:
		DFPeristalticPump(int Pin, String ConfigFile = "DFPump.json");
		bool begin();
		std::tuple<bool, String> receiveAction(int action, String payload = "");
		void runTask(long elapsed);
		String getConfig();
		bool setConfig(String config);
};