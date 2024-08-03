#ifndef LOGGER_H_
#define LOGGER_H_

#include <pthread.h>
#include <sched.h>

#include <chrono>
#include <concepts>
#include <deque>
#include <format>
#include <iostream>
#include <source_location>

#include "LoggerTypes.h"
#include "Mempool.h"
#include "SpinLock.h"

#define MAX_FILE_SIZE 3068

template <typename T>
struct DataForLog {
    bool log_time;
    bool log_location;
    bool new_line;
    std::source_location location;
    T* logger_pointer;
    EnumLoggerTypes logger_type;
    void* pointer;
    std::chrono::system_clock::time_point time_now;

    DataForLog(T* logger_pointer, EnumLoggerTypes _logger_type, void* _pointer,
               bool _log_time, bool _log_location, bool _new_line,
               const std::source_location& location_)
        : logger_type(_logger_type),
          pointer(_pointer),
          logger_pointer(logger_pointer),
          log_time(_log_time),
          log_location(_log_location),
          new_line(_new_line) {
        if (log_location) {
            location = location_;
        }
        if (log_time) {
            time_now = std::chrono::system_clock::now();
        }
    }
};

template <typename T>
concept HasPrintMethod = requires(T t, std::string* s) {
    { t.print(s) } -> std::same_as<void>;
};

class Logger {
   public:
    Logger(std::string_view filename_)
        : count(0), filename(filename_), size_of_the_new_file(0) {}

    ~Logger() {}

    // Once thread processing started you can multiple threads can log to same
    // file if needed
    static void StartThreadProcessing(int _core_id) {
        run = true;

        use_thread = true;
        core_id = _core_id;
        pthread_create(&thread, nullptr, Logger::Process, nullptr);
    }

    // This method should be called at the end of the program when all the
    // logging is done From this point logging won't asynchronous when this
    // method is called don't log from a different thread the log will be missed
    // Once this method call is completed multiple threads shouldn't log to the
    // same file
    static void StopThreadProcessing() {
        if (use_thread) {
            run = false;
            pthread_join(thread, nullptr);
            use_thread = false;
        }
    }

    // Use this method for getting of object from different logger types
    // you want to print but the obj needs to implement print method which takes
    // string pointer as input and assign the pointer to whatever you want to
    // print
    template <HasPrintMethod T>
    static T* getObj() {
        if (use_thread) {
            sp.lock();
        }
        auto pointer = mempool.allocate<T>();
        if (use_thread) {
            sp.unlock();
        }
        return pointer;
    }

    // After the values for the print object has been assigned
    // call this method for printing
    // Logger object
    // can choose to print time, location from where the printing is happening
    // (from where the obj is passed to logger)
    // whether to print new line or not
    // pointer that has been taken for the printing purpose
    template <HasPrintMethod T>
    static void Log(Logger* logger, bool log_time_, bool log_location_,
                    bool new_line_, T* data,
                    const std::source_location& location =
                        std::source_location::current()) {
        if (use_thread) {
            sp.lock();
            auto pointer = mempool.allocate<DataForLog<Logger>>(
                logger, getType<T>(), data, log_time_, log_location_, new_line_,
                location);
            log_queue.push_back(pointer);
            sp.unlock();
        } else {
            DataForLog<Logger> data_log(logger, getType<T>(), data, log_time_,
                                        log_location_, new_line_, location);
            logger->LogHelper(&data_log);
            mempool.deallocate(data);
        }
    }

   private:
    void LogHelper(DataForLog<Logger>* data_log) {
        std::string s;
        if (data_log->log_time) {
            s += std::format("[{}] ", data_log->time_now);
        }
        if (data_log->log_location) {
            s += std::format("[{} {} {}] ", data_log->location.file_name(),
                             data_log->location.function_name(),
                             data_log->location.line());
        }
        std::string data_to_actually_print;
        PrintType(data_log->logger_type, data_log->pointer,
                  &data_to_actually_print);
        s += data_to_actually_print;
        if (data_log->new_line) {
            s += '\n';
        }
        int sizeofstring = s.length();
        std::cout << sizeofstring << " " << size_of_the_new_file << std::endl;
        if (sizeofstring + size_of_the_new_file < MAX_FILE_SIZE) {
            std::cout << "Size is less" << std::endl;
            size_of_the_new_file += sizeofstring;
            std::cout << "Old: " << s;
        } else {
            std::cout << "Size is exceeded" << std::endl;
            size_of_the_new_file = sizeofstring;
            std::cout << "New: " << s;
        }
    }
    static void* Process(void*) {
        int size_of_the_queue = 0;
        do {
            sp.lock();
            size_of_the_queue = log_queue.size();
            if (log_queue.size()) {
                DataForLog<Logger>* data_log = log_queue.front();
                log_queue.pop_front();
                sp.unlock();
                data_log->logger_pointer->LogHelper(data_log);
                sp.lock();
                DellocateFromMempool(mempool, data_log->logger_type,
                                     data_log->pointer);
                mempool.deallocate(data_log);
            }
            sp.unlock();
        } while (run || size_of_the_queue);
        return nullptr;
    }

    const std::string filename;

    unsigned long size_of_the_new_file;
    int count;

    static volatile bool run;
    static std::deque<DataForLog<Logger>*> log_queue;
    static SpinLock sp;
    static Mempool mempool;
    static bool use_thread;
    static int core_id;
    static pthread_t thread;
};

#endif /* LOGGER_H_ */
