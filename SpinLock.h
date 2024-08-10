#ifndef SPINLOCK_H_
#define SPINLOCK_H_

#include <mutex>

struct SpinLock {
    std::mutex sp;
    SpinLock() noexcept {  }
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;
    ~SpinLock() noexcept {  }

    inline void lock() noexcept { sp.lock(); }
    inline void unlock() noexcept { sp.unlock(); }
};

#endif /* SPINLOCK_H_ */
