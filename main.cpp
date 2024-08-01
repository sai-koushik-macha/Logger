#include <csignal>

#include "Logger.h"

volatile bool run_application = true;

inline void signal_handler(int signal) noexcept {
    std::cout << "Stopping application" << std::endl;
    run_application = false;
}

int main() {
    Logger logger("Hi.txt", true);
    std::signal(SIGINT, signal_handler);
    logger.log<LoggerTypeDerived1>(true, true, LOGLOCATION, 1024);
    logger.log<LoggerTypeDerived1>(true, false, LOGLOCATION, 10);
    logger.log<LoggerTypeDerived2>(true, false, LOGLOCATION, 10, 'h');
    auto log2_helper = logger.getPrintHelperObj<LoggerTypeDerived2>();
    log2_helper->h = 'h';
    logger.logObj(true, true, LOGLOCATION, log2_helper);

    while (run_application) {
    }
    return 0;
}
