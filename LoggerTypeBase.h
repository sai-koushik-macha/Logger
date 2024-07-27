#ifndef LOGGERTYPEBASE_H_
#define LOGGERTYPEBASE_H_

#include <ctime>
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
        tm* time_info = localtime(&timestamp.tv_sec);

        // Use strftime for basic time formatting
        char buffer[80];
        size_t formatted_length =
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);

        // Check for formatting errors (optional)
        if (formatted_length == 0) {
            return "Error formatting time";
        }

        // Create the final formatted string with space for nanoseconds
        std::string formatted_time(buffer, formatted_length);
        formatted_time += ".";

        // Convert nanoseconds to a string with leading zeros
        std::string nanoseconds = std::to_string(timestamp.tv_nsec);
        nanoseconds.insert(0, 9 - nanoseconds.length(), '0');

        // Append nanoseconds to the formatted time
        formatted_time += nanoseconds;

        return "[" + formatted_time + "]";
    }

    inline std::string getLocation() noexcept {
        // std::stringstream ss;
        // ss << "[" << logging_location.file_name() << ","
        //    << logging_location.function_name() << "," <<
        //    logging_location.line()
        //    << "]";
        // std::string formatted_source =
        //     std::string("[") + std::string(logging_location.file_name()) +
        //     std::string(", ") + std::string(logging_location.function_name())
        //     + std::string(", ") + std::string(logging_location.line()) +
        //     std::string("]");

        auto line_in_string = std::to_string(logging_location.line());

        const char* multiple_messages[] = {
            "(",  logging_location.file_name(),
            ", ", logging_location.function_name(),
            ", ", line_in_string.c_str(),
            ")"};

        int total_messages =
            sizeof(multiple_messages) / sizeof(multiple_messages[0]);

        std::string formatted_source;

        for (int i = 0; i < total_messages; i++) {
            formatted_source += multiple_messages[i];
        }

        return formatted_source;
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

    // void log() {}

    inline void logObjBase(
        const bool _log_time, const bool _log_location,
        const std::source_location& _logging_location) noexcept {
        log_time = _log_time;
        log_location = _log_location;
        if (log_time) {
            clock_gettime(CLOCK_REALTIME, &timestamp);
        }
        if (log_location) {
            logging_location = _logging_location;
        }
    }

    template <typename... Args>
    inline void log(const bool _log_time, const bool _log_location,
                    const std::source_location& _logging_location,
                    Args&&... args) noexcept {
        logObjBase(_log_time, _log_location, _logging_location);
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
