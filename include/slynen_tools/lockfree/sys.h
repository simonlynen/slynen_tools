#ifndef __SYS_H__
    #define __SYS_H__

    #if defined(__ppc__) || defined(__PPC__)
        #define XX_PPC
    #endif

    #if defined(__i386__)
        #define XX_386
    #endif

    #if defined(__x86_64__)
        #define XX_86_84
    #endif

    #if defined(WIN32)
        #define XX_WIN32
    #endif

    #if defined(linux)
        #define XX_LINUX
    #endif

    #if defined(__CYGWIN32__)
        #define XX_CYGWIN
    #endif

    #if defined(__APPLE__) && defined(__MACH__)
        #define XX_OSX
    #endif

    #ifdef __GNUC__
        #define XX_GCC
    #endif

    #if defined(DEBUG) || defined(_DEBUG)
        #define XX_DEBUG
    #endif

    #ifdef _MSC_VER
        #define XX_MSVC
        #define VC_EXTRALEAN
        #define _SECURE_SCL_DEPRECATE 0
        #define _CRT_SECURE_NO_DEPRECATE
        #define _CRT_NONSTDC_NO_DEPRECATE
    #endif

    #if defined(XX_LINUX) || defined(XX_CYGWIN) || defined(XX_OSX)
        #define XX_POSIX
    #endif

    #if defined(XX_POSIX)
        #include <time.h>
        #include <stdint.h>
        #include <stdlib.h>
        #include <unistd.h>
        #include <sys/time.h>
    #endif

    #if defined(XX_OSX)
        #include <fenv.h>
        #include <sys/types.h>
        #include <sys/sysctl.h>
    #endif

    #if defined(XX_WIN32)

        #include <stdlib.h>
        #include <inttypes.h>

    #endif

    // ----------------------------------------------------------------------------------------------------
    #include <math.h>
    #include <stdio.h>
    #include <stdarg.h>
    #include <string.h>

    // ----------------------------------------------------------------------------------------------------
    #define CHECKEQ(x, y) CHECK(fabs((x)-(y))<1e-7)
    #define CHECKIMPLY(x, y) (((x)==false) || ((x)==true && (y)==true))
    #define CHECKEQF(x, y) CHECK(Traits<Float>::abs((x)-(y))<Float(1e-7))

    #define CHECK(x)                                                \
    {                                                               \
        if(false==(x))                                              \
        {                                                           \
            printf(                                                 \
                "\nIn file %s, at line %d, assertion %s failed\n",  \
                __FILE__,                                           \
                __LINE__,                                           \
                #x                                                  \
            );                                                      \
        }                                                           \
    }                                                               \

    // ----------------------------------------------------------------------------------------------------
    class Sys
    {
    public:

        // ----------------------------------------------------------------------------------------------------
        static void *m128Alloc(
            int nbBytes
        )
        {
            nbBytes += sizeof(void*)+16;
            uint8_t *ptr = (uint8_t*)::malloc(nbBytes);
            uint8_t *sPtr = sizeof(void*) + ptr;
            intptr_t iPtr = (intptr_t)sPtr;
            iPtr += 0xF;
            iPtr |= 0xF;
            iPtr -= 0xF;

            ((void**)iPtr)[-1] = ptr;
            return (void*)iPtr;
        }

        // ----------------------------------------------------------------------------------------------------
        static void *m128CAlloc(
            int nbBytes
        )
        {
            void *p = m128Alloc(nbBytes);
            memset(p, 0, nbBytes);
            return p;
        }

        // ----------------------------------------------------------------------------------------------------
        static void m128Free(
            void *storage
        )
        {
            void *ptr = ((void**)storage)[-1];
            ::free(ptr);
        }
    };

#endif  // __SYS_H__

