#include "InputAccess.h"
#include <iostream>
#include <chrono>
#include <algorithm>

#define SONY_VID 0x054C
#define DS4_PID  0x09CC

InputAccess::InputAccess(Recorder* rec) : rec(rec) {
    if (hid_init()) {
        std::cerr << "HID init failed!" << std::endl;
        return;
    }
    std::cout << "HID initialized!" << std::endl;
    handle = hid_open(SONY_VID, DS4_PID, nullptr);
    if (!handle) {
        std::cerr << "Kein PS4-Controller gefunden!" << std::endl;
        return;
    }
    std::cout << "Controller found!" << std::endl;

    // Start threads
    reader_thread = std::thread(&InputAccess::reader_thread_func, this);
    processor_thread = std::thread(&InputAccess::processor_thread_func, this);
}

InputAccess::~InputAccess() {
    stop_threads = true;

    // Wecken, falls processor wartet
    queue_cv.notify_all();

    if (reader_thread.joinable()) reader_thread.join();
    if (processor_thread.joinable()) processor_thread.join();

    if (handle) hid_close(handle);
    hid_exit();
}

void InputAccess::reader_thread_func() {
    while (!stop_threads) {
        while (!pause_threads){
            std::vector<unsigned char> buffer(64);
            int result = hid_read_timeout(handle, buffer.data(), buffer.size(), 1);

            if (result >= 0) {
               std::lock_guard<std::mutex> lock(queue_mutex);
               input_queue.emplace(result, std::move(buffer));
               queue_cv.notify_one();
            }
        }
    }
}

void InputAccess::processor_thread_func() {
    while (!stop_threads) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock, [this]() {
            return stop_threads || !input_queue.empty();
            });

        while (!input_queue.empty()) {
            auto [result, buffer] = std::move(input_queue.front());
            input_queue.pop();
            lock.unlock();

            process_input_data(result, buffer);

            lock.lock();
        }
    }
}

void InputAccess::process_input_data(int result, const std::vector<unsigned char>& buffer) {
    auto start_time = Clock::now();
    if (result <= 0) {
        if (result < 0) {
            std::cout << "error. Did you remove the PS4 controller?\n";
        }
        return;
    }

    DS4_REPORT report{};
    ZeroMemory(&report, sizeof(report));

    // === STICKS ===
    report.bThumbLX = buffer[1];
    report.bThumbLY = buffer[2];
    report.bThumbRX = buffer[3];
    report.bThumbRY = buffer[4];

    // === FACE BUTTONS ===
    if (buffer[5] & (1 << 4)) report.wButtons |= DS4_BUTTON_SQUARE;
    if (buffer[5] & (1 << 5)) report.wButtons |= DS4_BUTTON_CROSS;
    if (buffer[5] & (1 << 6)) report.wButtons |= DS4_BUTTON_CIRCLE;
    if (buffer[5] & (1 << 7)) report.wButtons |= DS4_BUTTON_TRIANGLE;

    // === SHOULDER/TRIGGER/OPTIONS/SHARE ===
    uint8_t btn2 = buffer[6];
    if (btn2 & (1 << 0)) report.wButtons |= DS4_BUTTON_SHOULDER_LEFT;
    if (btn2 & (1 << 1)) report.wButtons |= DS4_BUTTON_SHOULDER_RIGHT;
    if (btn2 & (1 << 2)) report.wButtons |= DS4_BUTTON_TRIGGER_LEFT;
    if (btn2 & (1 << 3)) report.wButtons |= DS4_BUTTON_TRIGGER_RIGHT;
    if (btn2 & (1 << 4)) report.wButtons |= DS4_BUTTON_SHARE;
    if (btn2 & (1 << 5)) report.wButtons |= DS4_BUTTON_OPTIONS;
    if (btn2 & (1 << 6)) report.wButtons |= DS4_BUTTON_THUMB_LEFT;
    if (btn2 & (1 << 7)) report.wButtons |= DS4_BUTTON_THUMB_RIGHT;

    // === SPECIAL BUTTONS ===
    uint8_t btn3 = buffer[7];
    if (btn3 & (1 << 0)) report.bSpecial |= DS4_SPECIAL_BUTTON_PS;
    if (btn3 & (1 << 1)) report.bSpecial |= DS4_SPECIAL_BUTTON_TOUCHPAD;

    // === TRIGGERS (analog) ===
    report.bTriggerL = buffer[8];
    report.bTriggerR = buffer[9];

    auto elapsed_time = Clock::now();
    rec->save_controller_state(report, start_time, elapsed_time);
}



