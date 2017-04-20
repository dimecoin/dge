#include "dge.h"
#include "dge_inpt.h"

void dge_input_init() {
	kb_install(0);
}

bool key_pressed(int key) {
	kb_update();
return (kb_key(key));

}

void dge_input_shutdown() {
	kb_remove();
}
