#include <csignal>
#include <iostream>
#include <random>

#include "Logger.h"
#include "latency_profile.h"

volatile bool run_application = true;

inline void signal_handler(int signal) noexcept {
    std::cout << "Stopping application" << std::endl;
    run_application = false;
}

// std::timespec sleep_time{0, 10};
// inline void sleep_for_some_time() noexcept { nanosleep(&sleep_time, nullptr);
// }

int main() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> distrib(1, 1000);
    LatencyProfilingStats objget("ObjGet");
    LatencyProfilingStats objlog("ObjLog");
    Logger::StartThreadProcessing(-1);
    Logger logger("LOGFILE_TESTING");
    std::signal(SIGINT, signal_handler);
    // logger.log<LoggerType1>(true, true, LOGLOCATION, 1024);
    // logger.log<LoggerType1>(true, false, LOGLOCATION, 10);
    // logger.log<LoggerType2>(true, false, LOGLOCATION, 10, 'h');
    // auto log2_helper = logger.getPrintHelperObj<LoggerType2>();
    // log2_helper->h = 'h';
    // logger.logObj(true, true, LOGLOCATION, log2_helper);
    for (int i = 0; i < 10000000; i++) {
        objget.start();
        auto test = Logger::getObj<LoggerType1>();
        objget.end();
        test->number = distrib(gen);
        objlog.start();
        Logger::Log(&logger, true, true, true, test);
        objlog.end();
        volatile int work = 0;
        for (volatile int j = 0; j < 100; j = j + 1) {
            work = j + work;
        }
    }

    while (run_application) {
    }
    Logger::StopThreadProcessing();

    std::cout << objget.get_the_stats() << std::endl;
    std::cout << objlog.get_the_stats() << std::endl;

#ifdef LATENCY_FINDING
    Logger::PrintLatencies();
#endif
    return 0;
}
