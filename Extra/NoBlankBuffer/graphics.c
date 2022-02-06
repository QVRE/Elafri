#ifndef _EGFX
#define _EGFX
#include "evar.c"

#define BLACK (color){0}
#define WHITE (color){255,255,255}
#define RED (color){255}
#define GREEN (color){0,255}
#define BLUE (color){0,0,255}
#define YELLOW (color){255,255}
#define CYAN (color){0,255,255}

typedef struct AColor {u8 r,g,b,a;} color;
typedef struct GrBuffer {color *pal; u32 w,h;} gr;

char *grout; //converted sequences get stored here before being printed
u32 groff; //gr offset. How filled grout is

color blankbuf_d[448];
color *blankbuf = blankbuf_d + 128;

static inline void GrInit(u32 grout_size) {
	SineInit();
	grout = malloc(grout_size);
	grout[0] = '\e', grout[1] = '[', grout[2] = 'H'; //first 3 bytes always do home escape code
}

gr GrBuffer(u32 width, u32 height) {
	return (gr){calloc(width*height, sizeof(color)), width, height};
}
static inline void GrPixel(gr *buf, int x, int y, color clr) {
	if (x >= 0 && y >= 0 && x < buf->w && y < buf->h)
		buf->pal[y*buf->w+x] = clr;
}
void GrFill(gr *buf, const color clr) {
	color *ptr = buf->pal; //point to start of graphics buffer
	u32 size = buf->w*buf->h;
	while (size--)
		*ptr++ = clr;
}
void GrFree(gr *buf) {
	free(buf->pal);
}

void GrLine(gr *b, ivec2 A, ivec2 B, color clr) {
	int dx = B.x-A.x; //Δx & Δy
	int dy = B.y-A.y;
	GrPixel(b, B.x, B.y, clr);
	if (abs(dx) > abs(dy))
		for (int c=0,i=A.x; i != B.x; i+=s(dx), c++)
			GrPixel(b, i, A.y+(c*dy)/abs(dx), clr);
	else
		for (int c=0,i=A.y; i != B.y; i+=s(dy), c++)
			GrPixel(b, A.x+(c*dx)/abs(dy), i, clr);
}
void GrTriangleWire(gr *buf, ivec2 A, ivec2 B, ivec2 C, color clr) {
	GrLine(buf, A, B, clr);
	GrLine(buf, B, C, clr);
	GrLine(buf, C, A, clr);
}
void GrTriangle(gr *buf, ivec2 A, ivec2 B, ivec2 C, color clr) {
	ivec2 swap; //swap conditionally to order from highest to lowest point
	if (A.y > B.y)
		swap = A, A = B, B = swap;
	if (B.y > C.y)
		swap = B, B = C, C = swap;
	if (A.y > B.y)
		swap = A, A = B, B = swap;
	int i = A.y;
	F64 s1, s2, l, r; //steps used in loops
	if (B.y > A.y) {
		s1 = (F64)(B.x-A.x) / (B.y-A.y), s2 = (F64)(C.x-A.x) / (C.y-A.y);
		if (s2 < s1) //make sure it's left to right
			l = s1, s1 = s2, s2 = l;
		l=A.x, r=A.x;
		GrPixel(buf, A.x, A.y, clr);
		do { //Draw from Ay -> By
			l += s1, r += s2, i++;
			for (int j=round(l); j<=round(r); j++)
				GrPixel(buf, j, i, clr);
		} while (i < B.y);
	} else if (C.y > B.y) { //means Ay = By so we treat it as upside down triangle
		if (A.x < B.x) //left to right order
			l = A.x, r = B.x, s1 = (F64)(C.x-A.x)/(C.y-A.y), s2 = (F64)(C.x-B.x)/(C.y-B.y);
		else r = A.x, l = B.x, s2 = (F64)(C.x-A.x)/(C.y-A.y), s1 = (F64)(C.x-B.x)/(C.y-B.y);
		goto Draw2Cy;
	}
	if (C.y > B.y) {
		s1 = (F64)(C.x-l) / (C.y-i), s2 = (F64)(C.x-r) / (C.y-i);
Draw2Cy: do { //Draw from By -> Cy
			 for (int j=round(l); j<=round(r); j++)
				GrPixel(buf, j, i, clr);
			 l += s1, r += s2, i++;
		 } while (i <= C.y);
	}
}

void GrCircle(gr *b, const ivec2 O, int radius, int skip, color clr) {
	for (int i=0; i<ROT; i+=skip) //loop through precomputes
		GrPixel(b, O.x+(int)roundf(wcos[i]*radius), O.y+(int)roundf(wsin[i]*radius), clr);
}
void GrCircleFilled(gr *b, const ivec2 O, const int radius, color clr) {
	for (int y=-radius; y<=radius; y++)
		for (int x=radius; x<=radius; x++)
			if (x*x+y*y <= radius*radius) //check if inside
				GrPixel(b, O.x+x, O.y+y, clr);
}

void gremit(u8 val, const u32 end) { //convert u8 to ASCII decimal + add an extra char
	u32 pair = end; //lower part is set to ending character. Things are printed low to high
	u32 size = 1;
	while (val) {
		pair <<= 8;
		pair ^= '0' + val % 10;
		val /= 10;
		size++;
	}
	*(u32*)(&grout[groff]) = pair; //emit pair to grout
	groff += size;
}
void draw(gr *buf) {
	groff = 3; //first 3 bytes will always be the escape code to go to 0,0
	color last = BLACK; //small optimization, kinda similar to RLE
	u32 x=0, y=0;
	color c;

	draw:
	c = buf->pal[y*buf->w+x];
	if (c.r!=last.r || c.g!=last.g || c.b!=last.b) {
		*(u64*)(&grout[groff]) = 0x3b323b38345b1b; // \e[48;2; backwards
		groff += 7;
		gremit(c.r,';'); //emit red value in ASCII + a semicolon at the end (for an ESC seq)
		gremit(c.g,';'); //same with green
		gremit(c.b,'m'); //complete sequence with m for color (escape sequence)
	}
	last = c;
	grout[groff] = (!c.a * ' ') ^ c.a; //4th byte of color var is used as ASCII char
	groff++;
	x++;
	if (x==buf->w) { //at end of line do newline
		grout[groff] = '\n';
		groff++;
		x=0, y++;
		if (y==buf->h) { //check if this is the last line
			*(u32*)(&grout[groff]) = 0x6d305b1b; // \e[0m (set color to default)
			groff += 4;
			write(0,grout,groff); //finally output our buffer
			return;
		}
	}
	goto draw;
}
static inline void drawc(gr *buffer) { //draw and then clear
	draw(buffer);
	GrFill(buffer, BLACK);
}
#endif
