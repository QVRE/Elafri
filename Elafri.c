#ifndef _ELAFRI
#define _ELAFRI

#include <unistd.h>
#include <termios.h> //for term flags
#include <sys/ioctl.h>
#include "graphics.c"
#include "pkg.c"

#define INSERT 50 //in kbd[], this would be 255 50
#define DELETE 51
#define PgUP 53
#define PgDown 54
#define UP 65
#define DOWN 66
#define RIGHT 67
#define LEFT 68
#define END 70
#define HOME 72

u8 input[256]; //we read stdin to here
u8 kbd[64]; //parsed keyboard input
u32 kbdSize, mC; //size of input buffer & mouse code
uvec2 m; //mouse cords
uvec2 res; //term resolution

struct termios oldTermFlags;

void GetResolution() {
    struct winsize r;
    ioctl(0, TIOCGWINSZ, &r);
    res.x = r.ws_col;
    res.y = r.ws_row-1;
}

void ElafriInit() { //Init procedures
    tcgetattr(0, &oldTermFlags); //backup flags
    struct termios raw = oldTermFlags;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG); //turn off some flags
    tcsetattr(0, TCSAFLUSH, &raw);

    write(0,"\e[?1000h\e[?1003h\e[?25l",22); //enable mouse tracking and hide term cursor
    GetResolution();
    GrInit();
}

void Exit() {
    write(0,"\e[?1000l\e[?1003l\e[?25h\e[2J\e[H",30);
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
                kbd[kbdSize] = 255; //prepare just in case
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
