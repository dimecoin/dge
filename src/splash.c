#include "dge.h"
#include "dge_gfx.h"
#include "dge_bmp.h"

void render();

BITMAP bmp_logo;
BITMAP bmp_rocket;


int rocketx, rockety;
byte bgcolor = 0;

void main(int argc, char *argv[]) {

	int i, x, y;

	load_bmp("res/images/dge_lbig.bmp", &bmp_logo);
	load_bmp("res/images/rocket.bmp", &bmp_rocket);

//	dge_init(BIOS, 320, 200);
//	dge_init(MEMMAP, 320, 200);
	dge_init(DOUBLEBUFF, 320, 200);

	set_palette(bmp_logo.palette);

	rocketx=(screen_width/2)-(bmp_rocket.width/2);
	rockety=(screen_height/2)-(bmp_rocket.height/2);


	while (true) {
		render();
		if (getchar()) {
			dge_shutdown();
			exit(1);
		}
	}

	dge_shutdown();

}

void render() {

	graphics_begin();

	clear_screen(bgcolor);
	//bgcolor++;

	draw_transparent_bitmap(&bmp_logo, 0, 0);

	draw_transparent_bitmap(&bmp_rocket, rocketx, rockety);
	
	graphics_end();

}
