#ifndef _ELAFRI
#define _ELAFRI
#include <unistd.h>
#include <termios.h> //for term flags
#include <sys/ioctl.h>
#include "evar.c"
#include "graphics.c"
//#include "pkg.c"

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

u8 kbd[64]; //parsed keyboard input
u32 mC; //mouse code
ivec2 m; //mouse cords
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
	GrInit(res.x*res.y*20);
}

void Exit() {
	write(0,"\e[?1000l\e[?1003l\e[?25h\e[2J\e[H",30);
	tcsetattr(0, TCSAFLUSH, &oldTermFlags); //reset terminal flags
	exit(0);
}

F32 FramerateHandler(u32 max_fps) { //handles a framerate limit and returns delta time
	static InitTimer(ftimer); //timer
	int exec_time, wait_time;
	static u32 fps; //last frame's fps
	fps ^= (fps==0); //safeguards division by 0

	StopTimer(ftimer);

	exec_time = mod32(dt_usec(ftimer), 1000000); //Compute time it took for execution
	wait_time = 1000000 / fps - exec_time; //Check if behind/ahead
	fps = max(min(1000000*fps / (1000000-wait_time*fps), max_fps), 1); //Compute new fps

	ftimer_tend.tv_usec = max(1000000 / max_fps - exec_time, 0);
	ftimer_tend.tv_sec=0;
	select(1, NULL, NULL, NULL, &ftimer_tend); //sleep if over max fps

	StartTimer(ftimer);
	return 1. / fps;
}

u32 Input() { //read stdin and sort keyboard and mouse input (CTRL + C quits)
	u8 input[256]; //we read stdin to here
	u32 kbdSize=0;
	struct timeval mtv = {0,1}; //max sleep time
	fd_set fdread;
	FD_ZERO(&fdread); //clear select set
	FD_SET(0,&fdread); //add fd 0 (stdin)
	if (select(1,&fdread,NULL,NULL,&mtv)>0) { //check if stdin has data
		const u32 insz = read(0,input,256); //256 bytes should be enough
		for (u32 i=0; i<insz; i++) {
			if (input[i]==3) Exit(); //ctrl + C (remove this to handle exits yourself)
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
								m = (ivec2){input[i+4]-33, input[i+5]-33};
								i+=5;
								break;
							case 49: //F5-8 are ordered as 4-7
								kbd[kbdSize] = input[i+3]-49-(input[i+3]>53);
							case 50:
								if (input[i+3]!=126) {
									if (input[i+4]==126) //F9-12 are ordered as 8-11
										kbd[kbdSize] = input[i+3]-40-(input[i+3]>50);
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
	return kbdSize;
}
#endif
