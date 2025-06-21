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

class InputAccess {
public:
    InputAccess(Recorder* rec);
    ~InputAccess();

    std::atomic_bool pause_threads{ false };
private:
    void reader_thread_func();
    void processor_thread_func();
    void process_input_data(int result, const std::vector<unsigned char>& buffer);

    Recorder* rec;
    hid_device* handle = nullptr;

    std::thread reader_thread;
    std::thread processor_thread;

    std::atomic_bool stop_threads{ false };
    
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::queue<std::pair<int, std::vector<unsigned char>>> input_queue;
};
