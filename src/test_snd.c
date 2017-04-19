#include "dge.h"
#include "dge_gfx.h"
#include "dge_snd.h"

int note = 0;
bool direction = true;
void test_song();
void random_song();
void up_song();

void main(int argc, char *argv[]) {


	enum RENDER_MODE mode=DOUBLEBUFF;

	dge_init(mode, 320, 200);
	

	while (true) {
		graphics_begin();

		//testt_song();
		//random_song();

		up_song();

		graphics_end();
	}

	sleep(1);
	dge_shutdown();

}


void test_song() {

	int i=0; 
	int j=0;

	for (j=0; i<2; j++) {

		for (i=0; i<8; i++) {

			pc_sound(20);
			delay(25);
//			nosound();

			delay(50);
			pc_sound(note);
			delay(50);
			note++;
		}

		pc_sound(20);
		delay(200);
		//nosound();
		delay(100);
		pc_sound(20);
		delay(200);

		pc_sound(note);
		delay(50);
		note++;
	}

	pc_sound(note%500);
	delay(100);

	pc_sound(2500);
	delay(100);
	//nosound();

	pc_sound(note);
	delay(50);

	note+=20;
	if(note>4000) {
		note=0;
	}

}

void random_song() {
	pc_sound(rand()%4000);
}


void up_song() {

	char text[50];

	if (note >= 500) {
		direction = false;
	} 
	if (note <= 0) {
		direction = true;
	}

	if (direction) {
		note++;
		print_text(8, 8, PFC_LBLUE, "UP UP UP");
	} else {
		note--;
		print_text(12, 12, PFC_BLUE, "DROP THE BASS");
	}

	sprintf(text, "Direction : %d NOTE: %d", direction, note);
	print_text(10, 10, PFC_PURPLE, text);


	pc_sound(note);
}
