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

void VirtualController::replay(std::vector<long long> timestamps, std::vector<DS4_REPORT> recordings) {
    if (timestamps.empty() || recordings.empty() || timestamps.size() != recordings.size()) {
        std::cerr << "Invalid recordings or timestamps!" << std::endl;
        return;
    }

    start_time = Clock::now();

    for (size_t i = 0; i < timestamps.size(); ++i) {
        // Zielzeitpunkt berechnen
        auto target_time = start_time + std::chrono::milliseconds(timestamps[i]);

        // Warten bis zur Zielzeit (ggf. sleep für CPU-Schonung)
        while (Clock::now() < target_time) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(500)); // klein halten, um präzise zu bleiben
        }

        // Recording senden
        auto result = vigem_target_ds4_update(client, controller, recordings[i]);
        if (result != VIGEM_ERROR_NONE) {
            std::cerr << "Failed to update controller at timestamp " << timestamps[i] << ": " << result << std::endl;
        }
    }

    std::cout << "Replay completed." << std::endl;
}

