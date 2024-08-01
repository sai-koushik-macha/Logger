#include "Logger.h"

volatile bool Logger::run = false;
std::deque<DataForLog<Logger>*> Logger::log_queue = {};
SpinLock Logger::sp = {};
bool Logger::use_thread = false;
Mempool Logger::mempool = {};
int Logger::core_id = -1;
pthread_t Logger::thread = {};
