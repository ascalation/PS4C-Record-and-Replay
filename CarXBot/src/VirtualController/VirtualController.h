#pragma once
#include <Windows.h>
#include <ViGem/Client.h>
#include <iostream>
#include <vector>
#include <chrono>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::steady_clock::time_point;

class VirtualController
{
public:
	VirtualController();
	~VirtualController();

	void replay(std::vector<long long> timestamps, std::vector<DS4_REPORT> recordings);

private:
	TimePoint start_time;
	PVIGEM_CLIENT client;
	PVIGEM_TARGET controller;
};
