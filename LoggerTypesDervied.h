#ifndef LOGGERTYPESDERIVED_H_
#define LOGGERTYPESDERIVED_H_

#include "LoggerTypeBase.h"
#include "Mempool.h"

enum class EnumDerivedTypes { Default = 0, LoggerTypeDerived1 = 1 };

struct LoggerTypeDerived1 : LoggerTypeBase<LoggerTypeDerived1> {
    int number;

    template <typename LoggerType>
    inline void print_impl(LoggerType* logger) noexcept {
        std::cout << "Derived 1 " << number;
    }

    inline void log_impl(const int& _number) noexcept { number = _number; }
};

template <typename T>
EnumDerivedTypes getType() {
    if constexpr (std::is_same<T, LoggerTypeDerived1>::value) {
        return EnumDerivedTypes::LoggerTypeDerived1;
    }
};

void DellocateFromMempool(Mempool& mempool, EnumDerivedTypes derived_type,
                          void* pointer) {
    switch (derived_type) {
        case EnumDerivedTypes::LoggerTypeDerived1: {
            auto data = static_cast<LoggerTypeDerived1*>(pointer);
            mempool.deallocate(data);
            break;
        }
        case EnumDerivedTypes::Default: {
            break;
        }
    }
}

template <typename T>
void PrintDerivedClass(T* logger_type, EnumDerivedTypes derived_type,
                       void* pointer) {
    switch (derived_type) {
        case EnumDerivedTypes::LoggerTypeDerived1: {
            auto data = static_cast<LoggerTypeDerived1*>(pointer);
            data->print(logger_type);
            break;
        }
        case EnumDerivedTypes::Default: {
            break;
        }
    }
}

#endif /* LOGGERTYPESDERIVED_H_ */
