#ifndef LOGGERTYPESDERIVED_H_
#define LOGGERTYPESDERIVED_H_

#include <format>
#include <string>

#include "Mempool.h"

// This file is where you define types of your choice for printing purpose
// Once you define your type write the same name as your class and give it to
// the below enum like the example
// After defining it in the below few logger helper function are defined
// make sure to write them

enum class EnumLoggerTypes { Default = 0, LoggerType1 = 1, LoggerType2 = 2 };

struct LoggerType1 {
    int number;

    inline void print(std::string* data_to_print) const noexcept {
        *data_to_print = std::format("Derived 1 {}", number);
    }
};

struct LoggerType2 {
    int number;
    char h;

    inline void print(std::string* data_to_print) const noexcept {
        *data_to_print = std::format("Derived 2 {} {}", number, h);
    }
};

inline void RegisterLogDerivedTypes(Mempool& mempool) noexcept {
#define REGISTERTYPEHELPER(TYPE) mempool.Register<TYPE>()
    REGISTERTYPEHELPER(LoggerType1);
    REGISTERTYPEHELPER(LoggerType2);
}

// This is mainly for getting enum type that you have
// Please you the TYPECHECKERANDRETURNCORRECTENUM macro as given in the example
// and input to macro as your class name
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

// This is for internal class usage
// Use the CASECHECKMEMPOOLDELLOCATE inside the switch statement by giving your
// class name as input
inline void DellocateFromMempool(Mempool& mempool,
                                 const EnumLoggerTypes derived_type,
                                 void const* const pointer) noexcept {
#define CASECHECKMEMPOOLDELLOCATE(TYPE)                                       \
    case EnumLoggerTypes::TYPE: {                                             \
        auto data =                                                           \
            const_cast<TYPE* const>(static_cast<TYPE const* const>(pointer)); \
        mempool.deallocate(data);                                             \
        break;                                                                \
    }

    switch (derived_type) {
        CASECHECKMEMPOOLDELLOCATE(LoggerType1);
        CASECHECKMEMPOOLDELLOCATE(LoggerType2);
        case EnumLoggerTypes::Default: {
            break;
        }
    }
}

// This is for calling print method based on type
// Inside the switch statement call CASECHECKPRINT macro by giving class name as
// input
inline void PrintType(const EnumLoggerTypes derived_type,
                      void const* const pointer,
                      std::string* data_to_print) noexcept {
#define CASECHECKPRINT(TYPE)                                 \
    case EnumLoggerTypes::TYPE: {                            \
        auto data = static_cast<TYPE const* const>(pointer); \
        data->print(data_to_print);                          \
        break;                                               \
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
