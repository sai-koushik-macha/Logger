#ifndef SPINLOCK_H_
#define SPINLOCK_H_

#include <pthread.h>

struct SpinLock {
    pthread_spinlock_t sp;
    SpinLock() noexcept { pthread_spin_init(&sp, PTHREAD_PROCESS_PRIVATE); }
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;
    ~SpinLock() noexcept { pthread_spin_destroy(&sp); }

    inline void lock() noexcept { pthread_spin_lock(&sp); }
    inline void unlock() noexcept { pthread_spin_unlock(&sp); }
};

#endif /* SPINLOCK_H_ */
