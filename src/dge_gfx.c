#include "dge.h"
#include "dge_gfx.h"

int screen_width = -1;
int screen_height = -1;
int screen_size = -1;
int num_colors = -1;
enum RENDER_MODE render_mode = NONE;

/* this points to video memory. */
#ifdef __WATCOMC__
byte *VGA = (byte *) 0xA0000L;
#elif __DJGPP__
byte *VGA = (byte *) 0xA0000;
#else
	// Borland C or Turbo C
byte *VGA = (byte *) 0xA0000000L;
#endif

byte *double_buffer = NULL;

byte *screen;

fixed16_16 SIN_ACOS[1024];

bool vsync = true;

bool continuous_rendering = false;

byte background_color = 0;

bool show_fps = true;

int fps_counter, fps_avg;
word clock_start_time;
float clock_cum_time, delta;

void dge_graphics_init(enum RENDER_MODE mode, int width, int height) {

	screen_width = width;
	screen_height = height;
	screen_size = screen_width * screen_height;
	num_colors = 256;

	render_mode = mode;

	// Free our double buffer 
	if (mode != DOUBLEBUFF && double_buffer != NULL) {
		free(double_buffer);
		double_buffer = NULL;
	}

	if (!init_once) {

		// Cache our trig values for faster circle drawing.
		{
			int i;
			for (i = 0; i < 1024; i++) {
				SIN_ACOS[i] = sin(acos((float)i / 1024)) * 0x10000L;
			}
		}

#ifdef __DJGPP__
		VGA += __djgpp_conventional_base;
#endif

	}
	// Finally switch our mode.
	set_mode(VGA_256_COLOR_MODE);

	switch (mode) {
	case BIOS:
	case MEMMAP:
		screen = VGA;
		break;
	case DOUBLEBUFF:

		if (double_buffer == NULL) {
			if ((double_buffer = (byte *) malloc(screen_size)) == NULL) {
				printf("Not enough memory for double buffer.\n");
				exit(1);
			}
		}

		screen = double_buffer;
		break;
	}

}

void set_palette(byte * palette) {
	int i;

	outp(PALETTE_INDEX, 0);	/* tell the VGA that palette data is coming. */
	for (i = 0; i < 256 * 3; i++) {
		outp(PALETTE_DATA, palette[i]);	/* write the data */
	}
}

void dge_graphics_shutdown() {

	if (double_buffer != NULL) {
		free(double_buffer);
		double_buffer = NULL;
	}

	set_mode(TEXT_MODE);
}

void set_mode(byte mode) {
	union REGS regs;

	regs.h.ah = SET_MODE;
	regs.h.al = mode;

#ifdef __WATCOMC__
	int386(VIDEO_INT, &regs, &regs);
#else
	int86(VIDEO_INT, &regs, &regs);
#endif

}

void show_buffer(byte * buffer) {
	memcpy(VGA, buffer, screen_size);
}

void graphics_begin() {

	clock_start_time = *my_clock;

	if (vsync && render_mode != DOUBLEBUFF) {
		wait_for_retrace();
	}

	if (!continuous_rendering) {
		clear_screen(background_color);
	}

}

char fps_text[8];
char delta_text[13];
void graphics_end() {


	if (render_mode == DOUBLEBUFF) {

		if (vsync) {
			wait_for_retrace();
		}

		show_buffer(double_buffer);
	}

	delta = (*my_clock - clock_start_time) / CLOCK_SPEED;
	if (delta >= 100) {
		delta = 99.999;
	}

	if (show_fps) {

		sprintf(fps_text, "FPS: %d", fps_avg);
		print_text(1, 1, PFC_GREEN, fps_text);

		sprintf(delta_text, "delta: %.3f", delta);
		print_text(1, 2, PFC_LIME, delta_text);

		fps_counter++;
		clock_cum_time += delta;
		if (clock_cum_time >= 1.0f) {
			fps_avg = fps_counter;

			if (fps_avg > 999) {
				fps_avg = 999;
			}

			fps_counter = 0;
			clock_cum_time = 0;
		}
	}

}

void wait_for_retrace(void) {
	/* wait until done with vertical retrace */
	while ((inp(INPUT_STATUS) & VRETRACE)) ;
	/* wait until done refreshing */
	while (!(inp(INPUT_STATUS) & VRETRACE)) ;
}

void draw_pixel(int x, int y, byte color) {

	if (render_mode == BIOS) {
		union REGS regs;

		regs.h.ah = WRITE_DOT;
		regs.h.al = color;

#ifdef __WATCOMC__
		regs.w.cx = x;
		regs.w.dx = y;

		int386(VIDEO_INT, &regs, &regs);
#else
		regs.x.cx = x;
		regs.x.dx = y;

		int86(VIDEO_INT, &regs, &regs);
#endif

		return;
	}

	if (render_mode == MEMMAP || render_mode == DOUBLEBUFF) {
		screen[y * screen_width + x] = color;
		return;
	}

}

