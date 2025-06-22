#include "Recorder.h"
#include <Windows.h>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::steady_clock::time_point;

void Recorder::set_start_time() {
	start_time = Clock::now();
}

void Recorder::save_controller_state(DS4_REPORT report) {
	// Safe timestamp
	saves.push_back(
		{
			std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time).count(),
			report
		}
	);
}

std::vector<controllerState> Recorder::get_saves() const {
	return saves;
}
