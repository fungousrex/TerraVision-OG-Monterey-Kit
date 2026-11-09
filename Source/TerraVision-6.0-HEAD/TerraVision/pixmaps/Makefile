#
# Makefile to build all of the inline pixmaps for TerraVision.
# These include all of the frames of the rotating globe animation
# as well as the web icon images.
#
# $Id: Makefile,v 1.2 1999/04/16 00:06:58 leei Exp $
#

CFLAGS = -g

PPM_IMAGES = \
	earth1.ppm \
	earth2.ppm \
	earth3.ppm \
	earth4.ppm \
	earth5.ppm \
	earth6.ppm \
	earth7.ppm \
	earth8.ppm \
	earth9.ppm \
	earth10.ppm \
	earth11.ppm \
	earth12.ppm \
	earth13.ppm \
	earth14.ppm \
	earth15.ppm \
	tv.ppm \
	magic.ppm \
	sri.ppm \
	darpa.ppm \
	icon.ppm

RAW_IMAGES = $(PPM_IMAGES:.ppm=.h)

all: $(RAW_IMAGES)

ppm2c: ppm2c.c
	$(CC) $(CFLAGS) ppm2c.c -o ppm2c

1.h: earth1.ppm ppm2c
	ppm2c earth1.ppm earth1.h -rgb 

2.h: earth2.ppm ppm2c
	ppm2c earth2.ppm earth2.h -rgb 

3.h: earth3.ppm ppm2c
	ppm2c earth3.ppm earth3.h -rgb 

4.h: earth4.ppm ppm2c
	ppm2c earth4.ppm earth4.h -rgb 

5.h: earth5.ppm ppm2c
	ppm2c earth5.ppm earth5.h -rgb 

6.h: earth6.ppm ppm2c
	ppm2c earth6.ppm earth6.h -rgb 

7.h: earth7.ppm ppm2c
	ppm2c earth7.ppm earth7.h -rgb 

8.h: earth8.ppm ppm2c
	ppm2c earth8.ppm earth8.h -rgb 

9.h: earth9.ppm ppm2c
	ppm2c earth9.ppm earth9.h -rgb 

10.h: earth10.ppm ppm2c
	ppm2c earth10.ppm earth10.h -rgb 

11.h: earth11.ppm ppm2c
	ppm2c earth11.ppm earth11.h -rgb 

12.h: earth12.ppm ppm2c
	ppm2c earth12.ppm earth12.h -rgb 

13.h: earth13.ppm ppm2c
	ppm2c earth13.ppm earth13.h -rgb 

14.h: earth14.ppm ppm2c
	ppm2c earth14.ppm earth14.h -rgb 

15.h: earth15.ppm ppm2c
	ppm2c earth15.ppm earth15.h -rgb 

tv.h: tv.ppm ppm2c
	ppm2c tv.ppm tv.h 

magic.h: magic.ppm ppm2c
	ppm2c magic.ppm magic.h 

sri.h: sri.ppm ppm2c
	ppm2c sri.ppm sri.h 

darpa.h: darpa.ppm ppm2c
	ppm2c darpa.ppm darpa.h 

icon.h: icon.ppm ppm2c
	ppm2c icon.ppm icon.h 

