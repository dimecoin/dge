#ifndef DGE_GRAPHICS_H
#define DGE_GRAPHICS_H

#include "dge.h"

#ifdef __WATCOMC__
#include <i86.h>
#endif

#define VIDEO_INT           0x10	/* the BIOS video interrupt. */
#define WRITE_DOT           0x0C	/* BIOS func to plot a pixel. */
#define SET_MODE            0x00	/* BIOS func to set the video mode. */

#define VGA_256_COLOR_MODE  0x13	/* use to set 256-color mode. */

#define TEXT_MODE           0x03	/* use to set 80x25 text mode. */

// double buffer
#define INPUT_STATUS_1      0x03da

#define DISPLAY_ENABLE      0x01	/* VGA input status bits */
#define VRETRACE            0x08
#define VERTICAL_RETRACE	/*comment out this line for more accurate timing */

extern int screen_width, screen_height, screen_size;
extern int num_colors;
extern enum RENDER_MODE render_mode;

extern byte *VGA;

extern byte *double_buffer;

// You should write to screen, it's either VGA memmap OR the buffer, depending on rendering mode
extern byte *screen;

extern fixed16_16 SIN_ACOS[1024];

void dge_graphics_init(enum RENDER_MODE mode, int width, int height);

void dge_graphics_shutdown();

void graphics_begin();
void graphics_end();

void set_mode(byte mode);

void draw_pixel(int x, int y, byte color);
void draw_line(int x1, int y1, int x2, int y2, byte color);

void draw_rect(int left, int top, int right, int bottom, byte color);
void fill_rect(int left, int top, int right, int bottom, byte color);

void draw_polygon(int num_vertices, int *vertices, byte color);
void fill_polygon(int num_vertices, int *vertices, byte color);

void draw_circle(int x, int y, int radius, byte color);
void fill_circle(int x, int y, int radius, byte color);

void show_buffer(byte * buffer);

void clear_screen();

#endif
