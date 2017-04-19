#include "dge.h"
#include "dge_snd.h"

void pc_sound(int note) {
	sound(note);
}

void stop_all_sounds() {
	nosound();
}
