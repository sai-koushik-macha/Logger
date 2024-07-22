#ifndef MEMPOOL_H_
#define MEMPOOL_H_

#include <pthread.h>

#include <cstdio>
#include <deque>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

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

class Mempool {
   public:
    ~Mempool() {
        for (int i = 0; i < memory_blocks.size(); i++) {
            auto memory_block = memory_blocks[i];
            delete[] memory_block;
        }
    }

    template <typename T, typename... Args>
    T* allocate(Args... args) noexcept {
        auto iter = mempools_mapper.find(typeid(T));
        if (iter == mempools_mapper.end()) {
            // log();
            mempools_mapper[typeid(T)] = {};
        }
        iter = mempools_mapper.find(typeid(T));
        if (mempools_mapper[typeid(T)].empty()) {
            // log();
            static constexpr auto type_size = sizeof(T);
            static constexpr std::size_t chunk_size_ = 1024;
            static constexpr auto size_to_create = type_size * chunk_size_;
            auto memory_block = new char[size_to_create];
            for (int i = 0; i < chunk_size_; i++) {
                iter->second.push_back(
                    static_cast<void*>(memory_block + i * type_size));
            }
            memory_blocks.push_back(memory_block);
        }

        auto data = iter->second.back();
        iter->second.pop_back();
        auto typecasted_data = new (data) T(std::forward<Args>(args)...);
        return typecasted_data;
    }

    template <typename T>
    void deallocate(T* obj) noexcept {
        // log();
        obj->~T();
        auto iter = mempools_mapper.find(typeid(T));
        if (iter != mempools_mapper.end()) {
            mempools_mapper[typeid(T)].push_back(obj);
        }
    }

    template <typename T>
    void Register() {
        auto data = allocate<T>();
        deallocate(data);
    }

   private:
    std::unordered_map<std::type_index, std::deque<void*>> mempools_mapper;
    std::vector<char*> memory_blocks;
};

#endif /* MEMPOOL_H_ */
