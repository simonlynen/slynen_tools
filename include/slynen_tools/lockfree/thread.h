/*
 *   Written 2009 by <mgix@mgix.com>
 *   This code is in the public domain
 *   See http://www.mgix.com/snippets/?LockFree for details
 *
 *   OS-dependent code, thread manipulation, etc ...
 *
 */
#ifndef __THREAD_H__
    #define __THREAD_H__

    #if defined(WIN32)

        #define WIN32_LEAN_AND_MEAN
        #include <io.h>
        #include <windows.h>

        void *thread(
            void *(*head)(void*),
            void *data = 0
        )
        {
            return CreateThread(
                0,
                0,
                (LPTHREAD_START_ROUTINE)head,
                data,
                0,
                0
            );
        }

        void join(
            void *t
        )
        {
            WaitForSingleObject(
                t,
                INFINITE
            );
        }

    #else // defined(WIN32)

        #include <unistd.h>
        #include <pthread.h>

        void *thread(
            void *(*head)(void*),
            void *data = 0
        )
        {
            pthread_t *t = new pthread_t;
            pthread_create(t, 0, head, data);
            return t;
        }

        void join(
            void *vt
        )
        {
            pthread_t *t = (pthread_t*)vt;
            pthread_join(*t, 0);
            delete t;
        }

    #endif // defined(unix)

#endif // __THREAD_H__

