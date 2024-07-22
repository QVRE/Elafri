#include <stdlib.h>
#include <stdio.h>
#include "image.h"

//downsamples an image by applying the new image's grid onto the old image and getting the
//average values of the sums of the old pixels inside every grid block
gr Downsample(gr *buf, u32 new_w, u32 new_h) {
	gr new_buf = GrBuffer(new_w, new_h);
	u64 x_scale = ((u64)buf->w << 48) / new_w; //use a fixed point number for decimal precision
	u64 y_scale = ((u64)buf->h << 48) / new_h; //we will use 48 bits for the fractional part
	u64 x, y=0, x_next, y_next, x_next_frac=0, y_next_frac=0, x_next_r, y_next_r, scale;

	//multiply the x & y downscaling factors together to get the downscaled area ([32].[32])
	scale = ((x_scale >> 32) + ((x_scale >> 31)&1)) * ((y_scale >> 32) + ((y_scale >> 31)&1));
	//inverted 0 is basically 1 followed by 64 decimal bits which means that there are 32 bits
	//left to hold the fractional value of the result. Since this is 1/x, the other 32 are zero
	scale = ~(u64)0 / scale;

	for (u32 i=0; i<new_h; i++) {
		x=0, x_next_frac=0;
		y_next_frac += y_scale; //value of where the next pixel's y in the old buffer starts
		y_next_r = (y_next_frac >> 48) + 1; //get pixel position after the last full pixel
		y_next = y_next_r - (y_next_frac << 16 == 0); //if no fraction, discard that other pixel

		for (u32 j=0; j<new_w; j++) {
			x_next_frac += x_scale;
			x_next_r = (x_next_frac >> 48) + 1;
			x_next = x_next_r - (x_next_frac << 16 == 0);

			u64 r=0, g=0, b=0, a=0; //RGBA accumulators
			u64 x_start = x >> 48, y_start = y >> 48;
			for (u32 u = y_start; u < y_next; u++)
				for (u32 v = x_start; v < x_next; v++) {
					u64 mult = 0x100000000; //start with a multiplier of 1 (32 decimal bits)
					//account for pixels that are partially inside by lowering the multiplier
					if (u == y_start) mult *= 0x10000000 - ((y >> 20) & 0xfffffff), mult >>= 28;
					if (v == x_start) mult *= 0x10000000 - ((x >> 20) & 0xfffffff), mult >>= 28;
					if (u == y_next_r-1) mult *= (y_next_frac >> 20) & 0xfffffff, mult >>= 28;
					if (v == x_next_r-1) mult *= (x_next_frac >> 20) & 0xfffffff, mult >>= 28;
					const color clr = buf->dat[u * buf->w + v];
					r += mult * clr.r, g += mult * clr.g, b += mult * clr.b, a += mult * clr.a;
				}
			r = (r >> 16) * scale, r >>= 48, g = (g >> 16) * scale, g >>= 48, //divide values
			b = (b >> 16) * scale, b >>= 48, a = (a >> 16) * scale, a >>= 48;
			new_buf.dat[i * new_w + j] = (color){r,g,b,a}; //store the result
			x = x_next_frac;
		}
		y = y_next_frac;
	}
	return new_buf;
}

// Portable Pixel Map (PPM) | Does not store Alpha values | ffmpeg supports conversion for this
// http://davis.lbl.gov/Manuals/NETPBM/doc/ppm.html

gr ReadPPM(char *filename) {
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 3, SEEK_SET); //skip magic bytes (P6\n)
	u32 w,h,m; //width, height and max value which is always assumed to be 255
	fscanf(fp, "%u %u %u ", &w, &h, &m);
	gr img = GrBuffer(w,h);

	u8 *tmp = malloc(w*h*3);
	fread(tmp, 3, w*h, fp);
	fclose(fp);
	for (u32 i=0; i<w*h; i++)
		img.dat[i] = (color){tmp[i*3], tmp[i*3+1], tmp[i*3+2], 255};

	free(tmp);
	return img;
}

void WritePPM(char *filename, gr *buf) {
	FILE* fp = fopen(filename, "wb");
	fprintf(fp, "P6\n%u %u\n255\n", buf->w, buf->h);
	u8 *tmp = malloc(buf->w*buf->h*3);
	for (u32 i=0; i<buf->w*buf->h; i++) {
		const color clr = buf->dat[i];
		tmp[i*3] = clr.r, tmp[i*3+1] = clr.g, tmp[i*3+2] = clr.b;
	}
	fwrite(tmp, 3, buf->w*buf->h, fp);
	fclose(fp);
	free(tmp);
}

// Quite OK Image (QOI) | Not efficient with Alpha values
// https://qoiformat.org/qoi-specification.pdf

