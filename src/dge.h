#ifndef DGE_H
#define DGE_H

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>
#include <conio.h>
#include <math.h>
#include <string.h>

#if defined (__DJGPP__) || defined (__WATCOMC__)
#include <unistd.h>
#endif

#ifdef __DJGPP__
#include <sys/nearptr.h>
#endif

/* macro to return the sign of a number */
#define sgn(x) ((x<0)?-1:((x>0)?1:0))

/* macro to write a word to a port */
#define word_out(port,register,value) \
	  outpw(port,(((word)value<<8) + register))

typedef enum { false, true } bool;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef long fixed16_16;

enum RENDER_MODE {
	NONE,			// Not setup yet.
	TEXT,			// normal text mode
	BIOS,			// direct to bios, slow
	MEMMAP,			// memory map
	DOUBLEBUFF,		// memory map with double buffer
	MODEX			// unchained mode
};

/* this points to the 18.2hz system clock */
extern word *my_clock;
#define CLOCK_SPEED 18.2

extern bool init_once;

void dge_init(enum RENDER_MODE mode, int width, int height);

void dge_shutdown();

#endif
