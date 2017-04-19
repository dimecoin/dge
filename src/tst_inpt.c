#include "dge.h"
#include "libkb/kb.h"

#define KB_DEBUG 1

int main() {

	printf("Starting...\n");

	kb_install(0);

	while (1) {

		kb_update();

		int i = 0;


		if (kb_keys_pressed() > 0) {

			printf("%d keys now pressed: ", kb_keys_pressed());
			for (i = 0; i < 128; i++) {
				if (kb_key(i)) {

					char c = kb_keycode(i);

  					/* ignore shift state */
					c &= 0x7f;

					printf("Pressed: %c\n", c);
				}
			}
		}

		if (kb_key(KB_SCAN_W)) {
			char k = 'W';
			printf("%c\n", k);

		}



	}

	kb_remove();
	return 0;
}
