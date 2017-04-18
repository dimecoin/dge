#include "dge.h"
#include "dge_gfx.h"

/* this points to the 18.2hz system clock */
#ifdef __WATCOMC__
word *my_clock = (word *) 0x0000046C;
#elif __DJGPP__
word *my_clock = (word *) 0x046C;
#else
// Borland C or Turbo C
word *my_clock = (word *) 0x0000046C;
#endif

bool init_once = false;

void dge_init(enum RENDER_MODE mode, int width, int height) {

#ifdef __DJGPP__

	if (!init_once) {

		if (__djgpp_nearptr_enable() == 0) {
			printf("Could get access to first 640K of memory.\n");
			exit(-1);
		}

		my_clock = (void *)my_clock + __djgpp_conventional_base;
	}
#endif

	dge_graphics_init(mode, width, height);

	init_once = true;

}

void dge_shutdown() {

#ifdef __DJGPP__
	__djgpp_nearptr_disable();
#endif

	dge_graphics_shutdown();

}
