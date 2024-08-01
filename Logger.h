#ifndef LOGGER_H_
#define LOGGER_H_

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
#include "SpinLock.h"

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
        DataForLog data;
        while (DataAssignAndDellocateHelper(data)) {
            PrintDerivedClass(this, data.derived_type, data.pointer);
        }
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
            auto data = mempool.allocate<T>();
            data->log(_log_time, _log_location, _logging_location,
                      std::forward<Args>(args)...);
            log_queue.emplace_back(getType<T>(), data);
            sp.unlock();
        } else {
            T data;
            data.log(_log_time, _log_location, _logging_location,
                     std::forward<Args>(args)...);
            PrintHelper(getType<T>(), &data);
        }
    }

    template <typename T>
    auto getPrintHelperObj() {
        static_assert(
            LoggerTypeBase<T>::template isDerviedOfLoggerTypeBase<T>(),
            "It is not derived class of LoggerTypeBase");
        T* obj = nullptr;
        if (use_thread) {
            sp.lock();
            obj = mempool.allocate<T>();
            sp.unlock();
        } else {
            obj = mempool.allocate<T>();
        }
        return obj;
    }

    template <typename T>
    void logObj(const bool _log_time, const bool _log_location,
                const std::source_location& _logging_location,
                T* pointer) noexcept {
        static_assert(
            LoggerTypeBase<T>::template isDerviedOfLoggerTypeBase<T>(),
            "It is not derived class of LoggerTypeBase");
        pointer->logObjBase(_log_time, _log_location, _logging_location);

        if (use_thread) {
            sp.lock();
            log_queue.emplace_back(getType<T>(), pointer);
            sp.unlock();
        } else {
            PrintHelper(getType<T>(), pointer);
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

    inline bool DataAssignAndDellocateHelper(DataForLog& data) noexcept {
        if (log_queue.size()) {
            data = log_queue.front();
            log_queue.pop_front();
            DellocateFromMempool(mempool, data.derived_type, data.pointer);
            return true;
        }
        return false;
    }

    inline void PrintHelper(const auto& derived_type,
                            auto DataPointer) noexcept {
        PrintDerivedClass(this, derived_type, DataPointer);
    }

    inline void print() noexcept {
        DataForLog data;
        if (use_thread) {
            sp.lock();
        }
        auto has_data = DataAssignAndDellocateHelper(data);
        if (use_thread) {
            sp.unlock();
        }
        if (has_data) {
            PrintHelper(data.derived_type, data.pointer);
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

#endif /* LOGGER_H_ */
