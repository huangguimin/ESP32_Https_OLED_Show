#ifndef UTOG_H
#define UTOG_H

#include "global.h"

char *strrpc(char *str,char *oldstr,char *newstr);
/*UNICODE码转为GB2312码*/
int Utf8ToGb2312(const char* utf8, char *temp);

#endif
