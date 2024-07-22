#ifndef LOGGERTYPEBASE_H_
#define LOGGERTYPEBASE_H_

#include <ctime>
#include <iomanip>
#include <iostream>
#include <source_location>
#include <string>
#include <type_traits>

template <typename Derived>
struct LoggerTypeBase {
   public:
    bool log_time;
    bool log_location;
    std::timespec timestamp;
    std::source_location logging_location;

    template <typename T>
    inline static constexpr auto isDerviedOfLoggerTypeBase() noexcept {
        return std::is_base_of<LoggerTypeBase, T>::value;
    }

    inline std::string getTime() noexcept {
        std::stringstream ss;
        auto timeinfo = localtime(&timestamp.tv_sec);
        ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(9) << timestamp.tv_nsec;
        return ss.str();
    }

    inline std::string getLocation() noexcept {
        std::stringstream ss;
        ss << "[" << logging_location.file_name() << ","
           << logging_location.function_name() << "," << logging_location.line()
           << "]";
        return ss.str();
    };

    template <typename LoggerType>
    void print(LoggerType* logger) noexcept {
        if (log_time) {
            std::cout << getTime() << " ";
        }
        if (log_location) {
            std::cout << getLocation() << " ";
        }
        static_cast<Derived*>(this)->print_impl(logger);
        std::cout << "\n";
    }

    template <typename... Args>
    void log(const bool _log_time, const bool _log_location,
             const std::source_location& _logging_location,
             Args&&... args) noexcept {
        log_time = _log_time;
        log_location = _log_location;
        if (log_time) {
            clock_gettime(CLOCK_REALTIME, &timestamp);
        }
        if (log_location) {
            logging_location = _logging_location;
        }
        static_cast<Derived*>(this)->log_impl(std::forward<Args>(args)...);
    }

   private:
    LoggerTypeBase() {
        log_time = false;
        log_location = false;
    };
    friend Derived;
};

#endif /* LOGGERTYPEBASE_H_ */
