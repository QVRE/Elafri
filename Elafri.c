#ifndef _ELAFRI
#define _ELAFRI

//just simpler names
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long
#define F32 float
#define F64 double

#define max(x,y) ((x) > (y) ? (x) : (y))
#define min(x,y) ((x) < (y) ? (x) : (y))
#define pi 3.1415926535897932384626433832795

#define INSERT 50
#define DELETE 51
#define PgUP 53
#define PgDown 54
#define UP 65
#define DOWN 66
#define RIGHT 67
#define LEFT 68
#define END 70
#define HOME 72

#define InitTimer(t) struct timeval t ## _tstart, t ## _tend
#define StartTimer(t) gettimeofday(&t ## _tstart, NULL)
#define StopTimer(t) gettimeofday(&t ## _tend, NULL)
#define dt_sec(t) (t ## _tend.tv_sec-t ## _tstart.tv_sec)
#define dt_usec(t) (t ## _tend.tv_usec-t ## _tstart.tv_usec)
#define Sleep(time) select(1, NULL, NULL, NULL, &time)

typedef struct FVector2 {F32 x,y;} vec2;
typedef struct UVector2 {u32 x,y;} uvec2;
typedef struct IVector2 {int x,y;} ivec2;

int s(int x) { //get sign of 32 bit integer
    return -(((u32)x>>31)*2-1);
}
int mod32(int x, int m) { //the way actual mod works
    return (x%m + m)%m;
}
long mod64(long x, long m) {
    return (x%m + m)%m;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <termios.h> //for term flags
#include <sys/select.h>
#include <sys/time.h>
#include "graphics.c"

u8 input[256]; //we read stdin to here
u8 kbd[64]; //parsed keyboard input
u32 kbdSize, mC; //mouse code
uvec2 m; //mouse cords

struct termios oldTermFlags;

void ElafriInit() { //Init procedures
    tcgetattr(0, &oldTermFlags); //backup flags
    struct termios raw = oldTermFlags;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG); //turn off some flags
    tcsetattr(0, TCSAFLUSH, &raw);

    write(0,"\e[?1000h\e[?1003h\e[?25l",22); //enable mouse tracking and hide term cursor
    GrInit();
}

void Exit() {
    write(0,"\e[?1000l\e[?1003l\e[?25h\e[2J\e[H",40);
    tcsetattr(0, TCSAFLUSH, &oldTermFlags); //reset terminal flags
    exit(0);
}

void Input(struct timeval *mtv) { //read stdin and sort kbd and mouse input
    kbdSize=0;
    fd_set fdread;
    FD_ZERO(&fdread); //clear select set
    FD_SET(0,&fdread); //add fd 0 (stdin)
    if (select(1,&fdread,NULL,NULL,mtv)>0) { //check if stdin has data
        const u32 insz = read(0,input,256); //256 bytes should be enough
        for (u32 i=0; i<insz; i++) {
            if (input[i]==3) Exit(); //ctrl + C
            if (input[i]==27) { //ESC
                kbd[kbdSize] = 252; //prepare just in case
                if (input[i+1]==79) {//F1-4 are ordered as 0-3
                    kbd[kbdSize+1] = input[i+2]-80;
                    kbdSize++;
                    i+=2;
                } else
                    if (input[i+1]==91) {
                        kbdSize++;
                        switch (input[i+2]) {
                            case 77: //Mouse Input
                                kbdSize-=2;
                                mC = input[i+3];
                                m = (uvec2){input[i+4]-33, input[i+5]-33};
                                i+=5;
                                break;
                            case 49: //F5-8 are ordered as 4-7
                                kbd[kbdSize] = input[i+3]-49-(input[i+3]>53);
                            case 50:
                                if (input[i+3]!=126) {
                                    i+=4;
                                    break;
                                }
                            default:
                                kbd[kbdSize] = input[i+2];
                                i+=3-(input[i+2]>64);
                        }
                    } else kbd[kbdSize] = input[i]; //1 byte
            } else kbd[kbdSize] = input[i]; //1 byte
            kbdSize++;
        }
    }
}
#endif
