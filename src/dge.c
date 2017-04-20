#include "dge.h"
#include "dge_gfx.h"
#include "dge_snd.h"
#include "dge_inpt.h"

#ifdef __WATCOMC__
int round(float num) {
	return num < 0 ? num - 0.5 : num + 0.5;
}
#endif


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

	srand(*my_clock);

	dge_graphics_init(mode, width, height);

	dge_input_init();

	init_once = true;

}

void dge_shutdown() {

#ifdef __DJGPP__
	__djgpp_nearptr_disable();
#endif

	stop_all_sounds();

	dge_input_shutdown();

	dge_graphics_shutdown();

	exit(0);
}


