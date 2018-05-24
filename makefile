CC = g++
CPPFLAGS = -Wall #-g #-D OLD_FFMPEG #-pg
LINK = -lX11 -lz -lpthread #-lexpat #-pg
CV_LIB = -lcv -lcvaux -lhighgui
INCLUDE = 
CV_INC = -I/usr/include/opencv
BUILD_DEPS = Stats.h
WARP_OBJ = Warp.o TransformPoly.o WriteKml.o ImageWarpByFunc.o ReadDelimitedFile.o StringUtils.o ganzc/LatLong-OSGBconversion.o gbos1936/Gbos1936.o ImgMagick.o
WARP_OS_OBJ = WarpOs.o TransformPoly.o WriteKml.o ImageWarpByFunc.o ReadDelimitedFile.o StringUtils.o ganzc/LatLong-OSGBconversion.o gbos1936/Gbos1936.o ImgMagick.o Tile.o
CLEAR_TILES_OBJ = ClearTiles.o gbos1936/Gbos1936.o StringUtils.o Tile.o ImgMagick.o ImageWarpByFunc.o ReadDelimitedFile.o ganzc/LatLong-OSGBconversion.o ReadKmlFile.o GetBounds.o
GEN_TILE_OBJ = GenTiles.o gbos1936/Gbos1936.o StringUtils.o Tile.o ImgMagick.o ImageWarpByFunc.o ReadDelimitedFile.o ganzc/LatLong-OSGBconversion.o ReadKmlFile.o GetBounds.o
MASK_RECTANGLE = MaskRectangle.o Gbos1936.o ganzc/LatLong-OSGBconversion.o ImgMagick.o
GM_INCLUDE = `GraphicsMagick++-config --cppflags --cxxflags --ldflags`
GM_LINK = `GraphicsMagick++-config --libs`
FFMPEG_LINK = -lavutil -lavformat -lavcodec
BOOST_LINK = -lboost_program_options-mt
GTK_FLAGS = `pkg-config --cflags gtk+-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0`
NEWMAT_LNK = -lnewmat
NEWMAT_INC = -Inewmat
AVCODEC_INC = `pkg-config --cflags libavcodec`
AVCODEC_LINK = `pkg-config --libs libavcodec`
AVFORMAT_INC = `pkg-config --cflags libavformat`
AVFORMAT_LINK = `pkg-config --libs libavformat`
SWSCALE_INC = `pkg-config --cflags swscale`
SWSCALE_LINK = `pkg-config --libs swscale`
MAGICKWAND_INC = `MagickWand-config --cflags --cppflags`
MAGICKWAND_LIB = `MagickWand-config --ldflags --libs`
LIBXML++_INC = `pkg-config libxml++-2.6 --cflags`
LIBXML++_LIB = `pkg-config libxml++-2.6 --libs`
MAGICK++_LIB = -lMagick++
BOOST_FS = -lboost_filesystem-mt
BOOST_THREAD = -lboost_thread-mt
#If you can't compile with the higher accuracy OSTN02, available in Perl, comment out the next 4 lines.
PERL_INC = -I/usr/lib/perl/5.10/CORE
PERL_LIB = -lperl
PERL_DEF = -DWITH_OSTN02_PERL
PERL_OBJ = OSTN02Perl.o
PERL_CONV = $(PERL_LIB) $(PERL_DEF)
CONV = -DWITH_OSTN02_PYTHON -I/usr/include/python2.6 -lpython2.6
CONV_OBJ = PyOstn02.o

#GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs
all: warpos gentiles
%.o : %.cpp
	$(CC) -c $(CPPFLAGS) $(INCLUDE) $(MAGICKWAND_INC) $(LIBXML++_INC) $(CONV) $< -o $@
cimg.gch:
	g++ -c CImg/CImg.h -o cimg.gch

warp: $(WARP_OBJ)
	g++  $(WARP_OBJ) $(BOOST_LINK) $(MAGICKWAND_LIB) -lnewmat -lmorph -ICImg -lX11 $(CPPFLAGS) -lpthread -o $@
warpos: $(WARP_OS_OBJ) $(CONV_OBJ)
	g++  $(WARP_OS_OBJ) $(CONV_OBJ) $(BOOST_LINK) $(MAGICKWAND_LIB) $(CONV) -lnewmat -lmorph -ICImg -lX11 $(CPPFLAGS) -lpthread -o $@
 : $(CLEAR_TILES_OBJ) $(CONV_OBJ)
	g++  $(CLEAR_TILES_OBJ) $(CONV_OBJ) $(BOOST_LINK) $(MAGICKWAND_LIB) $(BOOST_FS) $(CONV) $(LIBXML++_LIB) -lX11 -lmorph $(CPPFLAGS) -lpthread -o $@
gentiles: $(GEN_TILE_OBJ) $(CONV_OBJ)
	g++  $(GEN_TILE_OBJ) $(CONV_OBJ) $(BOOST_LINK) $(MAGICKWAND_LIB) $(BOOST_FS) $(BOOST_THREAD) $(CONV) $(LIBXML++_LIB) -lX11 -lmorph $(CPPFLAGS) -lpthread -o $@
maskrectangle: $(MASK_RECTANGLE)
	g++  $(MASK_RECTANGLE) $(BOOST_LINK) $(MAGICKWAND_LIB) $(BOOST_FS) -lX11 -lmorph $(CPPFLAGS) -lpthread -o $@
cleartiles: $(CLEAR_TILES_OBJ)
	g++  $(CLEAR_TILES_OBJ) $(BOOST_LINK) $(MAGICKWAND_LIB) $(BOOST_FS) $(LIBXML++_LIB) -lX11 -lmorph $(CPPFLAGS) -lpthread -o $@

newmat/libnewmat.a:
	cd newmat && make -f nm_gnu.mak

clean: clean_prog
	
clean_prog:
	rm *.gch *~ *.o *.a warp *.png warpos maskrectangle gentiles ganzc/*.o ganzc/*~ gbos1936/*.o gbos1936/*~ cleartiles

