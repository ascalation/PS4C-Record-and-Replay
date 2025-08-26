#include "InputAccess.h"
#include <iostream>
#include <cstring>

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

    reader_thread = std::thread(&InputAccess::reader_thread_func, this);
    processor_thread = std::thread(&InputAccess::processor_thread_func, this);
}

InputAccess::~InputAccess() {
    stop_threads = true;
    queue_cv.notify_all();

    if (reader_thread.joinable()) reader_thread.join();
    if (processor_thread.joinable()) processor_thread.join();

    if (handle) hid_close(handle);
    hid_exit();
}

void InputAccess::reader_thread_func() {
    while (!stop_threads) {
        if (pause_threads) {
            std::this_thread::yield();
            continue;
        }

        Buffer buffer{};
        const int result = hid_read(handle, buffer.data(), buffer.size());
        const auto timestamp = std::chrono::high_resolution_clock::now();

        if (result > 0) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                timestamp_queue.emplace(timestamp);
                input_queue.emplace(result, std::move(buffer));
            }
            queue_cv.notify_one();
        }
        else if (result < 0) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(100));
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
            TimePoint timestamp = timestamp_queue.front();
            input_queue.pop();
            timestamp_queue.pop();

            lock.unlock();
            process_input_data(result, buffer, timestamp);
            lock.lock();
        }
    }
}

void InputAccess::process_input_data(int result, const Buffer& buffer, TimePoint timestamp) {
    if (result <= 0 || buffer.size() < 10) {
        return;
    }
    DS4_REPORT report{};
    std::memset(&report, 0, sizeof(report));

    report.bThumbLX = buffer[1];
    report.bThumbLY = buffer[2];
    report.bThumbRX = buffer[3];
    report.bThumbRY = buffer[4];

    if (buffer[5] & (1 << 4)) report.wButtons |= DS4_BUTTON_SQUARE;
    if (buffer[5] & (1 << 5)) report.wButtons |= DS4_BUTTON_CROSS;
    if (buffer[5] & (1 << 6)) report.wButtons |= DS4_BUTTON_CIRCLE;
    if (buffer[5] & (1 << 7)) report.wButtons |= DS4_BUTTON_TRIANGLE;

    uint8_t btn2 = buffer[6];
    if (btn2 & (1 << 0)) report.wButtons |= DS4_BUTTON_SHOULDER_LEFT;
    if (btn2 & (1 << 1)) report.wButtons |= DS4_BUTTON_SHOULDER_RIGHT;
    if (btn2 & (1 << 2)) report.wButtons |= DS4_BUTTON_TRIGGER_LEFT;
    if (btn2 & (1 << 3)) report.wButtons |= DS4_BUTTON_TRIGGER_RIGHT;
    if (btn2 & (1 << 4)) report.wButtons |= DS4_BUTTON_SHARE;
    if (btn2 & (1 << 5)) report.wButtons |= DS4_BUTTON_OPTIONS;
    if (btn2 & (1 << 6)) report.wButtons |= DS4_BUTTON_THUMB_LEFT;
    if (btn2 & (1 << 7)) report.wButtons |= DS4_BUTTON_THUMB_RIGHT;

    uint8_t btn3 = buffer[7];
    if (btn3 & (1 << 0)) report.bSpecial |= DS4_SPECIAL_BUTTON_PS;
    if (btn3 & (1 << 1)) report.bSpecial |= DS4_SPECIAL_BUTTON_TOUCHPAD;

    report.bTriggerL = buffer[8];
    report.bTriggerR = buffer[9];

    if (rec) {
        rec->save_controller_state(report, timestamp);
    }
}
