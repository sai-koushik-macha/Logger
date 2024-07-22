#ifndef LOGGER_HELPER_H_
#define LOGGER_HELPER_H_

#include <pthread.h>
#include <sched.h>

#include <cstdarg>
#include <deque>
#include <fstream>
#include <iostream>
#include <ostream>
#include <source_location>

#include "LoggerTypeBase.h"
#include "LoggerTypesDervied.h"
#include "Mempool.h"

#define LOGLOCATION std::source_location::current()

struct DataForLog {
    EnumDerivedTypes derived_type;
    void* pointer;
    DataForLog() : derived_type(EnumDerivedTypes::Default), pointer(nullptr) {}
    DataForLog(EnumDerivedTypes _derived_type, void* _pointer)
        : derived_type(_derived_type), pointer(_pointer) {}
};

class Logger {
   public:
    Logger(char* filename_, bool use_thread_, int _core_id = -1)
        : use_thread(use_thread_), core_id(_core_id) {
        if (use_thread) {
            run = true;
            pthread_create(&thread, nullptr, Logger::processor, this);
        }
    }

    ~Logger() {
        if (use_thread) {
            run = false;
            pthread_join(thread, nullptr);
        }
    }

    void LogHelper(char const* format, ...) {
        va_list args;
        va_start(args, format);
        va_end(args);
    }

    template <typename T, typename... Args>
    void log(const bool _log_time, const bool _log_location,
             const std::source_location& _logging_location,
             Args&&... args) noexcept {
        static_assert(
            LoggerTypeBase<T>::template isDerviedOfLoggerTypeBase<T>(),
            "It is not derived class of LoggerTypeBase");

        if (use_thread) {
            sp.lock();
        }
        T* data = mempool.allocate<T>();
        data->log(_log_time, _log_location, _logging_location,
                  std::forward<Args>(args)...);
        log_queue.emplace_back(getType<T>(), data);
        if (use_thread) {
            sp.unlock();
        }

        if (!use_thread) {
            std::cout << "Hi not using thread" << std::endl;
            print();
        }
    }

   private:
    static void* processor(void* arg) {
        auto obj = static_cast<Logger*>(arg);
        obj->Process();
        return nullptr;
    }

    void Process() {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);

        while (run) {
            print();
        }
    }

    void print() noexcept {
        DataForLog data;
        bool has_data = false;
        if (use_thread) {
            sp.lock();
        }
        if (log_queue.size()) {
            data = log_queue.front();
            log_queue.pop_front();
            has_data = true;
            DellocateFromMempool(mempool, data.derived_type, data.pointer);
        }
        if (use_thread) {
            sp.unlock();
        }
        if (has_data) {
            PrintDerivedClass(this, data.derived_type, data.pointer);
        }
    }

    volatile bool run;
    std::deque<DataForLog> log_queue;
    SpinLock sp;
    Mempool mempool;
    const bool use_thread;
    const int core_id;
    pthread_t thread;
};

#endif /* LOGGER_HELPER_H_ */
