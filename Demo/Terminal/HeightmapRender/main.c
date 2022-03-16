#include "Elafri.c"
#include "mix.c"

#define FPS 60
#define MPS 1000000 // s/μs

color *map;
u32 mapS;

F32 smooth_inter(F32 x, F32 y, F32 s) {
    return x + (s*s*(3-s*2)) * (y-x);
}
u8 noise2D(F32 x, F32 y, F32 freq, F32 loop, u64 seed) {
    x *= freq, y *= freq, loop *= freq;
    u32 x_int = x;
    u32 y_int = y;
    F32 x_frac = x - x_int;
    F32 y_frac = y - y_int;
    u32 rx = modF32(x_int+1,loop);
    u32 ry = (u32)modF32(y_int+1,loop) << 16;
    u32 s = Mix(x_int+(y_int<<16),seed) & 255;
    u32 r = Mix(rx+(y_int<<16),seed) & 255;
    u32 d = Mix(x_int+ry,seed) & 255;
    u32 dr = Mix(rx+ry,seed) & 255;
    F32 low = smooth_inter(s, r, x_frac);
    F32 high = smooth_inter(d, dr, x_frac);
    return round(smooth_inter(low, high, y_frac));
}
void Shade(color *map) {
    for (int i=0; i<mapS; i++)
        for (int j=0; j<mapS; j++) {
            const u8 curh = map[i*mapS+j].a;
            for (u32 k=1; k<=curh >> 5; k++) {
                color *c = &map[mod32(i-(int)k,mapS)*mapS+j];
                if (c->a < curh-k) {
                    c->r = ((u32)c->r * 62) >> 6;
                    c->g = ((u32)c->g * 62) >> 6;
                    c->b = ((u32)c->b * 62) >> 6;
                }
            }
        }
}

int main()
{
    ElafriInit();

    gr Gr = GrBuffer(res.x,res.y,res.x/2,res.y/2);
    pkg lvl = ReadPkg("data.pkg");
    //change level to 0
    //mapS = lvl.files[0].elsize;
    //map = lvl.files[0].dat;
    mapS = 1000;
    map = malloc(mapS*mapS*sizeof(color));
    for (u32 i=0; i<mapS; i++)
        for (u32 j=0; j<mapS; j++) {
            u8 h = noise2D(i,j,0.01,mapS,1) >> 1;
            h += noise2D(i,j,0.02,mapS,11) >> 2;
            h += noise2D(i,j,0.04,mapS,111) >> 3;
            h += noise2D(i,j,0.08,mapS,1111) >> 4;
            h += noise2D(i,j,0.16,mapS,11111) >> 5;
            const u8 g = 160*(h>64);
            const u8 b = 100*(h>64) + 255*(h<=64);
            map[j*mapS+i] = (color){h,g,b,h*(h>64)+64*(h<=64)};
        }
    Shade(map);

    int *ybuf = malloc(res.x*sizeof(int));
    vec2 pos = {mapS/2,0};
    F32 height = 250;
    const F32 ratio = res.x*0.02;
    int pitch = 0;
    int rot = 0;
    vec2 vFront = {0,1};
    vec2 vLeft = {-1,1};
    vec2 vRight = {1,1};
    int moving = 0;

    /*Timekeeping-related Inits*/
    InitTimer(ftimer); //timer
    struct timeval mtv = {0,1};
    u32 fps = FPS;
    int exec_time, wt;
    F32 delta=0; //Δt per frame

    RenderLoop:
        StartTimer(ftimer);

        Input(&mtv);
        for (u32 i=0; i<kbdSize; i++) {
            if (kbd[i] > '0' && kbd[i] < '7') {
                const u8 j = kbd[i]-'1';
                mapS = lvl.files[j].elsize;
                map = lvl.files[j].dat; //change map
            } else switch (kbd[i]) {
                case 'w':
                    moving = min(moving+50,50);
                    break;
                case 's':
                    moving = max(moving-50,-50);
                    break;
                case 'd':
                    rot = mod32(rot+ROT/100,ROT);
                    goto UpdateRotation;
                case 'a':
                    rot = mod32(rot-ROT/100,ROT);
                UpdateRotation:
                    vFront = (vec2){wsin[rot],wcos[rot]};
                    vRight = (vec2){vFront.x+wcos[rot],vFront.y-wsin[rot]};
                    vLeft = (vec2){vFront.x-wcos[rot],vFront.y+wsin[rot]};
                    break;
                case 'e':
                    height+=10;
                    break;
                case 'q':
                    height-=10;
                    break;
                case 255:
                    i++;
                    switch (kbd[i]) {
                        case UP:
                            pitch++;
                            break;
                        case DOWN:
                            pitch--;
                            break;
                    }
                    break;
            }
        }
        pos.x += vFront.x*delta*moving;
        pos.y += vFront.y*delta*moving;

        GrFill(&Gr, (color){0,190,210});
        for (u32 i=0; i<res.x; i++)
            ybuf[i] = res.y; //reset all height

        vec2 pleft, pright; //points left & right on screen in map coords
        F32 dPx, dPy; //how fast we scan accross
        int pHeight; //height of pixel on screen
        for (F32 d=2; d<height*.7; d++) {
            pleft = (vec2){pos.x+vLeft.x*d, pos.y+vLeft.y*d};
            pright = (vec2){pos.x+vRight.x*d, pos.y+vRight.y*d};
            dPx = (pright.x-pleft.x)/res.x;
            dPy = (pright.y-pleft.y)/res.x;

            for (int x=0; x<res.x; x++) {
                const ivec2 mapPos = {modF32(pleft.x,mapS),modF32(pleft.y,mapS)};
                const color mapClr = map[mapPos.y*mapS+mapPos.x];
                pHeight = (height-mapClr.a) / d * ratio + pitch;
                for (int y=pHeight; y<ybuf[x]; y++)
                    if (y>=0)
                        Gr.pal[y][x] = mapClr, Gr.pal[y][x].a = 0;
                if (pHeight < ybuf[x])
                    ybuf[x] = pHeight;
                pleft.x += dPx;
                pleft.y += dPy;
            }
        }
        draw(&Gr);

        StopTimer(ftimer);
        exec_time = mod32(dt_usec(ftimer),MPS); //Compute execution Δt
        wt = MPS/fps-exec_time; //Check if behind/ahead
        fps = max(min(MPS*fps/(MPS-wt*fps), FPS), 1); //Compute new fps
        delta = 1. / fps; //sligtly accurate Δt

        ftimer_tend.tv_usec = max(MPS/FPS-exec_time, 0);
        ftimer_tend.tv_sec=0;
        select(1, NULL, NULL, NULL, &ftimer_tend); //wait if over max fps
    goto RenderLoop;
}