void draw_line(int x1, int y1, int x2, int y2, byte color) {

	int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

	dx = x2 - x1;		/* the horizontal distance of the line */
	dy = y2 - y1;		/* the vertical distance of the line */
	dxabs = abs(dx);
	dyabs = abs(dy);
	sdx = sgn(dx);
	sdy = sgn(dy);
	x = dyabs >> 1;
	y = dxabs >> 1;
	px = x1;
	py = y1;

	draw_pixel(px, py, color);

	if (dxabs >= dyabs) {	/* the line is more horizontal than vertical */
		for (i = 0; i < dxabs; i++) {
			y += dyabs;
			if (y >= dxabs) {
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			draw_pixel(px, py, color);
		}
	} else {		/* the line is more vertical than horizontal */

		for (i = 0; i < dyabs; i++) {
			x += dxabs;
			if (x >= dyabs) {
				x -= dyabs;
				px += sdx;
			}
			py += sdy;
			draw_pixel(px, py, color);
		}
	}
}

void draw_rect(int left, int top, int right, int bottom, byte color) {

	if (render_mode == BIOS) {

		draw_line(left, top, right, top, color);
		draw_line(left, top, left, bottom, color);
		draw_line(right, top, right, bottom, color);
		draw_line(left, bottom, right, bottom, color);

		return;
	}
	// Should be much faster        
	if (render_mode == MEMMAP || render_mode == DOUBLEBUFF) {

		word top_offset, bottom_offset, i, temp;

		if (top > bottom) {
			temp = top;
			top = bottom;
			bottom = temp;
		}
		if (left > right) {
			temp = left;
			left = right;
			right = temp;
		}

		top_offset = (top << 8) + (top << 6);
		bottom_offset = (bottom << 8) + (bottom << 6);

		for (i = left; i <= right; i++) {
			screen[top_offset + i] = color;
			screen[bottom_offset + i] = color;
		}
		for (i = top_offset; i <= bottom_offset; i += screen_width) {
			screen[left + i] = color;
			screen[right + i] = color;
		}

		return;
	}

}

void fill_rect(int left, int top, int right, int bottom, byte color) {

	word top_offset, bottom_offset, i, temp, width;

	if (top > bottom) {
		temp = top;
		top = bottom;
		bottom = temp;
	}

	if (left > right) {
		temp = left;
		left = right;
		right = temp;
	}

	if (render_mode == BIOS) {

		int x = left;
		int y = bottom;

		for (; x < right; x++) {
			// bottom and top are reversed.
			for (y = bottom; y > top; y--) {
				draw_pixel(x, y, color);
			}
		}

		return;
	}

	if (render_mode == MEMMAP || render_mode == DOUBLEBUFF) {

		top_offset = (top << 8) + (top << 6) + left;
		bottom_offset = (bottom << 8) + (bottom << 6) + left;
		width = right - left + 1;

		for (i = top_offset; i <= bottom_offset; i += screen_width) {
			memset(&screen[i], color, width);
		}

		return;
	}

}

// TODO: could be optimized? 
void draw_polygon(int num_vertices, int *vertices, byte color) {

	int i;

	for (i = 0; i < num_vertices - 1; i++) {
		draw_line(vertices[(i << 1) + 0], vertices[(i << 1) + 1],
			  vertices[(i << 1) + 2], vertices[(i << 1) + 3], color);
	}

	draw_line(vertices[0], vertices[1], vertices[(num_vertices << 1) - 2],
		  vertices[(num_vertices << 1) - 1], color);

}

// TODO: need to implement
void fill_polygon(int num_vertices, int *vertices, byte color) {
	draw_polygon(num_vertices, vertices, color);
}

void clear_screen(byte color) {

	if (render_mode == BIOS) {

		if (color == 0) {
			// This si a quick way to clear screen using default color.
			set_mode(VGA_256_COLOR_MODE);
			return;

		} else {
			int x, y;

			for (x = 0; x < screen_width; x++) {
				for (y = 0; y < screen_height; y++) {
					draw_pixel(x, y, color);
				}
			}
			return;
		}
	}

	if (render_mode == MEMMAP || render_mode == DOUBLEBUFF) {
		memset(screen, color, screen_size);
		return;
	}

}

// This is used internally to bound check our double_buffer, because you can crash if written outside of it.
// It is only really a problem for DJGPP builds with DOUBLEBUFF... 
void safe_screen_update(int value, byte color) {

	if (value < 0 || value >= screen_size) {
		return;
	}

	screen[value] = color;
}


// TODO: draw in BIOS mode, using draw_pixel instead
void draw_circle(int x, int y, int radius, byte color) {

	fixed16_16 n = 0, invradius = (1 / (float)radius) * 0x10000L;
	int dx = 0, dy = radius - 1;
	word dxoffset, dyoffset, offset = (y << 8) + (y << 6) + x;

	while (dx <= dy) {
		dxoffset = (dx << 8) + (dx << 6);
		dyoffset = (dy << 8) + (dy << 6);

		if (render_mode == DOUBLEBUFF) {

			safe_screen_update(offset + dy - dxoffset, color);
			safe_screen_update(offset + dx - dyoffset, color);	/* octant 1 */
			safe_screen_update(offset - dx - dyoffset, color);	/* octant 2 */
			safe_screen_update(offset - dy - dxoffset, color);	/* octant 3 */
			safe_screen_update(offset - dy + dxoffset, color);	/* octant 4 */
			safe_screen_update(offset - dx + dyoffset, color);	/* octant 5 */
			safe_screen_update(offset + dx + dyoffset, color);	/* octant 6 */
			safe_screen_update(offset + dy + dxoffset, color);	/* octant 7 */

		} else {
			screen[offset + dy - dxoffset] = color;	/* octant 0 */
			screen[offset + dx - dyoffset] = color;	/* octant 1 */
			screen[offset - dx - dyoffset] = color;	/* octant 2 */
			screen[offset - dy - dxoffset] = color;	/* octant 3 */
			screen[offset - dy + dxoffset] = color;	/* octant 4 */
			screen[offset - dx + dyoffset] = color;	/* octant 5 */
			screen[offset + dx + dyoffset] = color;	/* octant 6 */
			screen[offset + dy + dxoffset] = color;	/* octant 7 */
		}

		dx++;
		n += invradius;
		dy = (int)((radius * SIN_ACOS[(int)(n >> 6)]) >> 16);
	}

}

// TODO: draw in BIOS mode, using draw_pixel instead
void fill_circle(int x, int y, int radius, byte color) {

	fixed16_16 n = 0, invradius = (1 / (float)radius) * 0x10000L;
	int dx = 0, dy = radius - 1, i;
	word dxoffset, dyoffset, offset = (y << 8) + (y << 6) + x;

	while (dx <= dy) {

		dxoffset = (dx << 8) + (dx << 6);
		dyoffset = (dy << 8) + (dy << 6);

		for (i = dy; i >= dx; i--, dyoffset -= screen_width) {

			if (render_mode == DOUBLEBUFF) {

				safe_screen_update(offset + i - dxoffset, color);
				safe_screen_update(offset + dx - dyoffset, color);
				safe_screen_update(offset - dx - dyoffset, color);
				safe_screen_update(offset - i - dxoffset, color);
				safe_screen_update(offset - i + dxoffset, color);
				safe_screen_update(offset - dx + dyoffset, color);
				safe_screen_update(offset + dx + dyoffset, color);
				safe_screen_update(offset + i + dxoffset, color);

			} else {

				screen[offset + i - dxoffset] = color;
				screen[offset + dx - dyoffset] = color;
				screen[offset - dx - dyoffset] = color;
				screen[offset - i - dxoffset] = color;
				screen[offset - i + dxoffset] = color;
				screen[offset - dx + dyoffset] = color;
				screen[offset + dx + dyoffset] = color;
				screen[offset + i + dxoffset] = color;

			}

		}

		dx++;
		n += invradius;
		dy = (int)((radius * SIN_ACOS[(int)(n >> 6)]) >> 16);
	}
}

void set_cursor_pos(int x, int y) {

	union REGS iregs, oregs;

	iregs.h.ah = 0x0f;	/* get video page */

#ifdef __WATCOMC__
	int386(0x10, &iregs, &oregs);
#else
	int86(0x10, &iregs, &oregs);
#endif

	iregs.h.ah = 0x02;	/* set cursor pos */
	iregs.h.bh = oregs.h.bh;

	iregs.h.dl = (char)x - 1;
	iregs.h.dh = (char)y - 1;

#ifdef __WATCOMC__
	int386(0x10, &iregs, &oregs);
#else
	int86(0x10, &iregs, &oregs);
#endif
}

void print_text(int x, int y, int color, const char *string) {

	// gotoxy is not support under OpenWatcom.
	// gotoxy(x,y)

	set_cursor_pos(x, y);
	printf("\033[1;%dm%s\033[0m\n", color, string);
}
