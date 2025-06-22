#pragma once

#include <Windows.h>
#include <hidapi/hidapi.h>
#include <ViGEm/Client.h>
#include "../ControllerRecording/Recorder.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <array>
#include <chrono>

// Typalias für Eingabedaten
using Buffer = std::array<unsigned char, 64>;
using TimePoint = std::chrono::high_resolution_clock::time_point;

class InputAccess {
public:
    InputAccess(Recorder* rec);
    ~InputAccess();

    std::atomic_bool pause_threads{ true };

private:
    Recorder* rec;
    hid_device* handle = nullptr;

    std::thread reader_thread;
    std::thread processor_thread;
    std::atomic<bool> stop_threads = false;

    std::queue<std::pair<int, Buffer>> input_queue;
    std::queue<TimePoint> timestamp_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;

    void reader_thread_func();
    void processor_thread_func();
    void process_input_data(int result, const Buffer& buffer, TimePoint timestamp);
};
