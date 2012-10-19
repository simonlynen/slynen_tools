/*
 *   Written 2010 by <mgix@mgix.com>
 *   This code is in the public domain
 *   See http://www.mgix.com/snippets/?LockFree for details
 *
 *   Timers
 */
#ifndef __TIMER_H__
    #define __TIMER_H__

    #include <slynen_tools/lockfree/sys.h>

    struct Timer
    {
    private:

        static uint64_t rdtsc()
        {
            uint64_t r;

            #if defined(__i386__)

                #if defined(__PIC__)
                    #define CLOB_EBX
                    #define PUSH_EBX "pushl  %%ebx\n"
                    #define POP_EBX  "popl   %%ebx\n"
                #else
                    #define CLOB_EBX "ebx",
                    #define PUSH_EBX "\n"
                    #define POP_EBX "\n"
                #endif

              asm volatile(
                  PUSH_EBX
                  "cpuid          \n"   // finish up in-flight stuff (and crush 4 registers)
                  "rdtsc          \n"   // timer
                  POP_EBX
                  : "=A" (r)
                  :
                  : CLOB_EBX
                    "ecx",
                    "cc"
              );

            #endif

            #if defined(__x86_64__)
              asm volatile(
                  "cpuid          \n"   // finish up in-flight stuff (and crush 4 registers)
                  "rdtsc          \n"   // timer
                  : "=a" (r)
                  :
                  : "rbx",
                    "rcx",
                    "rdx",
                    "cc"
              );
            #endif

            return r;
        }

    public:
        static double nano()
        {
            struct timeval tv;
            gettimeofday(&tv, 0);
            return (tv.tv_sec*1e6 + tv.tv_usec)*1e3;
        }

        static uint64_t cycles()
        {
            return rdtsc();
        }
    };

#endif	// __TIMER_H__