gr ReadQOI(char *filename) {
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 4, SEEK_SET); //skip magic bytes (qoif)
	u32 w, h; //width & height
	fread(&w, 4, 1, fp);
	fread(&h, 4, 1, fp); //stored in Big Endian, will convert to Little Endian below
	w = (w >> 24) | (w << 24) | ((w >> 8) & 0xFF00) | ((w << 8) & 0xFF0000);
	h = (h >> 24) | (h << 24) | ((h >> 8) & 0xFF00) | ((h << 8) & 0xFF0000);
	fseek(fp, 0, SEEK_END);
	u64 data_size = ftell(fp) - 22; //get size of data minus header and 8 byte end marker size
	fseek(fp, 14, SEEK_SET); //offseted by header size
	gr img = GrBuffer(w,h);

	color pix = {0, 0, 0, 255};
	color table[64]; //a running array
	for (u32 i = 0; i < 64; i++) table[i] = (color){0,0,0,0};
	u8 *dat = malloc(data_size);
	fread(dat, 1, data_size, fp);
	fclose(fp);
	u32 offset = 0;
	for (u32 i = 0; i < w*h; i++) {
		u8 byte = dat[offset];
		switch (byte >> 6) {
			case 0: //index op, table lookup
				pix = table[byte & 63];
				break;
			case 1: //diff op, slight differences (-2..1) from previous pixel per channel
				pix.r += (int)((byte >> 4) & 3) - 2;
				pix.g += (int)((byte >> 2) & 3) - 2;
				pix.b += (int)(byte & 3) - 2;
				break;
			case 2: //luma op, bigger diffrences (-32..31 for green, -8..7 + dG for R & B)
				int dg = (int)(byte & 63) - 32; //get delta green
				pix.g += dg;
				offset++; //grab next byte
				byte = dat[offset];
				pix.r += (int)((byte >> 4) & 15) - 8 + dg; //do 4 bit delta + add delta green
				pix.b += (int)(byte & 15) - 8 + dg; //same as above
				break;
			default: //run, rgb or rgba op, we need subcases to confirm
				byte &= 63;
				switch (byte) {
					case 62:
						pix.r = dat[offset+1];
						pix.g = dat[offset+2];
						pix.b = dat[offset+3];
						offset += 3;
						break;
					case 63:
						pix = *(color*)&dat[offset+1];
						offset += 4;
						break;
					default:
						do {
							img.dat[i] = pix;
							i++;
						} while (byte--);
						i--;
						break;
				}
				break;
		}
		offset++;
		table[(pix.r * 3 + pix.g * 5 + pix.b * 7 + pix.a * 11) & 63] = pix;
		img.dat[i] = pix;
	}

	free(dat);
	return img;
}

void WriteQOI(char *filename, gr *buf) {
	u8 channels = 4, color_space = 0;
	u32 w = buf->w, h = buf->h; //width & height
	w = (w >> 24) | (w << 24) | ((w >> 8) & 0xFF00) | ((w << 8) & 0xFF0000); //Big Endian
	h = (h >> 24) | (h << 24) | ((h >> 8) & 0xFF00) | ((h << 8) & 0xFF0000);
	FILE* fp = fopen(filename, "wb");
	fprintf(fp, "qoif");
	fwrite(&w, 4, 1, fp);
	fwrite(&h, 4, 1, fp);
	fwrite(&channels, 1, 1, fp);
	fwrite(&color_space, 1, 1, fp);

	color pix = {0, 0, 0, 255};
	color table[64]; //a running array
	for (u32 i = 0; i < 64; i++) table[i] = (color){0,0,0,0};
	u32 run = 0, size = 0, max_size = 8192; //start with 8KB allocated
	u8 *dat = malloc(max_size + 8);
	for (u32 i = 0; i < buf->w*buf->h; i++) {
		const color cur = buf->dat[i]; //get current pixel
		u32 hash = (cur.r * 3 + cur.g * 5 + cur.b * 7 + cur.a * 11) & 63;
		if (cur.r == pix.r && cur.g == pix.g && cur.b == pix.b && cur.a == pix.a) {
			run++;
			if (run == 62) dat[size] = 0xFD, size++, run = 0; //maximum run number reached
		} else {
			if (run > 0) dat[size] = 191 + run, size++, run = 0;
			//check if hash exists in table
			color lookup = table[hash];
			if (cur.r == lookup.r && cur.g == lookup.g && cur.b == lookup.b && cur.a == lookup.a) {
				dat[size] = hash, size++;
				goto loop_around;
			}
			if (cur.a == pix.a) { //if Alpha stayed the same
				int dr = (int)cur.r - pix.r, dg = (int)cur.g - pix.g, db = (int)cur.b - pix.b;
				if (dr >= -2 && dr < 2 && dg >= -2 && dg < 2 && db >= -2 && db < 2) {
					dat[size] = 64 | ((dr+2) << 4) | ((dg+2) << 2) | (db+2), size++;
					goto loop_around; //DIFF op
				}
				if (dg >= -32 && dg < 32 && dr-dg >= -8 && dr-dg < 8 && db-dg >= -8 && db-dg < 8) {
					dat[size] = 128 | (u8)(dg+32);
					dat[size+1] = ((u8)(dr-dg+8)<<4) | (u8)(db-dg+8), size += 2;
					goto loop_around; //LUMA op
				}
				dat[size] = 254, dat[size+1] = cur.r, dat[size+2] = cur.g, dat[size+3] = cur.b;
				size += 4; //output RGB
			} else {
				dat[size] = 255, size++;
				dat[size] = cur.r, dat[size+1] = cur.g, dat[size+2] = cur.b, dat[size+3] = cur.a;
				size += 4; //output RGBA
			}
		}
		loop_around: pix = cur, table[hash] = cur;
		if (size > max_size) {
			max_size *= 2;
			dat = realloc(dat, max_size + 8);
		}
	}
	if (run > 0) dat[size] = 191 + run, size++;

	fwrite(dat, 1, size, fp);
	u64 end = 0x0100000000000000; //marks the end of the file
	fwrite(&end, 8, 1, fp);
	fclose(fp);
	free(dat);
}
