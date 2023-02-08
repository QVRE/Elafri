#include "gui.h"

void GrText(gr *buf, int x, int y, u32 w, u32 h, u32 len, color clr, char *str) {
	u32 scale_x = (w >> 2) / len, scale_y = h >> 2; //also multiply by 2 to do rounding after
	scale_x = (scale_x >> 1) + (scale_x & 1), scale_y = (scale_y >> 1) + (scale_y & 1); //round

	for (u32 i=0; i<len; i++) { //per character
		u8 str_ch = str[i];
		u64 character = str_ch > 127 ? global_font[127] : global_font[str_ch];
		for (u32 j=0; j < 8; j++)
			for (u32 k=0; k < 8; k++)
				if ((character >> (63-j*8-k)) & 1) {
					int px = x + (k + (i << 3)) * scale_x, py = y + j * scale_y;
					for (u32 sy=0; sy < scale_y; sy++)
						for (u32 sx=0; sx < scale_x; sx++) {
							GrPixel(buf, px + sx, py + sy, clr);
						}
				}
	}
}
