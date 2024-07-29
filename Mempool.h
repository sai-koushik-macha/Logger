#ifndef MEMPOOL_H_
#define MEMPOOL_H_

#include <cstdio>
#include <deque>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

// #define DEBUG_MEM

#ifdef DEBUG_MEM
#define LOG_LOCATION_MEMPOOL \
    printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
#else
#define LOG_LOCATION_MEMPOOL
#endif

class Mempool {
   public:
    ~Mempool() {
        for (int i = 0; i < memory_blocks.size(); i++) {
            auto memory_block = memory_blocks[i];
            delete[] memory_block;
        }
    }

    template <typename T, typename... Args>
    inline T* allocate(Args... args) {
        auto iter = mempools_mapper.find(typeid(T));
        if (iter == mempools_mapper.end()) {
            LOG_LOCATION_MEMPOOL;
            mempools_mapper[typeid(T)] = {};
        }
        iter = mempools_mapper.find(typeid(T));
        if (mempools_mapper[typeid(T)].empty()) {
            LOG_LOCATION_MEMPOOL;
            ExtendMemory<T>();
        }

        auto data = iter->second.back();
        iter->second.pop_back();
        auto typecasted_data = new (data) T(std::forward<Args>(args)...);
        return typecasted_data;
    }

    template <typename T>
    inline void deallocate(T* obj) {
        // log();
        LOG_LOCATION_MEMPOOL;
        obj->~T();
        auto iter = mempools_mapper.find(typeid(T));
        if (iter != mempools_mapper.end()) {
            mempools_mapper[typeid(T)].push_back(obj);
        }
    }

    template <typename T>
    inline void Register() noexcept {
        if (auto iter = mempools_mapper.find(typeid(T));
            iter == mempools_mapper.end()) {
            mempools_mapper[typeid(T)] = {};
            ExtendMemory<T>();
        }
    }

   private:
    std::unordered_map<std::type_index, std::deque<void*>> mempools_mapper;
    std::vector<char*> memory_blocks;

    template <typename T>
    inline void ExtendMemory() noexcept {
        LOG_LOCATION_MEMPOOL;
        auto iter = mempools_mapper.find(typeid(T));
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
};

#endif /* MEMPOOL_H_ */
