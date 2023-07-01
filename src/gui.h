#pragma once
#include "graphics.h"
#include "evar.h"

//the global font used to draw 8x8 bit characters on the screen. First 32 are used as symbols
extern u64 global_font[128];

//draws text at x,y with width,height (in pixels). Scaling is approximated
void GrText(gr *buf, int x, int y, u32 w, u32 h, u32 len, color clr, char *str);
