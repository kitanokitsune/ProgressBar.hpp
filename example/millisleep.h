#ifndef __MILLISLEEP_H__
#define __MILLISLEEP_H__

/* Prepare a sleep function: millisleep() */
/* POSIX and Windows use different.       */

#ifdef _WIN32
#   include <windows.h>  /* Sleep() : Windows' timer resolution is 15.6ms */
    static inline void millisleep(unsigned long x) {
        Sleep((DWORD)x);
    }
#   define shortsleep() Sleep(0)
#else
#   include <time.h>     /* nanosleep() */
    static inline void millisleep(unsigned long x) {
        struct timespec ts;
        ts.tv_sec  = x / 1000L;
        /* ts.tv_nsec = (x % 1000L) * 1000000L; */
        ts.tv_nsec = (x - ts.tv_sec * 1000L) * 1000000L;
        nanosleep(&ts, NULL);
    }
#   define shortsleep() millisleep(1)
#endif

#endif /* __MILLISLEEP_H__ */