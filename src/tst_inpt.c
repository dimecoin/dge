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
	// pixels per second
	int speed = 100; 
	int half_size = 20;
	byte color = 0;

	char coords_text[50];

        continuous_rendering = false;
        vsync = true;
        show_fps=true;

        dge_init(mode, 320, 200);

       	x = screen_width/2;
	y = screen_height/2;

/*
	while (1) {
		kb_update();
		if (kb_kbhit()) {
	 		char key = key = kb_getkey();
			printf("I:%c", key);
			fflush(NULL);
		}

		char key = kb_keycode(kb_last_key());
		//printf("C:%c", key);
		fflush(NULL);
		

		delay(100);

	}
*/

	while (1) {


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

		sprintf(coords_text, "[%.2fx%.2f]", x, y);
		print_text(1,5, PFC_GREEN, coords_text);

		sprintf(coords_text, "[%dx%d]", ix, iy);
		print_text(1,6, PFC_RED, coords_text);

		fill_rect(ix-half_size, iy-half_size, ix+half_size-1, iy+half_size-1, color);
		color++;

		draw_rect(ix-half_size, iy-half_size, ix+half_size-1, iy+half_size-1, 1);

		draw_line(0,0, ix, iy, color);
		color++;
		draw_line(screen_width, 0, ix, iy, color);
		color++;
		draw_line(screen_width, screen_height, ix, iy, color);
		color++;
		draw_line(0, screen_height, ix, iy, color);
		color++;


		graphics_end();
	}


        dge_shutdown();
	return 0;
}
