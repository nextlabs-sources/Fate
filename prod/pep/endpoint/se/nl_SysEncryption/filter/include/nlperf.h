

#pragma once
#ifndef __NL_PERFORMANCE_H__
#define __NL_PERFORMANCE_H__


typedef struct _NLPERFCOUNTER
{
    LARGE_INTEGER   elapse; // In microseconds
}NLPERFCOUNTER, *PNLPERFCOUNTER;

VOID
NLPerfInit(
           );

VOID
NLPerfStart(
            __out PNLPERFCOUNTER Counter
            );

VOID
NLPerfStop(
           __inout PNLPERFCOUNTER Counter
           );


#endif