// #include "stdafx.h"
/*#pragma hdrstop */
#include <stdio.h>

extern "C"
{
	int __mb_cur_max;
	unsigned short* _pctype;
	FILE _iob[3] = {__iob_func()[0], __iob_func()[1], __iob_func()[2]}; 
};