#pragma once

#include <Windows.h>

enum EMEncodeType
{
    emEncode_Unknown = 0,

    emEncode_UTF8,

    emEncode_Unicode_BigEnd,
    emEncode_Unicode,

    emEncode_UTF32_BigEnd,
    emEncode_UTF32,
};

typedef unsigned long BomCode;

typedef unsigned char byte;