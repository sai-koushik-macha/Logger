#ifndef LOGGERTYPESDERIVED_H_
#define LOGGERTYPESDERIVED_H_

#include <iostream>

#include "Mempool.h"

enum class EnumDerivedTypes { Default = 0, LoggerType1 = 1, LoggerType2 = 2 };

struct LoggerType1 {
    int number;

    inline std::string_view print() noexcept {
        return "Derived 1 " + std::to_string(number);
    }

    inline void log(const int& _number) noexcept { number = _number; }
};

struct LoggerType2 {
    int number;
    char h;

    inline std::string_view print() noexcept {
        std::cout << "Derived 2 " << number << h << '\n';
        return "Derived 2 " + std::to_string(number) + " " + h;
    }

    inline void log(const int& _number, const char& _h) noexcept {
        number = _number;
        h = _h;
    }
};

template <typename T>
inline EnumDerivedTypes getType() noexcept {
#define TYPECHECKERANDRETURNCORRECTENUM(DERIVED_TYPE)          \
    else if constexpr (std::is_same<T, DERIVED_TYPE>::value) { \
        return EnumDerivedTypes::DERIVED_TYPE;                 \
    }
    if constexpr (false) {
        return EnumDerivedTypes::Default;
    }
    TYPECHECKERANDRETURNCORRECTENUM(LoggerType1)
    TYPECHECKERANDRETURNCORRECTENUM(LoggerType2)
    else {
        static_assert(false, "Please Provide correct type");
    }
};

inline void DellocateFromMempool(Mempool& mempool,
                                 EnumDerivedTypes derived_type,
                                 void* pointer) noexcept {
#define CASECHECKMEMPOOLDELLOCATE(DERIVED_TYPE)          \
    case EnumDerivedTypes::DERIVED_TYPE: {               \
        auto data = static_cast<DERIVED_TYPE*>(pointer); \
        mempool.deallocate(data);                        \
        break;                                           \
    }

    switch (derived_type) {
        CASECHECKMEMPOOLDELLOCATE(LoggerType1);
        CASECHECKMEMPOOLDELLOCATE(LoggerType2);
        case EnumDerivedTypes::Default: {
            break;
        }
    }
}

inline std::string_view PrintDerivedClass(EnumDerivedTypes derived_type,
                                          void* pointer) noexcept {
#define CASECHECKPRINT(DERIVED_TYPE)                     \
    case EnumDerivedTypes::DERIVED_TYPE: {               \
        auto data = static_cast<DERIVED_TYPE*>(pointer); \
        return data->print();                            \
        break;                                           \
    }

    switch (derived_type) {
        CASECHECKPRINT(LoggerType1);
        CASECHECKPRINT(LoggerType2);
        case EnumDerivedTypes::Default: {
            break;
        }
    }
    return "";
}

#endif /* LOGGERTYPESDERIVED_H_ */
