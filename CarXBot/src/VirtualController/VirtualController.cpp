#include "VirtualController.h"
#include <thread>


VirtualController::VirtualController() {
    client = nullptr;
    controller = nullptr;

	client = vigem_alloc();
    if (vigem_connect(client) != VIGEM_ERROR_NONE) {
        std::cerr << "ViGEm connection failed!" << std::endl;
        return;
    }
    controller = vigem_target_ds4_alloc();
    if (vigem_target_add(client, controller) != VIGEM_ERROR_NONE) {
        std::cerr << "Failed to add target!" << std::endl;
        return;
    }
}

VirtualController::~VirtualController() {
    vigem_target_remove(client, controller);
    vigem_target_free(controller);
    vigem_free(client);
}

void VirtualController::replay(std::vector<controllerState> saves) {
    if (saves.empty()) {
        std::cerr << "Invalid recordings or timestamps!" << std::endl;
        return;
    }

    start_time = Clock::now();

    for (size_t i = 0; i < saves.size(); ++i) {
        auto target_time = start_time + std::chrono::milliseconds(saves[i].timestamp);

        while (Clock::now() < target_time) {
            _mm_pause();
        }

        const auto& report = saves[i].report;
        auto result = vigem_target_ds4_update(client, controller, report);

#ifdef _DEBUG
        std::cout << "Updated DS4 controller at index " << i
            << " (timestamp " << saves[i].timestamp << "ms)\n";
#endif

        if (result != VIGEM_ERROR_NONE) {
            std::cerr << "Failed to update controller at timestamp "
                << saves[i].timestamp << ": " << result << std::endl;
        }
    }

    std::cout << "Replay completed." << std::endl;
}


void VirtualController::replay_legacy(std::vector<controllerState> saves) {
    if (saves.empty()) {
        std::cerr << "Invalid recordings or timestamps!" << std::endl;
        return;
    }
    start_time = Clock::now();
    for (size_t i = 0; i < saves.size(); ++i) {
        auto target_time = start_time + std::chrono::milliseconds(saves[i].timestamp);

        auto now = Clock::now();

        auto wait_duration = target_time - now;

        if (wait_duration > std::chrono::milliseconds(2)) {
            std::this_thread::sleep_for(wait_duration - std::chrono::milliseconds(1));
        }

        while (Clock::now() < target_time) {
            _mm_pause();
        }

        const auto& report = saves[i].report;
        auto result = vigem_target_ds4_update(client, controller, report);

        #ifdef _DEBUG
        std::cout << "Updated DS4 controller at index " << i << " (timestamp " << timestamps[i] << "ms)\n";
        #endif

        if (result != VIGEM_ERROR_NONE) {
            std::cerr << "Failed to update controller at timestamp " << saves[i].timestamp << ": " << result << std::endl;
        }
    }

    std::cout << "Replay completed." << std::endl;
}
