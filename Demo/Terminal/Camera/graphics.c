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
typedef struct GrBuffer {color **pal; u32 w,h,xBlank,yBlank;} gr;

char *grout; //converted sequences get stored here before being printed
u32 groff; //gr offset. How filled grout is

color blankbuf_d[448];
color *blankbuf = blankbuf_d + 128;

static inline void GrInit(u32 grout_size) {
    SineInit();
    grout = malloc(grout_size);
    grout[0] = '\e', grout[1] = '[', grout[2] = 'H'; //first 3 bytes always do home escape code
}

gr GrBuffer(u32 width,u32 height,u32 xBlank,u32 yBlank) {
    gr buf;
    //allocate line pointers --- still has an old bug here
    buf.pal = (color**)malloc((height+yBlank*2)*sizeof(color*)) + yBlank;
    buf.pal[0] = calloc((width+xBlank)*height+xBlank,sizeof(color)) + xBlank;

    //make blanking areas so that out-of-screen graphics doesn't always crash or clip
    for (int i=1; i<height; i++) {
        if (i<=yBlank) { //assuming yBlank <= height
            buf.pal[-i] = blankbuf;
            buf.pal[height-1+i] = blankbuf;
        }
        buf.pal[i] = buf.pal[i-1]+width+xBlank;
    }
    buf.w = width;
    buf.h = height;
    buf.xBlank = xBlank;
    buf.yBlank = yBlank;
    return buf;
}
void GrFill(gr *buf, const color clr) {
    color *ptr = buf->pal[0]; //point to start of graphics buffer
    u32 size = (buf->w+buf->xBlank)*buf->h;
    while (size--)
        *ptr++ = clr;
}
void GrFree(gr buf) {
    free(buf.pal[0]);
    free(buf.pal);
}

void GrCircle(gr *b, const ivec2 O, int radius, int skip, color clr) {
    for (int i=0; i<ROT; i+=skip) //loop through precomputes
        b->pal[O.y+(int)roundf(wsin[i]*radius)][O.x+(int)roundf(wcos[i]*2*radius)] = clr;
}
void GrCircleFilled(gr *b, const ivec2 O, const int radius, color clr) {
    for(int y=-radius; y<=radius; y++)
        for(int x=-2*radius; x<=2*radius; x++)
            if(((x*x)>>2)+y*y <= radius*radius) //check if inside
                b->pal[O.y+y][O.x+x] = clr;
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
    c = buf->pal[y][x];
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
