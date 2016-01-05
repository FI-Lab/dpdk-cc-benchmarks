#ifndef DPI
#define DPI

#include "ac/sm_builder.h"

/*
 * 需要从snort官方的rule集里面，提取content作为key file
 *
 * */

void dpi_engine_init(char *pf);
int dpi_engine_exec(unsigned char *T, int len);
void dpi_engine_close();

#endif
