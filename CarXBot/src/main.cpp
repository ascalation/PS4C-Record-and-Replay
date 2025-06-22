#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "ControllerRecording/Recorder.h"
#include "InputAccess/InputAccess.h"
#include "VirtualController/VirtualController.h"

int main() {
    Recorder rec;
    #ifdef _DEBUG
    std::cout << "Recorder initialized!" << std::endl;
    #endif
    InputAccess iA(&rec);

    bool wasF1Pressed = false;
    bool wasF2Pressed = false;

    while (true) {
        // F1: Aufzeichnung starten/stoppen
        bool isF1Down = GetAsyncKeyState(VK_F1) & 0x8000;
        if (isF1Down && !wasF1Pressed) {
            wasF1Pressed = true;
            
            
            if (!iA.pause_threads) {
                std::cout << "[F1] Stopped recording\n";
                Beep(1500, 100);
            }
            else {
                std::cout << "[F1] Started recording\n";
                Beep(1500, 100);
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

            if (rec.get_saves().empty()) {
                std::cerr << "No recordings found – skipping replay.\n";
            }
            else {
                bool continue_loop = true;
                VirtualController controller = VirtualController();

                while (continue_loop) {
                    #ifdef _DEBUG
                    std::cout << "Thread started\n";
                    #endif
                    std::thread bot(&VirtualController::replay, &controller, rec.get_saves());
                    bot.join();
                    #ifdef _DEBUG
                    std::cout << "Replay done\n";
                    #endif
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));

                    char just_works{ 'y' };
                    while (just_works == 'y')
                    {
                        std::cin >> just_works;
                        if (just_works == 'y') {
                            break;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    //if (GetAsyncKeyState(VK_F2) & 0x8000) {
                    //    std::cout << "F2 held again. Exiting replay loop\n";
                    //    continue_loop = false;
                    //}
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
