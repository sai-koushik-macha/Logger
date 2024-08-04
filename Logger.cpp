#include "Logger.h"

volatile bool Logger::run = false;
std::deque<DataForLog<Logger>*> Logger::log_queue = {};
SpinLock Logger::sp = {};
bool Logger::use_thread = false;
Mempool Logger::mempool = {};
int Logger::core_id = -1;
pthread_t Logger::thread = {};

#ifdef LATENCY_FINDING
LatencyProfilingStats latency_1 = LatencyProfilingStats("GetObjMethod");
LatencyProfilingStats latency_2 = LatencyProfilingStats("LogMethod");
LatencyProfilingStats latency_3 = LatencyProfilingStats("PopQueue");
LatencyProfilingStats latency_4 = LatencyProfilingStats("FileWriting");
LatencyProfilingStats latency_5 = LatencyProfilingStats("Dellocate");
#endif
