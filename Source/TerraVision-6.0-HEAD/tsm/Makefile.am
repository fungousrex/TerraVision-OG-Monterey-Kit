lib_LIBRARIES = @TSMLIB@ 

libtsmApi_a_SOURCES = \
	tsmDatabase.c tsmDatabase_local.c tsmDatabase_web.c \
	tsmDatabase_dpss.c tsmConnectionParams.c \
	tsmConnection.c tsmConnection_dpss.c tsmConnection_web.c \
        tsmConnection_local.c tsmRequest.c tsmRequestSrv.c \
	tsmTile.c tsmTile_dpss.c tsmTile_local.c \
        tsmTile_web.c tsmHost_web.c tsmDatabase_wms.c \
	tsmWMS.c tsmConnection_wms.c tsmTspec_wms.c tsmTile_wms.c \
	tsmTspec.c tsmTspec_dpss.c tsmTspec_local.c \
        tsmTspec_web.c tsmImage.c tsmEmulate.c tsmEmulate_dpss.c \
	tsmEmulate_web.c tsmEmulate_local.c tsmArg.c tsmLocalDbase.c \
        tsmLocalDbase_local.c tsmLocalDbase_dpss.c tsmLocalDbase_web.c \
	tsmPyramidInfo_web.c tsmPyramidInfo_local.c tsmPyramidInfo_dpss.c \
	tsmPyramidInfo.c tsmUrlToFile.c tsmCreate.c tsmCreate_RAW.c \
	tsmAllocTspecs.c tsmTspecUtils.c tsmChecksum.c \
	tsmArray.c tsmPurl.c tsmUtils.c tsmImageUtils.c tsmJPEG.c tsmUTM.c \
	tsmHost.c tsmTrace.c tsmMem.c \
	tsmVRMLUtils.c tsmVRML.c orient.c \
	TsTileSpecUtils.c TsTileSpecIn.c TsTileSpecOut.c \
	TsTileSpecUrls.c TsTileSpecGen.c TsTileSpecParse.c \
	logging.c \
        tsmGeoVRML.c tsmTileCoord.c tsmGeoCoordBuffer.c tsmStringBuffer.c \
	tsmReadRowsPBM.c tsmReadRowsGIF.c tsmReadRowsJPEG.c tsmReadRowsRAW.c \
	tsmReadRowsDEM.c tsmReadRowsDTED.c tsmReadRowsDOQ.c \
	tsmReadRowsTIFF.c tsmWriteRows.c

if DPSS
DPSS_C = TsTileSpecDecs_xdr.c tsmPurlDefns_xdr.c tsmPackTspecs.c
endif

libtsmApi_DPSS_a_SOURCES = $(libtsmApi_a_SOURCES) $(DPSS_C)
libtsmApi_ISS_a_SOURCES = $(libtsmApi_a_SOURCES) $(DPSS_C)

pkginclude_HEADERS = \
	tsm.h TsTileSpecDecs.h TsTileSpecExtProts.h tsmArg.h \
	tsmArray.h tsmGenArray.h tsmPyramidInfo.h tsmExtProts.h \
	tsmPackTspecs.h tsmPurl.h tsmPurlDefns.h tsmImage.h \
	tsmTspecUtils.h tsmDatabase.h tsmConnection.h tsmTile.h \
        tsmConnectionParams.h tsmEmulate.h tsmLocalDbase.h tsmCreate.h \
	tsmVRMLUtils.h logging.h tsmWMS.h tsmConnect_wms.h \
        tsmGeoVRML.h tsmTileCoord.h tsmGeoCoordBuffer.h tsmStringBuffer.h \
	tsmReadRowsPBM.h tsmReadRowsGIF.h tsmReadRowsJPEG.h tsmReadRowsRAW.h \
	tsmReadRowsDEM.h tsmReadRowsDTED.h tsmReadRowsDOQ.h \
	tsmReadRowsTIFF.h tsmWriteRows.h

EXTRA_HEADERS = \
	tsmInternal.h tsmIntProts.h tsmTileInt.h \
	tsmEmulateInt.h TsTileSpecIntProts.h tsmConnectionParamsInt.h \
	tsmRequest.h tsmRequestSrv.h orient.h \
	tsmConnectionInt.h tsmConnect_web.h tsmConnect_local.h

EXTRA_LIBRARIES = libtsmApi.a libtsmApi-DPSS.a libtsmApi-ISS.a

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(pkgincludedir)
	$(INSTALL_DATA) tsmTrace.h $(pkgincludedir);

