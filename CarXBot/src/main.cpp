#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "ControllerRecording/Recorder.h"
#include "InputAccess/InputAccess.h"
#include "VirtualController/VirtualController.h"

int main() {
    Recorder rec;
    std::cout << "Recorder initialized!" << std::endl;
    VirtualController controller = VirtualController();
    InputAccess iA(&rec);

    bool wasF1Pressed = false;
    bool wasF2Pressed = false;

    while (true) {
        // ESC = Programm beenden
        if (GetAsyncKeyState(VK_ESCAPE) & 1) {
            std::cout << "ESC pressed – exiting." << std::endl;
            break;
        }

        // F1: Aufzeichnung starten/stoppen
        bool isF1Down = GetAsyncKeyState(VK_F1) & 0x8000;
        if (isF1Down && !wasF1Pressed) {
            wasF1Pressed = true;
            
            Beep(1000, 100);
            if (iA.pause_threads) {
                std::cout << "[F1] Stopped recording\n";
            }
            else {
                std::cout << "[F1] Started recording\n";
            }

            rec.set_start_time();
            iA.pause_threads = !iA.pause_threads;
        }
        else if (!isF1Down) {
            wasF1Pressed = false;
        }

        // F2: Replay
        bool isF2Down = GetAsyncKeyState(VK_F2) & 0x8000;
        if (isF2Down && !wasF2Pressed) {
            wasF2Pressed = true;

            std::cout << "[F2] Replay loop started\n";

            if (rec.get_timestamps().empty() || rec.get_recordings().empty()) {
                std::cerr << "No recordings found – skipping replay.\n";
            }
            else {
                bool continue_loop = true;

                while (continue_loop) {
                    std::thread bot(&VirtualController::replay, &controller, rec.get_timestamps(), rec.get_recordings());
                    bot.join();

                    std::cout << "Replay done\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));

                    if (GetAsyncKeyState(VK_F2) & 0x8000) {
                        std::cout << "F2 held again – exiting replay loop\n";
                        continue_loop = false;
                    }
                }
            }

        }
        else if (!isF2Down) {
            wasF2Pressed = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
