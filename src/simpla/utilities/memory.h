/**
 * Copyright (C) 2007-2011 YU Zhi. All rights reserved.
 * $Id$
 * @file memory_pool.h
 *
 *  created on: 2011-3-2
 *      Author: salmon
 */
#ifndef CORE_UTILITIES_MEMORY_POOL_H_
#define CORE_UTILITIES_MEMORY_POOL_H_

#include <memory>

namespace simpla {

/** @ingroup toolbox
 * @addtogroup memory_pool Memory Pool
 * @{
 * @brief    design to speed up  frequently and repeatedly
 * allocate operation of moderate size array or memory block.
 *
 */

enum { MANAGED_MEMORY, HOST_MEMORY, DEVICE_MEMORY };

int spMemoryAlloc(void **p, size_t s, int location = MANAGED_MEMORY);
int spMemoryFree(void **p, size_t s, int location = MANAGED_MEMORY);
int spMemoryFill(void *dest, size_t, void const *v, size_t else_size);
template <typename T>
int spMemoryFill(T *dest, T v, size_t n) {
    return spMemoryFill(dest, n, &v, sizeof(T));
}

int spMemoryFill(float *dest, float v, size_t n);
int spMemoryFill(double *dest, double v, size_t n);
int spMemoryFill(int *dest, int v, size_t n);
int spMemoryCopy(void *dest, void const *src, size_t s);

struct deleter_s {
    void *addr_;
    size_t m_size_;
    int m_loc_;
    deleter_s(void *p, size_t s, int loc) : addr_(p), m_size_(s), m_loc_(loc) {}
    ~deleter_s() = default;
    deleter_s(const deleter_s &) = default;
    deleter_s(deleter_s &&) = default;
    deleter_s &operator=(const deleter_s &) = default;
    deleter_s &operator=(deleter_s &&) = default;
    inline void operator()(void *ptr) { spMemoryFree(&addr_, m_size_, m_loc_); }
};

template <typename T>
std::shared_ptr<T> spMakeSharedArray(size_t s, int location = MANAGED_MEMORY) {
    void *addr = nullptr;
    spMemoryAlloc(&addr, s * sizeof(T), location);

    return std::shared_ptr<T>(reinterpret_cast<T *>(addr), deleter_s(addr, s * sizeof(T), location));
}

// template <typename T>
// int spMemoryFill(T *d, size_t s, T v) {
//    return spMemoryFill(d, s, &v, sizeof(T));
//}

template <typename T>
int spMemoryCopy(T *dest, T const &src, size_t s) {
    return spMemoryCopy(dest, src, s * sizeof(T));
}

}  // namespace simpla

#endif  // CORE_UTILITIES_MEMORY_POOL_H_
