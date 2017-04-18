#include "dge.h"
#include "dge_gfx.h"

void runAllTests(enum RENDER_MODE mode);
void screenBreaker(enum RENDER_MODE mode);
void runPixelTest(enum RENDER_MODE mode);
void runLineTest(enum RENDER_MODE mode);
void runRectTest(enum RENDER_MODE mode);
void runRectFillTest(enum RENDER_MODE mode);
void runPolyTest(enum RENDER_MODE mode, bool fill);
void runCircleTest(enum RENDER_MODE mode, bool fill);

float sleep_time = 0;
long object_count = 100L;

void main(int argc, char *argv[]) {

	enum RENDER_MODE mode;
	int count = 0;
	int i = 30;
	int y = 0;

	mode = DOUBLEBUFF;
	//mode = MEMMAP;

	continuous_rendering = true;
	vsync = true;

	dge_init(mode, 320, 200);

	srand(*my_clock);

	while (1) {
		runAllTests(mode);
	}

	dge_shutdown();

	return;
}

void runAllTests(enum RENDER_MODE mode) {

	runPixelTest(mode);
	sleep(sleep_time);

	runLineTest(mode);
	sleep(sleep_time);

	runRectTest(mode);
	sleep(sleep_time);

	runRectFillTest(mode);
	sleep(sleep_time);

	runPolyTest(mode, false);
	sleep(sleep_time);

	runPolyTest(mode, true);
	sleep(sleep_time);

	runCircleTest(mode, false);
	sleep(sleep_time);

	runCircleTest(mode, true);
	sleep(sleep_time);

	screenBreaker(mode);
	sleep(sleep_time);

}

void screenBreaker(enum RENDER_MODE mode) {

	//dge_init(mode, 320, 200);
	graphics_begin();

	draw_pixel(-1, -1, 128);
	draw_pixel(screen_width + 1, screen_height + 1, 128);

	draw_line(-1, -1, -1, -1, 128);
	draw_line(0, 0, screen_width + 1, screen_height + 1, 128);

	draw_circle(-1, -1, 100, 128);
	fill_circle(-1, -1, 100, 128);


	graphics_end();
}

void runPixelTest(enum RENDER_MODE mode) {

	int x, y, color;
	float time;
	word i, start;

	//dge_init(mode, 320, 200);
	graphics_begin();

	srand(*my_clock);
	start = *my_clock;

	for (i = 0; i < object_count; i++) {
		x = rand() % screen_width;
		y = rand() % screen_height;
		color = rand() % num_colors;
		draw_pixel(x, y, color);
	}

	graphics_end();

	time = (*my_clock - start) / 18.2;

}

void runLineTest(enum RENDER_MODE mode) {

	int x1, y1, x2, y2, color;
	float time;
	word i, start;

	//dge_init(mode, 320, 200);
	graphics_begin();

	srand(*my_clock);
	start = *my_clock;
	for (i = 0; i < object_count; i++) {

		x1 = rand() % screen_width;
		y1 = rand() % screen_height;
		x2 = rand() % screen_width;
		y2 = rand() % screen_height;
		color = rand() % num_colors;
		draw_line(x1, y1, x2, y2, color);
	}

	graphics_end();

	time = (*my_clock - start) / 18.2;
}

void runRectTest(enum RENDER_MODE mode) {

	int x1, y1, x2, y2, color;
	float time;
	word i, start;

	//dge_init(mode, 320, 200);
	graphics_begin();

	srand(*my_clock);
	start = *my_clock;

	for (i = 0; i < object_count; i++) {

		x1 = rand() % screen_width;
		y1 = rand() % screen_height;
		x2 = rand() % screen_width;
		y2 = rand() % screen_height;
		color = rand() % num_colors;

		draw_rect(x1, y1, x2, y2, color);
	}

	graphics_end();

	time = (*my_clock - start) / 18.2;

}

void runRectFillTest(enum RENDER_MODE mode) {

	int x1, y1, x2, y2, color;
	float time;
	word i, start;

	//dge_init(mode, 320, 200);
	graphics_begin();

	srand(*my_clock);
	start = *my_clock;

	for (i = 0; i < object_count; i++) {

		x1 = rand() % screen_width;
		y1 = rand() % screen_height;
		x2 = rand() % screen_width;
		y2 = rand() % screen_height;
		color = rand() % num_colors;

		fill_rect(x1, y1, x2, y2, color);
	}

	graphics_end();

	time = (*my_clock - start) / 18.2;
}

void runPolyTest(enum RENDER_MODE mode, bool fill) {

	int x1, y1, x2, y2, x3, y3, color;
	float time;
	word i, start;
	int vertices[6];

	//dge_init(mode, 320, 200);
	graphics_begin();

	srand(*my_clock);
	start = *my_clock;

	for (i = 0; i < object_count; i++) {

		vertices[0] = rand() % screen_width;
		vertices[1] = rand() % screen_height;

		vertices[2] = rand() % screen_width;
		vertices[3] = rand() % screen_height;

		vertices[4] = rand() % screen_width;
		vertices[5] = rand() % screen_height;

		color = rand() % num_colors;

		if (fill) {
			draw_polygon(3, vertices, color);
		} else {
			fill_polygon(3, vertices, color);
		}

	}

	graphics_end();

	time = (*my_clock - start) / 18.2;
}

void runCircleTest(enum RENDER_MODE mode, bool fill) {

	int x, y, radius, color;
	float time;
	word i, start;

	//dge_init(mode, 320, 200);
	graphics_begin();

	srand(*my_clock);
	start = *my_clock;

	for (i = 0; i < object_count; i++) {
		radius = rand() % 90 + 1;
		x = rand() % (screen_width - radius * 2) + radius;
		y = rand() % (screen_height - radius * 2) + radius;
		color = rand() % num_colors;
		if (fill) {
			fill_circle(x, y, radius, color);
		} else {
			draw_circle(x, y, radius, color);
		}
	}

	graphics_end();

	time = (*my_clock - start) / 18.2;

}
