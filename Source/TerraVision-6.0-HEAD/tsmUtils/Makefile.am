#  tsmUtils: External utilities using the tsm library
#
#  Copyright SRI International (1993-2000)
#
#  All rights reserved.
#

#
# Build the tsmUtils programs
#

bin_PROGRAMS = \
	simple \
	copy_dataset \
	get_tile \
	get_url \
	get_tspec \
	untile \
	log_info \
	time_tiles \
	calc_stats \
	database \
	tv2geo \
	make_oi \
	make_dem \
	make_geovrml \
	text2geovrml \
	checksum \
	utm_conv \
	calc_bbox \
	dem_to_pgm \
        html_database

#
# Create a library out of the TerraServer module
#

DEPS = @DEPS@

libterraserver.a: terraserver.o
	$(AR) cru libterraserver.a terraserver.o
	$(RANLIB) libterraserver.a

#
# Definitions for compiling Apache modules under IRIX
#

APACHE_DEFS = -DIRIX -DUSE_HSREGEX -DUSE_EXPAT -DSHARED_MODULE
APACHE_LIBS = -L. -lterraserver -ltsmApi -lgeotiff -lse_conv -lungif -lghttp \
              -lxml -lvrml2 -lpthrutil -ltiff -ljpeg -lz -lnspr4 -lm -lc

#
# explicit targets for compiling TerraVision Apache module
#

mod_terravision.so: libterraserver.a mod_terravision.o
	$(LD) -shared -o mod_terravision.so mod_terravision.o \
		$(LDFLAGS) $(APACHE_LIBS)

mod_terravision.o: mod_terravision.c
	$(COMPILE) -c -I$(srcdir)/apache/ $(DEFS) $(APACHE_DEFS) \
		$(srcdir)/mod_terravision.c

#
# explicit targets for compiling Web Mapping Server Apache module
#

mod_wms.so: libterraserver.a mod_wms.o
	$(LD) -shared -o mod_wms.so mod_wms.o \
		 $(LDFLAGS) $(APACHE_LIBS)

mod_wms.o: mod_wms.c
	$(COMPILE) -c -I$(srcdir)/apache/ $(DEFS) $(APACHE_DEFS) \
		$(srcdir)/mod_wms.c


# EXTRA_PROGRAMS = test get_http_tile

EXTRA_PROGRAMS = test_inmem test_inmem3 test_req tvlaunch test_wms

simple_DEPENDENCIES = $(DEPS)
copy_dataset_DEPENDENCIES = $(DEPS)
get_tile_DEPENDENCIES = $(DEPS)
get_url_DEPENDENCIES = $(DEPS)
get_tspec_DEPENDENCIES = $(DEPS)
untile_DEPENDENCIES = $(DEPS)
log_info_DEPENDENCIES = $(DEPS)
time_tiles_DEPENDENCIES = $(DEPS)
calc_stats_DEPENDENCIES = $(DEPS)
database_DEPENDENCIES = $(DEPS)
tv2geo_DEPENDENCIES = $(DEPS)
make_oi_DEPENDENCIES = $(DEPS)
make_dem_DEPENDENCIES = $(DEPS)
checksum_DEPENDENCIES = $(DEPS)
utm_conv_DEPENDENCIES = $(DEPS)
dem_to_pgm_DEPENDENCIES = $(DEPS)
make_geovrml_DEPENDENCIES = $(DEPS)
test_inmem_DEPENDENCIES = $(DEPS)
test_inmem3_DEPENDENCIES = $(DEPS)
test_req_DEPENDENCIES = $(DEPS)
test_wms_DEPENDENCIES = $(DEPS)

