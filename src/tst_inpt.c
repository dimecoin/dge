#include "dge.h"
#include "dge_gfx.h"
#include "dge_snd.h"
#include "dge_inpt.h"

#include "libkb/kb.h"


#define KEY_UP KB_SCAN_W
#define KEY_DOWN KB_SCAN_S
#define KEY_LEFT KB_SCAN_A
#define KEY_RIGHT KB_SCAN_D

int main() {

	enum RENDER_MODE mode = DOUBLEBUFF;
	float x, y;
	int ix, iy;
	int i=0;
	int ii=0;
	int xs[100];
	int ys[100];
	char key;
	int value=0;

	// pixels per second
	int speed = 100; 
	int half_size = 20;
	byte color = 0;

	char coords_text[50];

	for (i=0; i<100; i++) {
		xs[i] = 0;
		ys[i] = 0;
	}

        continuous_rendering = false;
        vsync = true;
        show_fps=true;

        dge_init(mode, 320, 200);

       	x = screen_width/2;
	y = screen_height/2;

		


	while (1) {

		kb_update();
		if (kb_kbhit()) {
			key = kb_getkey();

			if (key == 'i') {
				value++;
			}
			if (key == 'k') {
				value--;
			}

		}

		char key = kb_keycode(kb_last_key());
		//printf("C:%c", key);
		fflush(NULL);


		if (key_pressed(KEY_UP)) {
			y = y + -(delta * speed);
		}
		if (key_pressed (KEY_DOWN)) {
			y = y + (delta * speed);
		}

		if (key_pressed(KEY_LEFT)) {
			x = x + -(delta * speed);
		}

		if (key_pressed(KEY_RIGHT)) {
			x = x + (delta * speed);
		}

		x = MAX(0+half_size, x);
		x = MIN(screen_width-half_size, x);

		y = MAX(0+half_size, y);
		y = MIN(screen_height-half_size, y);

		ix=round(x);
		iy=round(y);

		graphics_begin();

		xs[ii] = ix;
		ys[ii] = iy;
		for (i=0;i<100; i++) {
			if (xs[i] != 0) {
				/*
				draw_rect(xs[i]-half_size, ys[i]-half_size, xs[i]+half_size-1, ys[i]+half_size-1, color);
				draw_line(xs[i], ys[i], ix, iy, color+1);
				draw_line(xs[i]-half_size, ys[i]-half_size, ix, iy, color+9);
				draw_line(xs[i]+half_size, ys[i]+half_size, ix, iy, color+9);
				draw_line(xs[i], ys[i]+half_size, ix, iy, color+9);
				draw_line(xs[i]+half_size, ys[i], ix, iy, color+9);
				draw_line(xs[i], ys[i]-half_size, ix, iy, color+9);
				draw_line(xs[i]-half_size, ys[i], ix, iy, color+9);
				*/
			}
		}

		print_text(1,4, PFC_GREEN, BUILD_TYPE);

		sprintf(coords_text, "[%.2fx%.2f]", x, y);
		print_text(1,5, PFC_GREEN, coords_text);

		sprintf(coords_text, "[%dx%d]", ix, iy);
		print_text(1,6, PFC_RED, coords_text);

		fill_rect(ix-half_size, iy-half_size, ix+half_size-1, iy+half_size-1, color);

		draw_rect(ix-half_size, iy-half_size, ix+half_size-1, iy+half_size-1, 2);

		draw_line(0,0, ix, iy, color);
		draw_line(screen_width, 0, ix, iy, color);
		draw_line(screen_width, screen_height, ix, iy, color);
		draw_line(0, screen_height, ix, iy, color);

		graphics_end();

		sprintf(coords_text, "Value: %d", value);
		print_text(2,10, PFC_WHITE, coords_text);


		color++;
		ii++;
		if (ii>99) {
			ii=0;
		}


	}


        dge_shutdown();
	return 0;
}
