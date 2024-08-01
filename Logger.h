#ifndef LOGGER_H_
#define LOGGER_H_

#include <fmt/chrono.h>
#include <pthread.h>
#include <sched.h>

#include <deque>
#include <source_location>

#include "LoggerTypes.h"
#include "Mempool.h"
#include "SpinLock.h"

#define LOGLOCATION std::source_location::current()

template <typename T>
struct DataForLog {
    bool log_time;
    bool log_location;
    std::source_location location;
    T* logger_pointer;
    EnumDerivedTypes derived_type;
    void* pointer;
    std::chrono::system_clock::time_point time_now;

    DataForLog(T* logger_pointer, EnumDerivedTypes _derived_type,
               void* _pointer, bool _log_time, bool _log_location,
               const std::source_location& location_)
        : derived_type(_derived_type),
          pointer(_pointer),
          logger_pointer(logger_pointer),
          log_time(_log_time),
          log_location(_log_location) {
        if (log_location) {
            location = location_;
        }
        if (log_time) {
            time_now = std::chrono::system_clock::now();
        }
    }
};

class Logger {
   public:
    Logger(char* filename_) {}

    ~Logger() {}

    static void StartThreadProcessing(int _core_id) {
        run = true;

        core_id = _core_id;
        pthread_create(&thread, nullptr, Logger::Process, nullptr);
    }

    static void StopThreadProcessing() {
        if (use_thread) {
            run = false;
            pthread_join(thread, nullptr);
        }
    }

    template <typename T>
    static T* getObj() {
        std::lock_guard<SpinLock> lock_g(Logger::sp);
        return mempool.allocate<T>();
    }

    template <typename T>
    static void Log(Logger* logger, bool log_time_, bool log_location_, T* data,
                    const std::source_location& location =
                        std::source_location::current()) {
        if (use_thread) {
            // auto pointer = mempool.allocate<DataForLog<Logger>>(getType<T>(),
            // data, logger);
        } else {
            DataForLog<Logger> data_log(logger, getType<T>(), data, log_time_,
                                        log_location_, location);
            logger->LogHelper(&data_log);
        }
    }

   private:
    void LogHelper(DataForLog<Logger>* data_log) {
        std::string s;
        if (data_log->log_time) {
            s += fmt::format("[{}] ", data_log->time_now);
        }
        if (data_log->log_location) {
            s += fmt::format("[{} {} {}] ", data_log->location.file_name(),
                             data_log->location.function_name(),
                             data_log->location.line());
        }
        std::cout << s;
        std::cout << PrintDerivedClass(data_log->derived_type,
                                       data_log->pointer)
                  << "\n";
    }
    static void* Process(void*) {
        while (run) {
        }
        return nullptr;
    }

    static volatile bool run;
    static std::deque<DataForLog<Logger>*> log_queue;
    static SpinLock sp;
    static Mempool mempool;
    static bool use_thread;
    static int core_id;
    static pthread_t thread;
};

#endif /* LOGGER_H_ */
