#pragma once

#if defined(_MSC_VER)
  #pragma warning(push)
  #pragma warning(disable: 4595)
#endif

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Winline-new-delete"
#endif

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Winline"
#endif

#ifdef TRACING_ALLOC
#include <new>
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include <stacktrace>

inline bool& is_tracking_disabled() {
    thread_local bool disabled = false;
    return disabled;
}

inline void print_stacktrace() {
    bool was_disabled = is_tracking_disabled();
    is_tracking_disabled() = true;

    std::cerr << "--- Stack trace ---\n";
    std::cerr << std::stacktrace::current() << '\n';

    is_tracking_disabled() = was_disabled;
}

struct AllocationInfo {
    std::size_t size;
    std::stacktrace trace;
};

struct AllocationTracker {
    std::unordered_map<void*, AllocationInfo> active_ptrs;
    std::mutex mtx;
    bool is_destroyed = false;

    ~AllocationTracker() {
        is_destroyed = true; 
        
        if (!active_ptrs.empty()) {
            std::printf(" WARNING: %zu allocation(s) were leaked!\n", active_ptrs.size());
            
            for (auto& [ptr, info] : active_ptrs) {
                std::printf("\n[LEAK] Address: %p (%zu bytes)\n", ptr, info.size);
                std::cerr << "Allocated by:\n" << info.trace << "\n";
                
                std::free(ptr);
            }
        }
    }
};

inline AllocationTracker& get_tracker() {
    static AllocationTracker tracker;
    return tracker;
}

inline void* operator new(std::size_t size) {
    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }

    if (!is_tracking_disabled()) {
        is_tracking_disabled() = true; 
        
        if (!get_tracker().is_destroyed) {
            auto trace = std::stacktrace::current(1); 
            
            std::lock_guard<std::mutex> lock(get_tracker().mtx);
            get_tracker().active_ptrs[ptr] = { size, std::move(trace) };
        }
        
        is_tracking_disabled() = false;
    }

    return ptr;
}

inline void track_delete(void* ptr) {
    if (!ptr) return;

    if (!is_tracking_disabled()) {
        is_tracking_disabled() = true;
        
        if (!get_tracker().is_destroyed) {
            std::lock_guard<std::mutex> lock(get_tracker().mtx);
            auto& ptrs = get_tracker().active_ptrs;
            auto it = ptrs.find(ptr);
            
            if (it != ptrs.end()) {
                ptrs.erase(it);
            } else {
                std::printf("\n[!!!] CRITICAL ERROR: Double free or untracked free at %p\n\n", ptr);
                print_stacktrace();
            }
        }
        
        is_tracking_disabled() = false;
    }
    
    std::free(ptr);
}

inline void operator delete(void* ptr) noexcept { track_delete(ptr); }
inline void operator delete(void* ptr, std::size_t) noexcept { track_delete(ptr); }

inline void* operator new[](std::size_t size) { return operator new(size); }
inline void operator delete[](void* ptr) noexcept { track_delete(ptr); }
inline void operator delete[](void* ptr, std::size_t) noexcept { track_delete(ptr); }

#endif

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#if defined(_MSC_VER)
  #pragma warning(pop)
#endif