#include <stdio.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "evar.h"

#define SND_DEVICE "default"
#define SND_BUFFER_SIZE 8192
#define MAX_SND_BUFFERS 32

int main() { //test function
    snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_t *playback_handle;
	snd_pcm_sframes_t frames_to_write;
	int rate = 44100, curframe = 0;
	F32 snd_buf[SND_BUFFER_SIZE];

	if (snd_pcm_open(&playback_handle, SND_DEVICE, SND_PCM_STREAM_PLAYBACK, 0) < 0)
		printf("cannot open sound device");
	snd_pcm_hw_params_malloc(&hw_params);
	snd_pcm_hw_params_any(playback_handle, hw_params);
	snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_FLOAT);
	snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0);
	snd_pcm_hw_params_set_channels(playback_handle, hw_params, 1);
	if (snd_pcm_hw_params(playback_handle, hw_params) < 0)
		printf("cannot set parameters\n");
	snd_pcm_hw_params_free(hw_params);

	//tell ALSA to wake us up whenever 4096+ frames of playback data can be written
	snd_pcm_sw_params_malloc(&sw_params);
	if (snd_pcm_sw_params_current(playback_handle, sw_params) < 0)
		printf("software parameters failed initializing");
	snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, SND_BUFFER_SIZE);
	snd_pcm_sw_params_set_start_threshold(playback_handle, sw_params, 0);
	snd_pcm_sw_params(playback_handle, sw_params);

	if (snd_pcm_prepare(playback_handle) < 0) //final prepare
		printf("cannot prepare audio interface for use\n");

	while (1) {
		//wait till the interface is ready for data, or 1 second has elapsed
		snd_pcm_wait(playback_handle, 1000);

		//find out how much space is available for playback data
		frames_to_write = snd_pcm_avail_update(playback_handle);
		frames_to_write = min(frames_to_write, SND_BUFFER_SIZE); //cap it

		for (int i=0; i<frames_to_write; i++) {
			snd_buf[curframe%SND_BUFFER_SIZE] = 0.2*sinf(curframe*(1./rate)*2*pi*440);
			curframe++;
		}
		snd_pcm_writei(playback_handle, snd_buf, frames_to_write);
	}
	snd_pcm_close(playback_handle);
}
