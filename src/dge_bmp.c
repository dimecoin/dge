#include "dge_bmp.h"

void fskip(FILE * fp, int num_bytes) {
	int i;
	for (i = 0; i < num_bytes; i++)
		fgetc(fp);
}

void load_bmp(char *file, BITMAP * b) {

	FILE *fp;
	long index;
	word num_colors;
	int x;

	/* open the file */
	if ((fp = fopen(file, "rb")) == NULL) {
		printf("Error opening file %s.\n", file);
		exit(1);
	}

	/* check to see if it is a valid bitmap file */
	if (fgetc(fp) != 'B' || fgetc(fp) != 'M') {
		fclose(fp);
		printf("%s is not a bitmap file.\n", file);
		exit(1);
	}

	/* read in the width and height of the image, and the
	 *      number of colors used; ignore the rest */
	fskip(fp, 16);
	fread(&b->width, sizeof(word), 1, fp);
	fskip(fp, 2);
	fread(&b->height, sizeof(word), 1, fp);
	fskip(fp, 22);
	fread(&num_colors, sizeof(word), 1, fp);
	fskip(fp, 6);

	/* assume we are working with an 8-bit file */
	if (num_colors == 0)
		num_colors = 256;

	/* try to allocate memory */
	if ((b->data = (byte *) malloc((word) (b->width * b->height))) == NULL) {
		fclose(fp);
		printf("Error allocating memory for file %s.\n", file);
		exit(1);
	}

	/* read the palette information */
	for (index = 0; index < num_colors; index++) {
		b->palette[(int)(index * 3 + 2)] = fgetc(fp) >> 2;
		b->palette[(int)(index * 3 + 1)] = fgetc(fp) >> 2;
		b->palette[(int)(index * 3 + 0)] = fgetc(fp) >> 2;
		x = fgetc(fp);
	}

	/* read the bitmap */
	for (index = (b->height - 1) * b->width; index >= 0; index -= b->width)
		for (x = 0; x < b->width; x++)
			b->data[(word) index + x] = (byte) fgetc(fp);

	fclose(fp);
}

void draw_bitmap(BITMAP * bmp, int x, int y) {

	int j;

	word screen_offset = (y << 8) + (y << 6) + x;
	word bitmap_offset = 0;

	for (j = 0; j < bmp->height; j++) {

		memcpy(&screen[screen_offset], &bmp->data[bitmap_offset], bmp->width);

		bitmap_offset += bmp->width;
		screen_offset += screen_width;

	}
}

void draw_transparent_bitmap(BITMAP * bmp, int x, int y) {
	int i, j;
	word screen_offset = (y << 8) + (y << 6);
	word bitmap_offset = 0;
	byte data;

	for (j = 0; j < bmp->height; j++) {
		for (i = 0; i < bmp->width; i++, bitmap_offset++) {
			data = bmp->data[bitmap_offset];
			if (data)
				screen[screen_offset + x + i] = data;
		}
		screen_offset += screen_width;
	}
}
