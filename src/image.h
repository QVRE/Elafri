#pragma once
#include "graphics.h"
#include "evar.h"

//downsamples an image by applying the new image's grid onto the old image and getting the
//average values of the sums of the old pixels inside every grid block
gr Downsample(gr *buf, u32 new_w, u32 new_h);

// Portable Pixel Map (PPM) | Does not store Alpha values | ffmpeg supports conversion for this
// http://davis.lbl.gov/Manuals/NETPBM/doc/ppm.html

gr ReadPPM(char *filename);

void WritePPM(char *filename, gr *buf);

// Quite OK Image (QOI) | Not efficient with Alpha values
// https://qoiformat.org/qoi-specification.pdf

gr ReadQOI(char *filename);

void WriteQOI(char *filename, gr *buf);
