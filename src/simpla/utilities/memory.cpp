/**
 * @file memory_pool.cpp
 *
 *  Created on: 2014-11-14
 *      Author: salmon
 */
#include "simpla/SIMPLA_config.h"

#include "memory.h"

#include <cstddef>
#include <cstring>
#include <map>
#include <mutex>
#include <new>
#include <tuple>
#include "Log.h"
#include "SingletonHolder.h"
#include "device_common.h"
#include "simpla/utilities/SingletonHolder.h"

namespace simpla {
class MemoryPool {
   public:
    typedef char byte_type;

    MemoryPool();
    ~MemoryPool();
    MemoryPool(MemoryPool const &) = delete;
    MemoryPool(MemoryPool &&) = delete;
    MemoryPool &operator=(MemoryPool const &) = delete;
    MemoryPool &operator=(MemoryPool &&) = delete;
    //!  unused memory will be freed when total memory size >= pool size
    void max_size(size_t s);

    /**
     *  return the total size of memory in pool
     * @return
     */
    double size() const;

    /**
     *  push memory into pool
     * @param d memory address
     * @param s size of memory in byte
     */
    int push(void *p, size_t s, int loc = MANAGED_MEMORY);

    /**
     * allocate an array TV[s] from local pool or system memory
     * if s < MIN_BLOCK_SIZE or s > MAX_BLOCK_SIZE or
     *    s + pool_depth_> max_pool_depth_ then directly allocate
     *    memory from system
     *
     * @param s size of memory in byte
     * @return shared point of memory
     */
    void *pop(size_t s, int loc = MANAGED_MEMORY);

    void clear();

    std::mutex locker_;
    std::multimap<size_t, void *> pool_;
    std::multimap<size_t, void *> pool_dev_ptr_;

    static constexpr size_t ONE_GIGA = 1024l * 1024l * 1024l;
    static constexpr size_t MAX_BLOCK_SIZE = 4 * ONE_GIGA;  // std::numeric_limits<size_t>::max();
    static constexpr size_t MIN_BLOCK_SIZE = 256;

    size_t max_pool_depth_ = 16 * ONE_GIGA;
    size_t pool_depth_ = 0;
};

MemoryPool::MemoryPool() { pool_depth_ = 0; }
MemoryPool::~MemoryPool() { clear(); }

//!  unused MemoryPool will be freed when total MemoryPool size >= pool size
void MemoryPool::max_size(size_t s) { max_pool_depth_ = s; }

/**
 *  return the total size of MemoryPool in pool
 * @return
 */
double MemoryPool::size() const { return static_cast<double>(pool_depth_); }

void MemoryPool::clear() {
    locker_.lock();
    for (auto &item : pool_) { delete[] reinterpret_cast<byte_type *>(item.second); }
    locker_.unlock();
}

int MemoryPool::push(void *p, size_t s, int loc) {
    if ((s > MIN_BLOCK_SIZE) && (s < MAX_BLOCK_SIZE)) {
        locker_.lock();
        if ((pool_depth_ + s < max_pool_depth_)) {
            pool_.emplace(s, p);
            pool_depth_ += s;
            p = nullptr;
        }
        locker_.unlock();
        if (p != nullptr) {
#ifdef __CUDA__
            SP_DEVICE_CALL(cudaFree(p));
#else
            free(p);
#endif
        }
    }
    return SP_SUCCESS;
    //    VERBOSE << SHORT_FILE_LINE_STAMP << "Free MemoryPool [" << s << " ]" << std::endl;
}

void *MemoryPool::pop(size_t s, int loc) {
    void *addr = nullptr;

    if ((s > MIN_BLOCK_SIZE) && (s < MAX_BLOCK_SIZE)) {
        locker_.lock();

        // find MemoryPool block which is not smaller than demand size
        auto pt = pool_.lower_bound(s);

        if (pt != pool_.end()) {
            size_t ts = 0;
            std::tie(ts, addr) = *pt;
            if (ts < s * 2) {
                s = ts;
                pool_.erase(pt);
                pool_depth_ -= s;
            } else {
                addr = nullptr;
            }
        }
        locker_.unlock();
    }

    if (addr == nullptr) {
        try {
#ifdef __CUDA__
            SP_DEVICE_CALL(cudaMallocManaged(&addr, s));
#else
            addr = malloc(s);
#endif
        } catch (std::bad_alloc const &error) { THROW_EXCEPTION_BAD_ALLOC(s); }
    }
    return addr;
}

// std::shared_ptr<void> sp_alloc_MemoryPool(size_t s) {
//    void *addr = SingletonHolder<MemoryPool>::instance().pop(s);
//    return std::shared_ptr<void>(addr, MemoryPool::deleter_s(addr, s));
//}

}  // namespace simpla
