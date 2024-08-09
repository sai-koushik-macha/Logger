#ifndef LOGGER_H_
#define LOGGER_H_

#include <sched.h>
#include <unistd.h>

#include <chrono>
#include <concepts>
#include <cstdio>
#include <deque>
#include <format>
#include <fstream>
#include <source_location>
#include <string>
#include <thread>

#include "LoggerTypes.h"
#include "Mempool.h"
#include "SpinLock.h"

#ifdef LATENCY_FINDING

#include <iostream>

#include "latency_profile.h"
#endif

struct FileWrapper {
    explicit FileWrapper(std::string_view filename_) noexcept {
        filename = filename_;
        filename += "_";
        filename += std::to_string(getpid());
        filename += "_";
        createFile();
    }
    FileWrapper() = delete;
    FileWrapper(const FileWrapper&) = delete;
    FileWrapper(const FileWrapper&&) = delete;
    FileWrapper operator=(const FileWrapper&) = delete;
    FileWrapper operator=(const FileWrapper&&) = delete;
    ~FileWrapper() noexcept { closeFile(); }

    inline void closeFile() noexcept {
        if (file.is_open()) {
            file.close();
        }
    }
    inline void createFile() noexcept {
        closeFile();
        size_used = 0;
        file.open(filename + (count < 0 ? '0' + std::to_string(count)
                                        : std::to_string(count)),
                  std::ios::out);
    }

    inline void WritetoFile(const std::string& data) noexcept {
        const unsigned long length_of_string = data.length();
        if (length_of_string + size_used > max_size) {
            count++;
            createFile();
        }
        file.write(data.c_str(), length_of_string);
        size_used += length_of_string;
    }

    int count{};
    static constexpr unsigned long max_size = 2251799813;
    std::string filename;
    unsigned long size_used{};
    std::fstream file;
};

template <typename T>
struct DataForLog {
    const bool log_time;
    const bool log_location;
    const bool new_line;
    std::source_location location;
    T* logger_pointer;
    const EnumLoggerTypes logger_type;
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
        : filewrapper(filename_), message_count(0) {}

    ~Logger() {
        while (message_count != 0)
            ;
        printf("Logger Destructed Properly\n");
    }

    // Once thread processing started you can multiple threads can log to same
    // file if needed
    static void StartThreadProcessing(int _core_id) {
        run = true;

        use_thread = true;
        core_id = _core_id;
        thread = std::thread(Logger::Process, nullptr);
    }

    // This method should be called at the end of the program when all the
    // logging is done From this point logging won't asynchronous when this
    // method is called don't log from a different thread the log will be missed
    // Once this method call is completed multiple threads shouldn't log to the
    // same file
    static void StopThreadProcessing() {
        if (use_thread) {
            run = false;
            thread.join();
            use_thread = false;
        }
    }

#ifdef LATENCY_FINDING
    static void PrintLatencies() {
        std::cout << latency_1.get_the_stats() << std::endl;
        std::cout << latency_2.get_the_stats() << std::endl;
        std::cout << latency_3.get_the_stats() << std::endl;
        std::cout << latency_4.get_the_stats() << std::endl;
        std::cout << latency_5.get_the_stats() << std::endl;
    }
#endif

    // Use this method for getting of object from different logger types
    // you want to print but the obj needs to implement print method which takes
    // string pointer as input and assign the pointer to whatever you want to
    // print
    template <HasPrintMethod T>
    static T* getObj() {
#ifdef LATENCY_FINDING
        LatencyProfilingHelper l(latency_1);
#endif
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
#ifdef LATENCY_FINDING
        LatencyProfilingHelper l(latency_2);
#endif
        if (use_thread) {
            sp.lock();
            auto pointer = mempool.allocate<DataForLog<Logger>>(
                logger, getType<T>(), data, log_time_, log_location_, new_line_,
                location);
            log_queue.push_back(pointer);
            logger->message_count++;
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
        filewrapper.WritetoFile(s);
    }
    static void* Process(void*) {
        int size_of_the_queue = 0;
        do {
#ifdef LATENCY_FINDING
            latency_3.start();
#endif
            sp.lock();
            size_of_the_queue = log_queue.size();
            if (size_of_the_queue) {
                DataForLog<Logger>* data_log = log_queue.front();
                log_queue.pop_front();
                sp.unlock();
#ifdef LATENCY_FINDING
                latency_3.end();
                latency_4.start();
#endif
                data_log->logger_pointer->LogHelper(data_log);
#ifdef LATENCY_FINDING
                latency_4.end();
                latency_5.start();
#endif
                sp.lock();
                data_log->logger_pointer->message_count--;
                DellocateFromMempool(mempool, data_log->logger_type,
                                     data_log->pointer);
                mempool.deallocate(data_log);
            }
            sp.unlock();
#ifdef LATENCY_FINDING
            latency_5.end();
#endif
        } while (run || size_of_the_queue);
        return nullptr;
    }

    FileWrapper filewrapper;
    int message_count;
    static volatile bool run;
    static std::deque<DataForLog<Logger>*> log_queue;
    static SpinLock sp;
    static Mempool mempool;
    static bool use_thread;
    static int core_id;
    static std::thread thread;

#ifdef LATENCY_FINDING
    static LatencyProfilingStats latency_1;
    static LatencyProfilingStats latency_2;
    static LatencyProfilingStats latency_3;
    static LatencyProfilingStats latency_4;
    static LatencyProfilingStats latency_5;
#endif
};

#endif /* LOGGER_H_ */
