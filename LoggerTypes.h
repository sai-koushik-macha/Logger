#ifndef LOGGERTYPESDERIVED_H_
#define LOGGERTYPESDERIVED_H_

#include <format>
#include <string>

#include "Mempool.h"

enum class EnumLoggerTypes { Default = 0, LoggerType1 = 1, LoggerType2 = 2 };

struct LoggerType1 {
    int number;

    inline void print(std::string& data_to_print) noexcept {
        data_to_print = std::format("Derived 1 {}", number);
    }
};

struct LoggerType2 {
    int number;
    char h;

    inline void print(std::string& data_to_print) noexcept {
        data_to_print = std::format("Derived 2 {} {}", number, h);
    }
};

template <typename T>
inline EnumLoggerTypes getType() noexcept {
#define TYPECHECKERANDRETURNCORRECTENUM(TYPE)          \
    else if constexpr (std::is_same<T, TYPE>::value) { \
        return EnumLoggerTypes::TYPE;                  \
    }
    if constexpr (false) {
        return EnumLoggerTypes::Default;
    }
    TYPECHECKERANDRETURNCORRECTENUM(LoggerType1)
    TYPECHECKERANDRETURNCORRECTENUM(LoggerType2)
    else {
        static_assert(false, "Please Provide correct type");
    }
};

inline void DellocateFromMempool(Mempool& mempool, EnumLoggerTypes derived_type,
                                 void* pointer) noexcept {
#define CASECHECKMEMPOOLDELLOCATE(TYPE)          \
    case EnumLoggerTypes::TYPE: {                \
        auto data = static_cast<TYPE*>(pointer); \
        mempool.deallocate(data);                \
        break;                                   \
    }

    switch (derived_type) {
        CASECHECKMEMPOOLDELLOCATE(LoggerType1);
        CASECHECKMEMPOOLDELLOCATE(LoggerType2);
        case EnumLoggerTypes::Default: {
            break;
        }
    }
}

inline void PrintType(EnumLoggerTypes derived_type, void* pointer,
                      std::string& data_to_print) noexcept {
#define CASECHECKPRINT(TYPE)                     \
    case EnumLoggerTypes::TYPE: {                \
        auto data = static_cast<TYPE*>(pointer); \
        data->print(data_to_print);              \
        break;                                   \
    }

    switch (derived_type) {
        CASECHECKPRINT(LoggerType1);
        CASECHECKPRINT(LoggerType2);
        case EnumLoggerTypes::Default: {
            break;
        }
    }
}

#endif /* LOGGERTYPESDERIVED_H_ */
