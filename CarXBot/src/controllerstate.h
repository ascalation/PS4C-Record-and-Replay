#pragma once
#include <Windows.h>
#include <ViGEm/Client.h>

struct controllerState {
	long long timestamp;
	DS4_REPORT report;
};