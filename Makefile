# This is Makefile for building OpenWatcom and DJGPP executables.

#####################################################
# This needs openwatcom installed.
# http://openwatcom.org/

# Just setting WATCOM should work in most cases.
export WATCOM:=${HOME}/opt/openwatcom/
export EDPATH:=${WATCOM}/eddat
export WIPFC:=${WATCOM}/wipfc
export INCLUDE:=${WATCOM}/h
export LIB:=${WATCOM}/lib386/dos/:${WATCOM}/lib386:.

#####################################################
# This is needs djgpp installed.
# https://github.com/andrewwutw/build-djgpp

# Just setting DJGPP_PREFIX should work in most cases.
export DJGPP_PREFIX=${HOME}/opt/djgpp
export BASE_DIR=${DJGPP_PREFIX}


export PATH:=${WATCOM}/binl:${DJGPP_PREFIX}/bin:${PATH}


#####################################################
# Envoriment setup

SRC=src

default: clean setup tests

setup:
	mkdir -p build/ow
	mkdir -p build/dj


tests:
	wcl386 -l=dos4g -lr -4 -ot -oi -lr $(SRC)/test_gfx.c $(SRC)/*.c
	# $(SRC)/dge.c $(SRC)/dge_graphics.c
	cp *.exe build/ow
	ls -l build/ow/*.exe

	i586-pc-msdosdjgpp-gcc $(SRC)/*.c -o build/dj/test_gfx.exe
	ls -l build/dj/*.exe

clean:
	rm *.o *.err *.exe *~ build/ow/* build/dj/* 2> /dev/null || true
	rm $(SRC)/*.err $(SRC)/*~ 2> /dev/null || true
