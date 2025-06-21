#include "Recorder.h"
#include <Windows.h>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::steady_clock::time_point;

void Recorder::set_start_time() {
	start_time = Clock::now();
}

void Recorder::save_controller_state(const DS4_REPORT report, TimePoint processing_start, TimePoint processing_time) {
	// Safe timestamp
	auto pt = std::chrono::duration_cast<std::chrono::milliseconds>(processing_start - processing_time).count();
	timestamps.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time).count() - pt);
	// Safe ControllerState as XUSB_report
	recordings.push_back(report);
}

std::vector<long long> Recorder::get_timestamps() const {
	return timestamps;
}

std::vector<DS4_REPORT> Recorder::get_recordings() const {
	return recordings;
}
