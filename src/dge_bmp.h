#ifndef DGE_BMP_H
#define DGE_BMP_H

#include "dge.h"
#include "dge_gfx.h"

typedef struct tagBITMAP {
	word width;
	word height;
	byte palette[256 * 3];
	byte *data;
} BITMAP;

void fskip(FILE * fp, int num_bytes);
void load_bmp(char *file, BITMAP * b);
void draw_bitmap(BITMAP * bmp, int x, int y);
void draw_transparent_bitmap(BITMAP * bmp, int x, int y);

#endif
