/*
 * SRT - Secure, Reliable, Transport
 * Copyright (c) 2018 Haivision Systems Inc.
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 */

/*****************************************************************************
written by
   Haivision Systems Inc.
 *****************************************************************************/

#include "win/wintime.h"
#include <sys/timeb.h>

void SRTCompat_timeradd(struct timeval *a, struct timeval *b, struct timeval *result)
{
    result->tv_sec  = a->tv_sec + b->tv_sec;
    result->tv_usec = a->tv_usec + b->tv_usec;
    if (result->tv_usec >= 1000000)
    {
        result->tv_sec++;
        result->tv_usec -= 1000000;
    }
}

int SRTCompat_gettimeofday(struct timeval* tp, struct timezone* tz)
{
#define FIXED_ISSUE_669
#if defined(FIXED_ISSUE_669)
    // https://github.com/Haivision/srt/issues/669#issuecomment-527062037
    struct timeb tb;
    ftime(&tb);
    tp->tv_sec  = (long)tb.time;
    tp->tv_usec = 1000*tb.millitm;
    return 0;
#else //defined(FIXED_ISSUE_669)
    static LARGE_INTEGER tickFrequency, epochOffset;

    // For our first call, use "ftime()", so that we get a time with a proper epoch.
    // For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.
    static int isFirstCall = 1;

    LARGE_INTEGER tickNow;
    QueryPerformanceCounter(&tickNow);

    if (isFirstCall)
    {
        struct timeb tb;
        ftime(&tb);
        tp->tv_sec  = (long)tb.time;
        tp->tv_usec = 1000*tb.millitm;

        // Also get our counter frequency:
        QueryPerformanceFrequency(&tickFrequency);

        // And compute an offset to add to subsequent counter times, so we get a proper epoch:
        epochOffset.QuadPart = tb.time*tickFrequency.QuadPart + (tb.millitm*tickFrequency.QuadPart)/1000 - tickNow.QuadPart;

        isFirstCall = 0; // for next time
    }
    else
    {
        // Adjust our counter time so that we get a proper epoch:
        tickNow.QuadPart += epochOffset.QuadPart;

        tp->tv_sec = (long) (tickNow.QuadPart / tickFrequency.QuadPart);
        tp->tv_usec = (long) (((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);
    }
    return 0;
#endif //defined(FIXED_ISSUE_669)
}
