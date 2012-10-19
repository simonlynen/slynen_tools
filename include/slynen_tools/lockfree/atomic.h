/*
 *   Written 2009, 2010 by <mgix@mgix.com>
 *   This code is in the public domain
 *   See http://www.mgix.com/snippets/?LockFree for details
 *
 *   Various atomic primitives
 */
#ifndef __ATOMIC_H__
    #define __ATOMIC_H__

    #include <assert.h>
    #include <inttypes.h>

    static void atomicAdd32(
        int32_t volatile *mem,
        int32_t          delta
    )
    {
        asm volatile(
            "lock; addl %0,(%1)"
                :
                :   "r"  (delta),
                    "r"  (mem)
                :   "memory",
                    "cc"
        );
    }

    static inline int32_t atomicGetThenAdd32(
        int32_t volatile *counter,
        int32_t          delta
    )
    {
        asm volatile
        (
            "lock; xaddl %0, %1"
                :   "=r"(delta)
                :   "m"(*counter),
                    "0"(delta)
        );
        return delta;
    }


    #if defined(__x86_64__)

        static void atomicAdd64(
            int64_t volatile *mem,
            int64_t          delta
        )
        {
            asm volatile(
                "lock; addq %0,(%1)"
                    :
                    :   "r"  (delta),
                        "r"  (mem)
                    :   "memory",
                        "cc"
            );
        }

        static inline int64_t atomicGetThenAdd64(
            int64_t volatile *counter,
            int64_t          delta
        )
        {
            asm volatile
            (
                "lock; xaddq %0, %1"
                    :   "=r"(delta)
                    :   "m"(*counter),
                        "0"(delta)
            );
            return delta;
        }

    #endif // defined(__x86_64__)

#endif // __ATOMIC_H__

