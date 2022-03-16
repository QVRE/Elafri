#include "Elafri.c"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define FPS 30
#define MPS 1000000 // s/μs

#define CAM_WD 160 //video width
#define CAM_HT 120 //height

int hashR, hashG, hashB;

void HashP(color **pal, int x, int y) {
	hashR=0, hashG=0, hashB=0;
	u32 dist; //pd: 4, ds: 4, es: 2, bs: 16
	color *cp;
	for (int i=-4; i<=4; i++)
		for (int j=-4; j<=4; j++) {
			cp = &pal[y+i][x+j];
			dist = (abs(i)+abs(j)) << 4;
			hashR += (((u32)cp->r>>2)<<16) >> dist;
			hashG += (((u32)cp->g>>2)<<16) >> dist;
			hashB += (((u32)cp->b>>2)<<16) >> dist;
		}
}

int main() {
    ElafriInit();

    gr Gr = GrBuffer(res.x,res.y,res.x/4,res.y/2);
    gr sGr = GrBuffer(CAM_WD,CAM_HT,8,8);

    /*Timekeeping-related Inits*/
    InitTimer(ftimer); //timer
    struct timeval mtv = {0,1};
    u32 fps = FPS;
    int exec_time, wt;
    F32 delta=0; //Δt per frame

	int fd = open("/dev/video0", O_RDWR, 0);

	struct v4l2_format fmt = {0};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = CAM_WD;
	fmt.fmt.pix.height = CAM_HT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
		printf("Error: Failed setting Pixel Format. If your camera doesn't support YUYV this may be why\n");

	struct v4l2_requestbuffers req = {0};
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
		printf("Error: Failed Requesting Buffer\n");

	struct v4l2_buffer buf = {0};
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;
	if(ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
		printf("Error: Failed to query Buffer\n");

	u8 *buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

	if (ioctl(fd, VIDIOC_STREAMON, &buf.type) == -1)
		printf("Error: Failed to start capturing\n");

	int y0,y1,u,v,tm,rdx,rdy; u32 ofs;
	color p0,p1;

	ivec2 p = {0}, bsp = {0};
	int phR, phG, phB, bestScore;

    RenderLoop:
        StartTimer(ftimer);

        Input(&mtv);
		if(ioctl(fd, VIDIOC_QBUF, &buf) == -1)
			printf("Error: Failed to get Buffer\n");
		if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
			printf("Error: capture failed\n");
		for (u32 i=0; i<CAM_HT; i++) //YUYV decoder into RGB, a bit messy
			for (u32 j=0; j<CAM_WD; j+=2) {
				ofs = (i*CAM_WD+j)*2;
				y0 = buffer[ofs], y1 = buffer[ofs+2];
				u = buffer[ofs+1]-128, v = buffer[ofs+3]-128;
				tm = y0 + 1.4065 * v; tm = min(tm,255);
				p0.r = max(tm,0);
				tm = y1 + 1.4065 * v; tm = min(tm,255);
				p1.r = max(tm,0);
				tm = y0 - 0.3455 * u - 0.7169 * v; tm = min(tm,255);
				p0.g = max(tm,0);
				tm = y1 - 0.3455 * u - 0.7169 * v; tm = min(tm,255);
				p1.g = max(tm,0);
				tm = y0 + 1.1790 * u; tm = min(tm,255);
				p0.b = max(tm,0);
				tm = y1 + 1.1790 * u; tm = min(tm,255);
				p1.b = max(tm,0);
				sGr.pal[i][j] = p0;
				sGr.pal[i][j+1] = p1;
				rdy = i/(F32)CAM_HT*res.y;
				rdx = j/(F32)CAM_WD*res.x;
				Gr.pal[rdy][rdx] = p0;
				rdx = (j+1)/(F32)CAM_WD*res.x;
				Gr.pal[rdy][rdx] = p1;
			}
		bestScore=2000000000; //set it high
		for (int i=4; i<CAM_WD-4; i++)
			for (int j=4; j<CAM_HT-4; j++) {
				HashP(sGr.pal, i, j);
				const int u = abs(phR-hashR) + abs(phG-hashG) + abs(phB-hashB);
				if (u < bestScore) {
					bestScore = u;
					bsp = (ivec2){roundf(i*(1./CAM_WD)*res.x),roundf(j*(1./CAM_HT)*res.y)};
				}
			}
        if (mC==32) {
			p = m;
			HashP(sGr.pal, roundf((F32)p.x/res.x*CAM_WD), roundf((F32)p.y/res.y*CAM_HT));
			phR = hashR, phG = hashG, phB = hashB;
			GrCircle(&Gr,p,3,ROT/4,YELLOW);
		}
		GrCircle(&Gr,bsp,4,ROT/8,GREEN);
        drawc(&Gr);

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
