#ifndef _EGFX
#define _EGFX
#include "evar.c"

#define BLACK (color){0,0,0}
#define WHITE (color){255,255,255}
#define RED (color){255,0,0}
#define GREEN (color){0,255,0}
#define BLUE (color){0,0,255}
#define YELLOW (color){255,255,0}
#define CYAN (color){0,255,255}

#define ROT 1024

typedef struct AColor {u8 r,g,b,a;} color;
typedef struct GrBuffer {color **pal; u32 w,h,xBlank,yBlank;} gr;

F32 sinebuf[ROT*2+ROT/4]; //just some precomputes
F32 *wsin;
F32 *wcos;

color blankbuf[512];

static inline void GrInit() {
    for (int i=0; i<ROT*2+ROT/4; i++) //Initialize sinewave precomputes
        sinebuf[i] = sinf((2*pi/ROT)*i);
    wsin = sinebuf+ROT;
    wcos = &wsin[ROT/4];
}

gr GrBuffer(u32 width,u32 height,u32 xBlank,u32 yBlank) {
    gr buf;
    //allocate line pointers
    buf.pal = malloc((height+yBlank*2)*sizeof(color*)) + yBlank*sizeof(color*);
    buf.pal[0] = calloc((width+xBlank)*height,sizeof(color));

    //make blanking lines and spaces so things like circles don't clip
    for (int i=1; i<height; i++) {
        if (i<=yBlank) {
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
void GrFill(gr buf, const color clr) {
    color *ptr = buf.pal[0];
    u32 size = (buf.w+buf.xBlank)*buf.h;
    while (size--)
        *ptr++ = clr;
}
void GrFree(gr buf) {
    free(buf.pal[0]);
    free(buf.pal);
}

void GrLine(gr b, uvec2 A, uvec2 B, color clr) {
    int dx = B.x-A.x; //Δx & Δy
    int dy = B.y-A.y;
    b.pal[B.y][B.x] = clr;
    if (abs(dx) > abs(dy))
        for (int c=0,i=A.x; i != B.x; i+=s(dx), c++)
            b.pal[A.y+(c*dy)/abs(dx)][i] = clr;
    else
        for (int c=0,i=A.y; i != B.y; i+=s(dy), c++)
            b.pal[i][A.x+(c*dx)/abs(dy)] = clr;
}
void GrTriangle(gr buf, uvec2 A, uvec2 B, uvec2 C, color clr) {
    GrLine(buf, A, B, clr);
    GrLine(buf, B, C, clr);
    GrLine(buf, C, A, clr);
}

void GrCircle(gr b, const uvec2 O, int radius, int skip, color clr) {
    for (int i=0; i<ROT; i+=skip)
        b.pal[(int)O.y+(int)round(wsin[i]*radius)][(int)O.x+(int)round(wcos[i]*2*radius)] = clr;
}
void GrCircleFilled(gr b, const uvec2 O, int radius, color clr) {
    for(int y=-radius; y<=radius; y++)
        for(int x=-2*radius; x<=2*radius; x++)
            if(((x*x)>>2)+y*y <= radius*radius)
                b.pal[(int)O.y+y][(int)O.x+x] = clr;
}

void draw(gr buf) {
    u32 y=0,x=1;
    color c = buf.pal[0][0];
    printf("\e[H\e[48;2;%d;%d;%dm ", c.r, c.g, c.b);

    print:
    c = buf.pal[y][x];
    printf("\e[48;2;%d;%d;%dm ", c.r, c.g, c.b);
    x++;
    if (x==buf.w) {
        printf("\e[0m\n");
        x=0;
        y++;
        if (y==buf.h) return;
    }
    goto print;
}
static inline void drawc(gr buffer) { //draw and then clear
    draw(buffer);
    GrFill(buffer, BLACK);
}
#endif
