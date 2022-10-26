

#include <ntifs.h>
#include "nlperf.h"

static LARGE_INTEGER   g_nlperf_freq = {0, 0};

VOID
NLPerfInit(
           )
{
    KeQueryPerformanceCounter(&g_nlperf_freq);
    // Frequence for 1 microsecond
    g_nlperf_freq.QuadPart /= 1000000;
}

VOID
NLPerfStart(
            __out PNLPERFCOUNTER Counter
            )
{
    if(g_nlperf_freq.QuadPart == 0) NLPerfInit();
    Counter->elapse = KeQueryPerformanceCounter(NULL);
}

VOID
NLPerfStop(
           __inout PNLPERFCOUNTER Counter
           )
{
    LARGE_INTEGER end;
    end = KeQueryPerformanceCounter(NULL);
    Counter->elapse.QuadPart = end.QuadPart - Counter->elapse.QuadPart;

    // Callculate how many microseconds elapsed.
    Counter->elapse.QuadPart = Counter->elapse.QuadPart / g_nlperf_freq.QuadPart;
}