if DO_RPCGEN
$(srcdir)/TsTileSpecDecs.h: $(srcdir)/TsTileSpecDecs.x
	cd $(srcdir); rpcgen -h -P -o TsTileSpecDecs.h TsTileSpecDecs.x

$(srcdir)/TsTileSpecDecs_xdr.c: $(srcdir)/TsTileSpecDecs.x
	cd $(srcdir); rpcgen -c -P -o TsTileSpecDecs_xdr.c TsTileSpecDecs.x

$(srcdir)/tsmPurlDefns.h: $(srcdir)/tsmPurlDefns.x
	cd $(srcdir); rpcgen -h -P -o tsmPurlDefns.h tsmPurlDefns.x

$(srcdir)/tsmPurlDefns_xdr.c: $(srcdir)/tsmPurlDefns.x
	cd $(srcdir); rpcgen -c -P -o tsmPurlDefns_xdr.c tsmPurlDefns.x
endif

#
#DATASETS = /homedir/magic/TileSets/Pyramids
#
#vtiles: vtile
#	./vtile -oi $(DATASETS)/earth.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth -worldscale 1000000 -elevscale 200
#
#trees: tree
#	./tree -oi $(DATASETS)/earth.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth -worldscale 1000000
#
#round-earth: vtile tree
#	./vtile -oi $(DATASETS)/earth.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth -worldscale 1000000 -elevscale 20
#	./tree -oi $(DATASETS)/earth.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth -worldscale 1000000 -elevscale 20
#
#flat-earth: vtile tree
#	./vtile -oi $(DATASETS)/earth.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth-flat -worldscale 10000000 -elevscale 100 \
#                -flat -numpolys 16
#	./tree -oi $(DATASETS)/earth.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth-flat -worldscale 10000000 -elevscale 100 \
#                -flat -numpolys 16
#
#colorado: vtile tree
#	./tree -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
#                -vrmldir ./colorado -worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm -nogeotiles
#
##	./vtile -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
##                -vrmldir ./colorado -worldscale 100000 -elevscale 5 \
##                -numpolys 8 -flat -utm
#
#p-icon: vtile
#	./vtile -oi $(DATASETS)/pendleton.oi -dem $(DATASETS)/pendleton.dem \
#                -vrmldir ./p-icon -worldscale 1000000 -elevscale 5 \
#                -numpolys 4
#
#pendleton-vtiles: vtile
#	./vtile -oi $(DATASETS)/pendleton.oi -dem $(DATASETS)/pendleton.dem \
#                -vrmldir ./pendleton -worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm
#
#pendleton-trees: tree
#	./tree -oi $(DATASETS)/pendleton.oi -dem $(DATASETS)/pendleton.dem \
#                -vrmldir ./pendleton -worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm -nogeotiles
#
#pendleton: pendleton-vtiles pendleton-trees
#
#pend1deg-vtiles: vtile
#	./vtile -oi $(DATASETS)/camp_1degree.oi \
#                -dem $(DATASETS)/camp_1degree.dem \
#                -vrmldir ./pendleton -worldscale 100000 -elevscale 3 \
#                -numpolys 8 -flat -utm
#
#pend1deg-trees: tree
#	./tree -oi $(DATASETS)/camp_1degree.oi \
#               -dem $(DATASETS)/camp_1degree.dem \
#               -vrmldir ./pendleton -worldscale 100000 -elevscale 3 \
#               -numpolys 8 -flat -utm -nogeotiles
#
#pend1deg: pendleton-vtiles pendleton-trees
#
#geotile-vtiles: vtile
#	./vtile -oi $(DATASETS)/pendleton.oi -dem $(DATASETS)/pendleton.dem \
#                -vrmldir ./GeoTileTest -worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm
#
#geotile-trees: tree
#	./tree -oi $(DATASETS)/pendleton.oi -dem $(DATASETS)/pendleton.dem \
#                -vrmldir ./GeoTileTest -worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm
#
#geotile: geotile-vtiles geotile-trees
#
#test: tree
#	./tree -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
#		-vrmldir ./test -worldscale 100000 -elevscale 5 \
#		-numpolys 8 -flat -utm -quadlodtrees
#
#test-vtiles: vtile
#	./vtile -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
#		-vrmldir ./test -worldscale 100000 -elevscale 5 \
#		-numpolys 8 -flat -utm
#
#ctrees: tree
#	./tree -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
#                -vrmldir ./colorado -worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm -quadlodtrees \
#		-absurl "http://www.ai.sri.com/VRMLSets/ColoradoGeoTile/"
#
#cvtiles: vtile
#	./vtile -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
#                -vrmldir ./colorado -worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm \
#		-absurl "http://www.ai.sri.com/VRMLSets/ColoradoGeoTile/"
#
#cimages: image
#	./image -oi $(DATASETS)/co.oi  -vrmldir ./colorado
#
## Ft. Benning
#btrees: tree
#	./tree -oi $(DATASETS)/benning.oi -dem $(DATASETS)/benning.dem \
#                -vrmldir ./benning -worldscale 100000 -elevscale 1 \
#                -numpolys 8 -flat -utm -quadlodtrees \
#		-absurl "http://www.ai.sri.com/VRMLSets/BenningGeoTile/"
#
#bvtiles: vtile
#	./vtile -oi $(DATASETS)/benning.oi -dem $(DATASETS)/benning.dem \
#                -vrmldir ./benning -worldscale 100000 -elevscale 1 \
#                -numpolys 8 -flat -utm \
#		-absurl "http://www.ai.sri.com/VRMLSets/BenningGeoTile/"
#
## Earth
#etrees: tree
#	./tree -oi /home/crazy5/magic/TileSets/Pyramids/Earth-AVHRR.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth -worldscale 100000 -elevscale 50 \
#                -numpolys 16 -quadlodtrees \
#		-absurl "http://www.ai.sri.com/VRMLSets/EarthGeoTile/"
#
#evtiles: vtile
#	./vtile -oi /home/crazy5/magic/TileSets/Pyramids/Earth-AVHRR.oi -dem $(DATASETS)/earth.dem \
#                -vrmldir ./earth -worldscale 100000 -elevscale 50 \
#                -numpolys 16 \
#		-absurl "http://www.ai.sri.com/VRMLSets/EarthGeoTile/"
#
#eimages: image
#	./image -oi /home/crazy5/magic/TileSets/Pyramids/Earth-AVHRR.oi -vrmldir ./earth
#
## Yosemite
#ytrees: tree
#	./tree -oi x-dpss://iss-1-spans.sri.magic.net/Pyramids/yosemite.oi \
#		-dem x-dpss://iss-1-spans.sri.magic.net/Pyramids/yosemite.dem \
#                -vrmldir ./yosemite -utm -worldscale 100000 -elevscale 1 \
#                -numpolys 16 -quadlodtrees \
#		-absurl "http://www.ai.sri.com/VRMLSets/YosemiteGeoTile/"
#
#yvtiles: vtile
#	./vtile -oi x-dpss://iss-1-spans.sri.magic.net/Pyramids/yosemite.oi \
#		-dem x-dpss://iss-1-spans.sri.magic.net/Pyramids/yosemite.dem \
#                -vrmldir ./yosemite -utm -worldscale 100000 -elevscale 1 \
#                -numpolys 16 \
#		-absurl "http://www.ai.sri.com/VRMLSets/YosemiteGeoTile/"
#
## Colorado PC
#ctrees-pc: tree
#	./tree -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
#                -vrmldir /home/lomond2/magic/GeoTile/Colorado \
#		-worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm -quadlodtrees \
#		-absurl "file:/D:/VRMLSets/GeoTile/Colorado/"
#
#cvtiles-pc: vtile
#	./vtile -oi $(DATASETS)/co.oi -dem $(DATASETS)/co.dem \
#                -vrmldir /home/lomond2/magic/GeoTile/Colorado \
#		-worldscale 100000 -elevscale 5 \
#                -numpolys 8 -flat -utm \
#		-absurl "file:/D:/VRMLSets/GeoTile/Colorado/"
#
## Benning
#benning-trees: tree
#	./tree -oi $(DATASETS)/benning.oi -dem $(DATASETS)/benning.dem \
#                -vrmldir ./Benning -worldscale 100000 -elevscale 1 \
#                -flat -utm -lods 5 -start 10 -end 7 -nogeotiles
#
#benning-vtiles: vtile
#	./vtile -oi $(DATASETS)/benning.oi -dem $(DATASETS)/benning.dem \
#                -vrmldir ./Benning -worldscale 100000 -elevscale 1 \
#                -flat -utm -lods 6
#
#benning: benning-trees benning-vtiles
#
#
