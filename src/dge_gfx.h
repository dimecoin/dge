#ifndef DGE_GFX_H
#define DGE_GFX_H

#include "dge.h"

#ifdef __WATCOMC__
#include <i86.h>
#endif

#define VIDEO_INT           0x10	/* the BIOS video interrupt. */
#define WRITE_DOT           0x0C	/* BIOS func to plot a pixel. */
#define SET_MODE            0x00	/* BIOS func to set the video mode. */

#define VGA_256_COLOR_MODE  0x13	/* use to set 256-color mode. */

#define TEXT_MODE           0x03	/* use to set 80x25 text mode. */

#define INPUT_STATUS      0x03da
#define VRETRACE            0x08

// double buffer
#define DISPLAY_ENABLE      0x01	/* VGA input status bits */

// palette data
#define PALETTE_INDEX       0x03c8
#define PALETTE_DATA        0x03c9


// PFC = Primative Font Color.  L=Light
// These are used with print_text
#define PFC_RED 31
#define PFC_LIGHTGREEN 32
#define PFC_YELLOW 33
#define PFC_PURPLE 34
#define PFC_BOLDPINK 35
#define PFC_LBLUE 36
#define PFC_WHITE 37
#define PFC_GREEN 42
#define PFC_BRICKRED 43
#define PFC_LIME 45
#define PFC_PINK 46
#define PFC_BLUE 47

extern int screen_width, screen_height, screen_size;
extern int num_colors;
extern enum RENDER_MODE render_mode;

// A poor man's vsync.  This still causes screen tearing with BIOS or MEMMAP.
// TODO: could be done better?  See wait_for_sync
extern bool vsync;

// If true, then it won't clear screen each cycle.
// This can save cpu if background doesn't change much, but have to do manually.
extern bool continuous_rendering;

// Used as clear_screen's defaut
extern byte background_color;

// Our direct memory buffer
extern byte *VGA;

// Our double buffer
extern byte *double_buffer;

// You should write to screen, it's either VGA memmap OR the buffer, depending on rendering mode
extern byte *screen;

extern fixed16_16 SIN_ACOS[1024];

extern int fps_counter, fps_avg;
extern word clock_start_time;
extern float clock_cum_time, delta;



void dge_graphics_init(enum RENDER_MODE mode, int width, int height);
void set_palette(byte * palette);

void dge_graphics_shutdown();

void graphics_begin();
void graphics_end();
void wait_for_retrace(void);

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

// Used by print_text to position.
void set_cursor_pos(int x, int y);


/*
 * This is a very primative way to print text in graphics mode.
 * It's pretty ulgy, probably only for debugging.
 * x/y are in cursor position (like text mode), not pixel locations.
 * use PFC_X  for colors
*/
void print_text(int x, int y, int color, const char *string);



void clear_screen(byte color);

#endif
