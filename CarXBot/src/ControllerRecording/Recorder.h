#pragma once
#include <windows.h>
#include <ViGEm/Client.h>
#include <vector>
#include <chrono>

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
	void save_controller_state(const DS4_REPORT report, TimePoint processing_start, TimePoint processing_time);
	// Getters for timestamps
	std::vector<long long> get_timestamps() const;
	// Getters for recordings
	std::vector<DS4_REPORT> get_recordings() const;

private:
	TimePoint start_time;
	std::vector<long long> timestamps{};
	std::vector<DS4_REPORT> recordings{};
};
