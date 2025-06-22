#pragma once
#include <windows.h>
#include <ViGEm/Client.h>
#include <vector>
#include <chrono>
#include "..\controllerstate.h"

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::steady_clock::time_point;

class Recorder
{
public:
	Recorder() = default;
	~Recorder() = default;

	// Sets start_time to 0
	void set_start_time();
	// saves current controller inputs with timestamp
	void save_controller_state(DS4_REPORT report);
	// Getters for timestamps
	std::vector<controllerState> get_saves() const ;

private:
	TimePoint start_time;
	std::vector<controllerState> saves{};
};
