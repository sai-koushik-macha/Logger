#ifndef SPINLOCK_H_
#define SPINLOCK_H_

#include <atomic>

struct SpinLock {
    std::atomic<bool> flag;
    SpinLock() noexcept {  }
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;
    ~SpinLock() noexcept {  }

    inline void lock() noexcept { 
        while(1) {
            if(!flag.exchange(true)) return;
            while (flag.load())
            {
            }
            
        } 
    }
    inline void unlock() noexcept { flag.store(false); }
};

#endif /* SPINLOCK_H_ */
