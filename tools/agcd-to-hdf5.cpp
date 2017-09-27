#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <sys/stat.h>

#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

#include <png.h>

#include <hdf5.h>
#include <hdf5_hl.h>

static const int WIDTH = 160;
static const int HEIGHT = 210;
static const int MAX_PATH_LENGTH = 2048;

typedef unsigned char pixel_t;

typedef struct {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers;
} png_data_t;

struct agcd_frame_t {
    int frame;
    int reward;
    int score;
    int terminal;
    int action;
};

static const pixel_t NTSC_palette[] = { /* {{{ */
	(pixel_t)0, (pixel_t)0, (pixel_t)0,
	(pixel_t)0, (pixel_t)0, (pixel_t)0,
	(pixel_t)74, (pixel_t)74, (pixel_t)74,
	(pixel_t)74, (pixel_t)74, (pixel_t)74,
	(pixel_t)111, (pixel_t)111, (pixel_t)111,
	(pixel_t)111, (pixel_t)111, (pixel_t)111,
	(pixel_t)142, (pixel_t)142, (pixel_t)142,
	(pixel_t)142, (pixel_t)142, (pixel_t)142,
	(pixel_t)170, (pixel_t)170, (pixel_t)170,
	(pixel_t)170, (pixel_t)170, (pixel_t)170,
	(pixel_t)192, (pixel_t)192, (pixel_t)192,
	(pixel_t)192, (pixel_t)192, (pixel_t)192,
	(pixel_t)214, (pixel_t)214, (pixel_t)214,
	(pixel_t)214, (pixel_t)214, (pixel_t)214,
	(pixel_t)236, (pixel_t)236, (pixel_t)236,
	(pixel_t)236, (pixel_t)236, (pixel_t)236,
	(pixel_t)72, (pixel_t)72, (pixel_t)0,
	(pixel_t)64, (pixel_t)64, (pixel_t)64,
	(pixel_t)105, (pixel_t)105, (pixel_t)15,
	(pixel_t)95, (pixel_t)95, (pixel_t)95,
	(pixel_t)134, (pixel_t)134, (pixel_t)29,
	(pixel_t)122, (pixel_t)122, (pixel_t)122,
	(pixel_t)162, (pixel_t)162, (pixel_t)42,
	(pixel_t)148, (pixel_t)148, (pixel_t)148,
	(pixel_t)187, (pixel_t)187, (pixel_t)53,
	(pixel_t)172, (pixel_t)172, (pixel_t)172,
	(pixel_t)210, (pixel_t)210, (pixel_t)64,
	(pixel_t)193, (pixel_t)193, (pixel_t)193,
	(pixel_t)232, (pixel_t)232, (pixel_t)74,
	(pixel_t)214, (pixel_t)214, (pixel_t)214,
	(pixel_t)252, (pixel_t)252, (pixel_t)84,
	(pixel_t)233, (pixel_t)233, (pixel_t)233,
	(pixel_t)124, (pixel_t)44, (pixel_t)0,
	(pixel_t)63, (pixel_t)63, (pixel_t)63,
	(pixel_t)144, (pixel_t)72, (pixel_t)17,
	(pixel_t)87, (pixel_t)87, (pixel_t)87,
	(pixel_t)162, (pixel_t)98, (pixel_t)33,
	(pixel_t)110, (pixel_t)110, (pixel_t)110,
	(pixel_t)180, (pixel_t)122, (pixel_t)48,
	(pixel_t)131, (pixel_t)131, (pixel_t)131,
	(pixel_t)195, (pixel_t)144, (pixel_t)61,
	(pixel_t)150, (pixel_t)150, (pixel_t)150,
	(pixel_t)210, (pixel_t)164, (pixel_t)74,
	(pixel_t)167, (pixel_t)167, (pixel_t)167,
	(pixel_t)223, (pixel_t)183, (pixel_t)85,
	(pixel_t)184, (pixel_t)184, (pixel_t)184,
	(pixel_t)236, (pixel_t)200, (pixel_t)96,
	(pixel_t)199, (pixel_t)199, (pixel_t)199,
	(pixel_t)144, (pixel_t)28, (pixel_t)0,
	(pixel_t)59, (pixel_t)59, (pixel_t)59,
	(pixel_t)163, (pixel_t)57, (pixel_t)21,
	(pixel_t)85, (pixel_t)85, (pixel_t)85,
	(pixel_t)181, (pixel_t)83, (pixel_t)40,
	(pixel_t)107, (pixel_t)107, (pixel_t)107,
	(pixel_t)198, (pixel_t)108, (pixel_t)58,
	(pixel_t)129, (pixel_t)129, (pixel_t)129,
	(pixel_t)213, (pixel_t)130, (pixel_t)74,
	(pixel_t)148, (pixel_t)148, (pixel_t)148,
	(pixel_t)227, (pixel_t)151, (pixel_t)89,
	(pixel_t)167, (pixel_t)167, (pixel_t)167,
	(pixel_t)240, (pixel_t)170, (pixel_t)103,
	(pixel_t)183, (pixel_t)183, (pixel_t)183,
	(pixel_t)252, (pixel_t)188, (pixel_t)116,
	(pixel_t)199, (pixel_t)199, (pixel_t)199,
	(pixel_t)148, (pixel_t)0, (pixel_t)0,
	(pixel_t)44, (pixel_t)44, (pixel_t)44,
	(pixel_t)167, (pixel_t)26, (pixel_t)26,
	(pixel_t)68, (pixel_t)68, (pixel_t)68,
	(pixel_t)184, (pixel_t)50, (pixel_t)50,
	(pixel_t)90, (pixel_t)90, (pixel_t)90,
	(pixel_t)200, (pixel_t)72, (pixel_t)72,
	(pixel_t)110, (pixel_t)110, (pixel_t)110,
	(pixel_t)214, (pixel_t)92, (pixel_t)92,
	(pixel_t)128, (pixel_t)128, (pixel_t)128,
	(pixel_t)228, (pixel_t)111, (pixel_t)111,
	(pixel_t)146, (pixel_t)146, (pixel_t)146,
	(pixel_t)240, (pixel_t)128, (pixel_t)128,
	(pixel_t)161, (pixel_t)161, (pixel_t)161,
	(pixel_t)252, (pixel_t)144, (pixel_t)144,
	(pixel_t)176, (pixel_t)176, (pixel_t)176,
	(pixel_t)132, (pixel_t)0, (pixel_t)100,
	(pixel_t)51, (pixel_t)51, (pixel_t)51,
	(pixel_t)151, (pixel_t)25, (pixel_t)122,
	(pixel_t)74, (pixel_t)74, (pixel_t)74,
	(pixel_t)168, (pixel_t)48, (pixel_t)143,
	(pixel_t)95, (pixel_t)95, (pixel_t)95,
	(pixel_t)184, (pixel_t)70, (pixel_t)162,
	(pixel_t)115, (pixel_t)115, (pixel_t)115,
	(pixel_t)198, (pixel_t)89, (pixel_t)179,
	(pixel_t)132, (pixel_t)132, (pixel_t)132,
	(pixel_t)212, (pixel_t)108, (pixel_t)195,
	(pixel_t)149, (pixel_t)149, (pixel_t)149,
	(pixel_t)224, (pixel_t)124, (pixel_t)210,
	(pixel_t)164, (pixel_t)164, (pixel_t)164,
	(pixel_t)236, (pixel_t)140, (pixel_t)224,
	(pixel_t)178, (pixel_t)178, (pixel_t)178,
	(pixel_t)80, (pixel_t)0, (pixel_t)132,
	(pixel_t)39, (pixel_t)39, (pixel_t)39,
	(pixel_t)104, (pixel_t)25, (pixel_t)154,
	(pixel_t)63, (pixel_t)63, (pixel_t)63,
	(pixel_t)125, (pixel_t)48, (pixel_t)173,
	(pixel_t)85, (pixel_t)85, (pixel_t)85,
	(pixel_t)146, (pixel_t)70, (pixel_t)192,
	(pixel_t)107, (pixel_t)107, (pixel_t)107,
	(pixel_t)164, (pixel_t)89, (pixel_t)208,
	(pixel_t)125, (pixel_t)125, (pixel_t)125,
	(pixel_t)181, (pixel_t)108, (pixel_t)224,
	(pixel_t)143, (pixel_t)143, (pixel_t)143,
	(pixel_t)197, (pixel_t)124, (pixel_t)238,
	(pixel_t)159, (pixel_t)159, (pixel_t)159,
	(pixel_t)212, (pixel_t)140, (pixel_t)252,
	(pixel_t)174, (pixel_t)174, (pixel_t)174,
	(pixel_t)20, (pixel_t)0, (pixel_t)144,
	(pixel_t)22, (pixel_t)22, (pixel_t)22,
	(pixel_t)51, (pixel_t)26, (pixel_t)163,
	(pixel_t)49, (pixel_t)49, (pixel_t)49,
	(pixel_t)78, (pixel_t)50, (pixel_t)181,
	(pixel_t)73, (pixel_t)73, (pixel_t)73,
	(pixel_t)104, (pixel_t)72, (pixel_t)198,
	(pixel_t)96, (pixel_t)96, (pixel_t)96,
	(pixel_t)127, (pixel_t)92, (pixel_t)213,
	(pixel_t)116, (pixel_t)116, (pixel_t)116,
	(pixel_t)149, (pixel_t)111, (pixel_t)227,
	(pixel_t)136, (pixel_t)136, (pixel_t)136,
	(pixel_t)169, (pixel_t)128, (pixel_t)240,
	(pixel_t)153, (pixel_t)153, (pixel_t)153,
	(pixel_t)188, (pixel_t)144, (pixel_t)252,
	(pixel_t)169, (pixel_t)169, (pixel_t)169,
	(pixel_t)0, (pixel_t)0, (pixel_t)148,
	(pixel_t)17, (pixel_t)17, (pixel_t)17,
	(pixel_t)24, (pixel_t)26, (pixel_t)167,
	(pixel_t)41, (pixel_t)41, (pixel_t)41,
	(pixel_t)45, (pixel_t)50, (pixel_t)184,
	(pixel_t)64, (pixel_t)64, (pixel_t)64,
	(pixel_t)66, (pixel_t)72, (pixel_t)200,
	(pixel_t)85, (pixel_t)85, (pixel_t)85,
	(pixel_t)84, (pixel_t)92, (pixel_t)214,
	(pixel_t)104, (pixel_t)104, (pixel_t)104,
	(pixel_t)101, (pixel_t)111, (pixel_t)228,
	(pixel_t)121, (pixel_t)121, (pixel_t)121,
	(pixel_t)117, (pixel_t)128, (pixel_t)240,
	(pixel_t)137, (pixel_t)137, (pixel_t)137,
	(pixel_t)132, (pixel_t)144, (pixel_t)252,
	(pixel_t)153, (pixel_t)153, (pixel_t)153,
	(pixel_t)0, (pixel_t)28, (pixel_t)136,
	(pixel_t)32, (pixel_t)32, (pixel_t)32,
	(pixel_t)24, (pixel_t)59, (pixel_t)157,
	(pixel_t)60, (pixel_t)60, (pixel_t)60,
	(pixel_t)45, (pixel_t)87, (pixel_t)176,
	(pixel_t)85, (pixel_t)85, (pixel_t)85,
	(pixel_t)66, (pixel_t)114, (pixel_t)194,
	(pixel_t)109, (pixel_t)109, (pixel_t)109,
	(pixel_t)84, (pixel_t)138, (pixel_t)210,
	(pixel_t)130, (pixel_t)130, (pixel_t)130,
	(pixel_t)101, (pixel_t)160, (pixel_t)225,
	(pixel_t)150, (pixel_t)150, (pixel_t)150,
	(pixel_t)117, (pixel_t)181, (pixel_t)239,
	(pixel_t)168, (pixel_t)168, (pixel_t)168,
	(pixel_t)132, (pixel_t)200, (pixel_t)252,
	(pixel_t)186, (pixel_t)186, (pixel_t)186,
	(pixel_t)0, (pixel_t)48, (pixel_t)100,
	(pixel_t)40, (pixel_t)40, (pixel_t)40,
	(pixel_t)24, (pixel_t)80, (pixel_t)128,
	(pixel_t)69, (pixel_t)69, (pixel_t)69,
	(pixel_t)45, (pixel_t)109, (pixel_t)152,
	(pixel_t)95, (pixel_t)95, (pixel_t)95,
	(pixel_t)66, (pixel_t)136, (pixel_t)176,
	(pixel_t)120, (pixel_t)120, (pixel_t)120,
	(pixel_t)84, (pixel_t)160, (pixel_t)197,
	(pixel_t)141, (pixel_t)141, (pixel_t)141,
	(pixel_t)101, (pixel_t)183, (pixel_t)217,
	(pixel_t)162, (pixel_t)162, (pixel_t)162,
	(pixel_t)117, (pixel_t)204, (pixel_t)235,
	(pixel_t)182, (pixel_t)182, (pixel_t)182,
	(pixel_t)132, (pixel_t)224, (pixel_t)252,
	(pixel_t)200, (pixel_t)200, (pixel_t)200,
	(pixel_t)0, (pixel_t)64, (pixel_t)48,
	(pixel_t)43, (pixel_t)43, (pixel_t)43,
	(pixel_t)24, (pixel_t)98, (pixel_t)78,
	(pixel_t)74, (pixel_t)74, (pixel_t)74,
	(pixel_t)45, (pixel_t)129, (pixel_t)105,
	(pixel_t)101, (pixel_t)101, (pixel_t)101,
	(pixel_t)66, (pixel_t)158, (pixel_t)130,
	(pixel_t)127, (pixel_t)127, (pixel_t)127,
	(pixel_t)84, (pixel_t)184, (pixel_t)153,
	(pixel_t)151, (pixel_t)151, (pixel_t)151,
	(pixel_t)101, (pixel_t)209, (pixel_t)174,
	(pixel_t)173, (pixel_t)173, (pixel_t)173,
	(pixel_t)117, (pixel_t)231, (pixel_t)194,
	(pixel_t)193, (pixel_t)193, (pixel_t)193,
	(pixel_t)132, (pixel_t)252, (pixel_t)212,
	(pixel_t)212, (pixel_t)212, (pixel_t)212,
	(pixel_t)0, (pixel_t)68, (pixel_t)0,
	(pixel_t)40, (pixel_t)40, (pixel_t)40,
	(pixel_t)26, (pixel_t)102, (pixel_t)26,
	(pixel_t)71, (pixel_t)71, (pixel_t)71,
	(pixel_t)50, (pixel_t)132, (pixel_t)50,
	(pixel_t)98, (pixel_t)98, (pixel_t)98,
	(pixel_t)72, (pixel_t)160, (pixel_t)72,
	(pixel_t)124, (pixel_t)124, (pixel_t)124,
	(pixel_t)92, (pixel_t)186, (pixel_t)92,
	(pixel_t)147, (pixel_t)147, (pixel_t)147,
	(pixel_t)111, (pixel_t)210, (pixel_t)111,
	(pixel_t)169, (pixel_t)169, (pixel_t)169,
	(pixel_t)128, (pixel_t)232, (pixel_t)128,
	(pixel_t)189, (pixel_t)189, (pixel_t)189,
	(pixel_t)144, (pixel_t)252, (pixel_t)144,
	(pixel_t)207, (pixel_t)207, (pixel_t)207,
	(pixel_t)20, (pixel_t)60, (pixel_t)0,
	(pixel_t)41, (pixel_t)41, (pixel_t)41,
	(pixel_t)53, (pixel_t)95, (pixel_t)24,
	(pixel_t)74, (pixel_t)74, (pixel_t)74,
	(pixel_t)82, (pixel_t)126, (pixel_t)45,
	(pixel_t)104, (pixel_t)104, (pixel_t)104,
	(pixel_t)110, (pixel_t)156, (pixel_t)66,
	(pixel_t)132, (pixel_t)132, (pixel_t)132,
	(pixel_t)135, (pixel_t)183, (pixel_t)84,
	(pixel_t)157, (pixel_t)157, (pixel_t)157,
	(pixel_t)158, (pixel_t)208, (pixel_t)101,
	(pixel_t)181, (pixel_t)181, (pixel_t)181,
	(pixel_t)180, (pixel_t)231, (pixel_t)117,
	(pixel_t)203, (pixel_t)203, (pixel_t)203,
	(pixel_t)200, (pixel_t)252, (pixel_t)132,
	(pixel_t)223, (pixel_t)223, (pixel_t)223,
	(pixel_t)48, (pixel_t)56, (pixel_t)0,
	(pixel_t)47, (pixel_t)47, (pixel_t)47,
	(pixel_t)80, (pixel_t)89, (pixel_t)22,
	(pixel_t)79, (pixel_t)79, (pixel_t)79,
	(pixel_t)109, (pixel_t)118, (pixel_t)43,
	(pixel_t)107, (pixel_t)107, (pixel_t)107,
	(pixel_t)136, (pixel_t)146, (pixel_t)62,
	(pixel_t)133, (pixel_t)133, (pixel_t)133,
	(pixel_t)160, (pixel_t)171, (pixel_t)79,
	(pixel_t)157, (pixel_t)157, (pixel_t)157,
	(pixel_t)183, (pixel_t)194, (pixel_t)95,
	(pixel_t)179, (pixel_t)179, (pixel_t)179,
	(pixel_t)204, (pixel_t)216, (pixel_t)110,
	(pixel_t)200, (pixel_t)200, (pixel_t)200,
	(pixel_t)224, (pixel_t)236, (pixel_t)124,
	(pixel_t)220, (pixel_t)220, (pixel_t)220,
	(pixel_t)72, (pixel_t)44, (pixel_t)0,
	(pixel_t)47, (pixel_t)47, (pixel_t)47,
	(pixel_t)105, (pixel_t)77, (pixel_t)20,
	(pixel_t)79, (pixel_t)79, (pixel_t)79,
	(pixel_t)134, (pixel_t)106, (pixel_t)38,
	(pixel_t)107, (pixel_t)107, (pixel_t)107,
	(pixel_t)162, (pixel_t)134, (pixel_t)56,
	(pixel_t)133, (pixel_t)133, (pixel_t)133,
	(pixel_t)187, (pixel_t)159, (pixel_t)71,
	(pixel_t)157, (pixel_t)157, (pixel_t)157,
	(pixel_t)210, (pixel_t)182, (pixel_t)86,
	(pixel_t)179, (pixel_t)179, (pixel_t)179,
	(pixel_t)232, (pixel_t)204, (pixel_t)99,
	(pixel_t)200, (pixel_t)200, (pixel_t)200,
	(pixel_t)252, (pixel_t)224, (pixel_t)112,
	(pixel_t)220, (pixel_t)220, (pixel_t)220
}; /* }}} */

static inline png_data_t read_png_file(const char *filename) { /* {{{ */
    png_data_t data;

    FILE *fp = fopen(filename, "rb");

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                             NULL,
                                             NULL,
                                             NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png);
    if(!info) abort();

    if(setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_read_info(png, info);

    data.width      = png_get_image_width(png, info);
    data.height     = png_get_image_height(png, info);
    data.color_type = png_get_color_type(png, info);
    data.bit_depth  = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if (data.bit_depth == 16)
        png_set_strip_16(png);

    if (data.color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(data.color_type == PNG_COLOR_TYPE_GRAY && data.bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if(data.color_type == PNG_COLOR_TYPE_RGB ||
       data.color_type == PNG_COLOR_TYPE_GRAY ||
       data.color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(data.color_type == PNG_COLOR_TYPE_GRAY ||
       data.color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    data.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * data.height);
    for(int y = 0; y < data.height; y++) {
        data.row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, data.row_pointers);

    png_destroy_read_struct(&png, &info, (png_infopp)NULL);

    fclose(fp);

    return data;
} /* }}} */

static pixel_t rgb_to_ntsc_index(png_bytep pixel) { /* {{{ */
    if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
        return 0;
    }

    if (pixel[0] == 64 && pixel[1] == 64 && pixel[2] == 64) {
        return 2;
    }

    if (pixel[0] == 108 && pixel[1] == 108 && pixel[2] == 108) {
        return 4;
    }

    if (pixel[0] == 144 && pixel[1] == 144 && pixel[2] == 144) {
        return 6;
    }

    if (pixel[0] == 176 && pixel[1] == 176 && pixel[2] == 176) {
        return 8;
    }

    if (pixel[0] == 200 && pixel[1] == 200 && pixel[2] == 200) {
        return 10;
    }

    if (pixel[0] == 220 && pixel[1] == 220 && pixel[2] == 220) {
        return 12;
    }

    if (pixel[0] == 236 && pixel[1] == 236 && pixel[2] == 236) {
        return 14;
    }

    if (pixel[0] == 68 && pixel[1] == 68 && pixel[2] == 0) {
        return 16;
    }

    if (pixel[0] == 100 && pixel[1] == 100 && pixel[2] == 16) {
        return 18;
    }

    if (pixel[0] == 132 && pixel[1] == 132 && pixel[2] == 36) {
        return 20;
    }

    if (pixel[0] == 160 && pixel[1] == 160 && pixel[2] == 52) {
        return 22;
    }

    if (pixel[0] == 184 && pixel[1] == 184 && pixel[2] == 64) {
        return 24;
    }

    if (pixel[0] == 208 && pixel[1] == 208 && pixel[2] == 80) {
        return 26;
    }

    if (pixel[0] == 232 && pixel[1] == 232 && pixel[2] == 92) {
        return 28;
    }

    if (pixel[0] == 252 && pixel[1] == 252 && pixel[2] == 104) {
        return 30;
    }

    if (pixel[0] == 112 && pixel[1] == 40 && pixel[2] == 0) {
        return 32;
    }

    if (pixel[0] == 132 && pixel[1] == 68 && pixel[2] == 20) {
        return 34;
    }

    if (pixel[0] == 152 && pixel[1] == 92 && pixel[2] == 40) {
        return 36;
    }

    if (pixel[0] == 172 && pixel[1] == 120 && pixel[2] == 60) {
        return 38;
    }

    if (pixel[0] == 188 && pixel[1] == 140 && pixel[2] == 76) {
        return 40;
    }

    if (pixel[0] == 204 && pixel[1] == 160 && pixel[2] == 92) {
        return 42;
    }

    if (pixel[0] == 220 && pixel[1] == 180 && pixel[2] == 104) {
        return 44;
    }

    if (pixel[0] == 236 && pixel[1] == 200 && pixel[2] == 120) {
        return 46;
    }

    if (pixel[0] == 132 && pixel[1] == 24 && pixel[2] == 0) {
        return 48;
    }

    if (pixel[0] == 152 && pixel[1] == 52 && pixel[2] == 24) {
        return 50;
    }

    if (pixel[0] == 172 && pixel[1] == 80 && pixel[2] == 48) {
        return 52;
    }

    if (pixel[0] == 192 && pixel[1] == 104 && pixel[2] == 72) {
        return 54;
    }

    if (pixel[0] == 208 && pixel[1] == 128 && pixel[2] == 92) {
        return 56;
    }

    if (pixel[0] == 224 && pixel[1] == 148 && pixel[2] == 112) {
        return 58;
    }

    if (pixel[0] == 236 && pixel[1] == 168 && pixel[2] == 128) {
        return 60;
    }

    if (pixel[0] == 252 && pixel[1] == 188 && pixel[2] == 148) {
        return 62;
    }

    if (pixel[0] == 136 && pixel[1] == 0 && pixel[2] == 0) {
        return 64;
    }

    if (pixel[0] == 156 && pixel[1] == 32 && pixel[2] == 32) {
        return 66;
    }

    if (pixel[0] == 176 && pixel[1] == 60 && pixel[2] == 60) {
        return 68;
    }

    if (pixel[0] == 192 && pixel[1] == 88 && pixel[2] == 88) {
        return 70;
    }

    if (pixel[0] == 208 && pixel[1] == 112 && pixel[2] == 112) {
        return 72;
    }

    if (pixel[0] == 224 && pixel[1] == 136 && pixel[2] == 136) {
        return 74;
    }

    if (pixel[0] == 236 && pixel[1] == 160 && pixel[2] == 160) {
        return 76;
    }

    if (pixel[0] == 252 && pixel[1] == 180 && pixel[2] == 180) {
        return 78;
    }

    if (pixel[0] == 120 && pixel[1] == 0 && pixel[2] == 92) {
        return 80;
    }

    if (pixel[0] == 140 && pixel[1] == 32 && pixel[2] == 116) {
        return 82;
    }

    if (pixel[0] == 160 && pixel[1] == 60 && pixel[2] == 136) {
        return 84;
    }

    if (pixel[0] == 176 && pixel[1] == 88 && pixel[2] == 156) {
        return 86;
    }

    if (pixel[0] == 192 && pixel[1] == 112 && pixel[2] == 176) {
        return 88;
    }

    if (pixel[0] == 208 && pixel[1] == 132 && pixel[2] == 192) {
        return 90;
    }

    if (pixel[0] == 220 && pixel[1] == 156 && pixel[2] == 208) {
        return 92;
    }

    if (pixel[0] == 236 && pixel[1] == 176 && pixel[2] == 224) {
        return 94;
    }

    if (pixel[0] == 72 && pixel[1] == 0 && pixel[2] == 120) {
        return 96;
    }

    if (pixel[0] == 96 && pixel[1] == 32 && pixel[2] == 144) {
        return 98;
    }

    if (pixel[0] == 120 && pixel[1] == 60 && pixel[2] == 164) {
        return 100;
    }

    if (pixel[0] == 140 && pixel[1] == 88 && pixel[2] == 184) {
        return 102;
    }

    if (pixel[0] == 160 && pixel[1] == 112 && pixel[2] == 204) {
        return 104;
    }

    if (pixel[0] == 180 && pixel[1] == 132 && pixel[2] == 220) {
        return 106;
    }

    if (pixel[0] == 196 && pixel[1] == 156 && pixel[2] == 236) {
        return 108;
    }

    if (pixel[0] == 212 && pixel[1] == 176 && pixel[2] == 252) {
        return 110;
    }

    if (pixel[0] == 20 && pixel[1] == 0 && pixel[2] == 132) {
        return 112;
    }

    if (pixel[0] == 48 && pixel[1] == 32 && pixel[2] == 152) {
        return 114;
    }

    if (pixel[0] == 76 && pixel[1] == 60 && pixel[2] == 172) {
        return 116;
    }

    if (pixel[0] == 104 && pixel[1] == 88 && pixel[2] == 192) {
        return 118;
    }

    if (pixel[0] == 124 && pixel[1] == 112 && pixel[2] == 208) {
        return 120;
    }

    if (pixel[0] == 148 && pixel[1] == 136 && pixel[2] == 224) {
        return 122;
    }

    if (pixel[0] == 168 && pixel[1] == 160 && pixel[2] == 236) {
        return 124;
    }

    if (pixel[0] == 188 && pixel[1] == 180 && pixel[2] == 252) {
        return 126;
    }

    if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 136) {
        return 128;
    }

    if (pixel[0] == 28 && pixel[1] == 32 && pixel[2] == 156) {
        return 130;
    }

    if (pixel[0] == 56 && pixel[1] == 64 && pixel[2] == 176) {
        return 132;
    }

    if (pixel[0] == 80 && pixel[1] == 92 && pixel[2] == 192) {
        return 134;
    }

    if (pixel[0] == 104 && pixel[1] == 116 && pixel[2] == 208) {
        return 136;
    }

    if (pixel[0] == 124 && pixel[1] == 140 && pixel[2] == 224) {
        return 138;
    }

    if (pixel[0] == 144 && pixel[1] == 164 && pixel[2] == 236) {
        return 140;
    }

    if (pixel[0] == 164 && pixel[1] == 184 && pixel[2] == 252) {
        return 142;
    }

    if (pixel[0] == 0 && pixel[1] == 24 && pixel[2] == 124) {
        return 144;
    }

    if (pixel[0] == 28 && pixel[1] == 56 && pixel[2] == 144) {
        return 146;
    }

    if (pixel[0] == 56 && pixel[1] == 84 && pixel[2] == 168) {
        return 148;
    }

    if (pixel[0] == 80 && pixel[1] == 112 && pixel[2] == 188) {
        return 150;
    }

    if (pixel[0] == 104 && pixel[1] == 136 && pixel[2] == 204) {
        return 152;
    }

    if (pixel[0] == 124 && pixel[1] == 156 && pixel[2] == 220) {
        return 154;
    }

    if (pixel[0] == 144 && pixel[1] == 180 && pixel[2] == 236) {
        return 156;
    }

    if (pixel[0] == 164 && pixel[1] == 200 && pixel[2] == 252) {
        return 158;
    }

    if (pixel[0] == 0 && pixel[1] == 44 && pixel[2] == 92) {
        return 160;
    }

    if (pixel[0] == 28 && pixel[1] == 76 && pixel[2] == 120) {
        return 162;
    }

    if (pixel[0] == 56 && pixel[1] == 104 && pixel[2] == 144) {
        return 164;
    }

    if (pixel[0] == 80 && pixel[1] == 132 && pixel[2] == 172) {
        return 166;
    }

    if (pixel[0] == 104 && pixel[1] == 156 && pixel[2] == 192) {
        return 168;
    }

    if (pixel[0] == 124 && pixel[1] == 180 && pixel[2] == 212) {
        return 170;
    }

    if (pixel[0] == 144 && pixel[1] == 204 && pixel[2] == 232) {
        return 172;
    }

    if (pixel[0] == 164 && pixel[1] == 224 && pixel[2] == 252) {
        return 174;
    }

    if (pixel[0] == 0 && pixel[1] == 60 && pixel[2] == 44) {
        return 176;
    }

    if (pixel[0] == 28 && pixel[1] == 92 && pixel[2] == 72) {
        return 178;
    }

    if (pixel[0] == 56 && pixel[1] == 124 && pixel[2] == 100) {
        return 180;
    }

    if (pixel[0] == 80 && pixel[1] == 156 && pixel[2] == 128) {
        return 182;
    }

    if (pixel[0] == 104 && pixel[1] == 180 && pixel[2] == 148) {
        return 184;
    }

    if (pixel[0] == 124 && pixel[1] == 208 && pixel[2] == 172) {
        return 186;
    }

    if (pixel[0] == 144 && pixel[1] == 228 && pixel[2] == 192) {
        return 188;
    }

    if (pixel[0] == 164 && pixel[1] == 252 && pixel[2] == 212) {
        return 190;
    }

    if (pixel[0] == 0 && pixel[1] == 60 && pixel[2] == 0) {
        return 192;
    }

    if (pixel[0] == 32 && pixel[1] == 92 && pixel[2] == 32) {
        return 194;
    }

    if (pixel[0] == 64 && pixel[1] == 124 && pixel[2] == 64) {
        return 196;
    }

    if (pixel[0] == 92 && pixel[1] == 156 && pixel[2] == 92) {
        return 198;
    }

    if (pixel[0] == 116 && pixel[1] == 180 && pixel[2] == 116) {
        return 200;
    }

    if (pixel[0] == 140 && pixel[1] == 208 && pixel[2] == 140) {
        return 202;
    }

    if (pixel[0] == 164 && pixel[1] == 228 && pixel[2] == 164) {
        return 204;
    }

    if (pixel[0] == 184 && pixel[1] == 252 && pixel[2] == 184) {
        return 206;
    }

    if (pixel[0] == 20 && pixel[1] == 56 && pixel[2] == 0) {
        return 208;
    }

    if (pixel[0] == 52 && pixel[1] == 92 && pixel[2] == 28) {
        return 210;
    }

    if (pixel[0] == 80 && pixel[1] == 124 && pixel[2] == 56) {
        return 212;
    }

    if (pixel[0] == 108 && pixel[1] == 152 && pixel[2] == 80) {
        return 214;
    }

    if (pixel[0] == 132 && pixel[1] == 180 && pixel[2] == 104) {
        return 216;
    }

    if (pixel[0] == 156 && pixel[1] == 204 && pixel[2] == 124) {
        return 218;
    }

    if (pixel[0] == 180 && pixel[1] == 228 && pixel[2] == 144) {
        return 220;
    }

    if (pixel[0] == 200 && pixel[1] == 252 && pixel[2] == 164) {
        return 222;
    }

    if (pixel[0] == 44 && pixel[1] == 48 && pixel[2] == 0) {
        return 224;
    }

    if (pixel[0] == 76 && pixel[1] == 80 && pixel[2] == 28) {
        return 226;
    }

    if (pixel[0] == 104 && pixel[1] == 112 && pixel[2] == 52) {
        return 228;
    }

    if (pixel[0] == 132 && pixel[1] == 140 && pixel[2] == 76) {
        return 230;
    }

    if (pixel[0] == 156 && pixel[1] == 168 && pixel[2] == 100) {
        return 232;
    }

    if (pixel[0] == 180 && pixel[1] == 192 && pixel[2] == 120) {
        return 234;
    }

    if (pixel[0] == 204 && pixel[1] == 212 && pixel[2] == 136) {
        return 236;
    }

    if (pixel[0] == 224 && pixel[1] == 236 && pixel[2] == 156) {
        return 238;
    }

    if (pixel[0] == 68 && pixel[1] == 40 && pixel[2] == 0) {
        return 240;
    }

    if (pixel[0] == 100 && pixel[1] == 72 && pixel[2] == 24) {
        return 242;
    }

    if (pixel[0] == 132 && pixel[1] == 104 && pixel[2] == 48) {
        return 244;
    }

    if (pixel[0] == 160 && pixel[1] == 132 && pixel[2] == 68) {
        return 246;
    }

    if (pixel[0] == 184 && pixel[1] == 156 && pixel[2] == 88) {
        return 248;
    }

    if (pixel[0] == 208 && pixel[1] == 180 && pixel[2] == 108) {
        return 250;
    }

    if (pixel[0] == 232 && pixel[1] == 204 && pixel[2] == 124) {
        return 252;
    }

    if (pixel[0] == 252 && pixel[1] == 224 && pixel[2] == 140) {
        return 254;
    }

    return 0;
} /* }}} */

static inline void process_png_file(pixel_t *screen, png_data_t data) {
    size_t offset = 0;
    for (register int y = 0; y < data.height; y++) {
        png_bytep row = data.row_pointers[y];
        for (register int x = 0; x < data.width; x++) {
            png_bytep px = &(row[x * 4]);
            pixel_t pixel = rgb_to_ntsc_index(px);
            screen[offset++] = pixel;
        }
    }
}

static inline void free_png_data(png_data_t data) {
    if (data.row_pointers) {
        if (data.row_pointers[0]) {
            for (int y = 0; y < data.height; y++) {
                free(data.row_pointers[y]);
            }
        }
        free(data.row_pointers);
    }
}

inline void load_screen(std::string path, pixel_t screen[]) {
    png_data_t data = read_png_file(path.c_str());
    assert(data.width == WIDTH);
    assert(data.height == HEIGHT);
    process_png_file(screen, data);
    free_png_data(data);
}

void usage(char *name) {
    printf("usage: %s /path/to/root /path/to/hdf5.h5\n", name);
}

static inline int path_to_number(const char *path) {
    int ret;
    register int start = 0, end = strlen(path);
    if (path[end-1] == '/') {
        end -= 1;
    }
    for (start = end; path[start] != '/' && start >= 0; start--);
    if (path[start] != '/') {
        printf("Found invalid path %s", path);
        abort();
    }
    end--;
    start++;
    int len = end - start + 1;
    char *tmp = (char *) malloc(sizeof(char) * len);
    strncpy(tmp, path + start, len);
    tmp[len] = 0;
    ret = atoi(tmp);
    free(tmp);
    return ret;
}

static inline bool numeric_file_name_comparison(std::string a, std::string b) {
    return atoi(a.c_str()) < atoi(b.c_str());
}

inline bool path_exists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

inline static char *get_line(char *str, size_t strsize, FILE *fp) {
    register int c;
    register size_t offset = 0;
    while (((c = fgetc(fp)) != EOF) && offset < strsize - 1) {
        str[offset++] = c;
        if (c == '\n') {
            break;
        }
    }
    str[offset] = '\0';
    if (c == EOF) {
        return NULL;
    } else {
        return str;
    }
}

inline std::vector<agcd_frame_t> read_events(const std::string &base_path) {
    char c;
    char buffer[512];
    FILE *fp = fopen(base_path.c_str(), "r");
    /* skip 2 lines {{{ */
    get_line(buffer, 512, fp);
    get_line(buffer, 512, fp);
    /* }}} */
    std::string last_accessible_path;
    std::vector<agcd_frame_t> frames;

    int i;
    size_t j = 0;
    bool keep_processing = true;
    while (keep_processing && get_line(buffer, 512, fp) != NULL) {
        char tmp[16];
        agcd_frame_t current;
        i = sscanf(buffer, "%d,%d, %d, %s %d\n",
               &current.frame,
               &current.reward,
               &current.score,
               &tmp,
               &current.action
        );
        if (i == EOF) {
            continue;
        }
        if (strncmp(tmp, "True", 4) == 0) {
            current.terminal = true;
        } else {
            current.terminal = false;
        }
        frames.push_back(current);
    }
    fclose(fp);

    return frames;
}



static inline std::vector<std::string> agcd_listdir(const char *path, bool add_prefix=false, bool sort=false) {
    DIR *dir;
    struct dirent *ent;
    std::string prefix(path);
    std::vector<std::string> ret;
    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            std::string filename(ent->d_name);
            ret.push_back(add_prefix ? prefix + "/" + filename : filename);
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
    }

    if (sort) {
        std::sort(ret.begin(), ret.end(), numeric_file_name_comparison);
    }

    return ret;
}

static herr_t write_dataset(hid_t loc_id, const char *dset_name, int rank,
        const hsize_t *dims, hid_t tid, const void *data) {

    bool error = false;
    hid_t did = -1, sid = -1;

    if (dset_name == NULL) {
        return -1;
    }

    if((sid = H5Screate_simple(rank, dims, NULL)) < 0) {
        return -1;
    }

    hid_t plist_id = H5Pcreate(H5P_DATASET_CREATE);
    if (!error && H5Pset_chunk(plist_id, rank, dims) < 0) {
        error = true;
    }

    if (!error && H5Pset_deflate(plist_id, 3)) {
        error = true;
    }

    if (!error && (did = H5Dcreate2(loc_id, dset_name, tid, sid, H5P_DEFAULT, plist_id, H5P_DEFAULT)) < 0) {
        error = true;
    }

    if (data) {
        if (!error && H5Dwrite(did, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0) {
            error = true;
        }
    }

    if (error) {
        H5E_BEGIN_TRY {
            H5Dclose(did);
            H5Sclose(sid);
        } H5E_END_TRY;
        return -1;
    }

    if(H5Dclose(did) < 0) {
        return -1;
    }

    if(H5Sclose(sid) < 0) {
        return -1;
    }

    return 0;
}


static inline int create_dataset(const std::string &game, const std::string &trajectory, const std::vector<std::string> &screens, std::vector<agcd_frame_t> events, hid_t screen_group, const hid_t event_group) {
    pixel_t *buffer = (pixel_t *) malloc(sizeof(pixel_t) * WIDTH * HEIGHT * screens.size());
    pixel_t *p = buffer;
    std::string prefix = "screens/" + game + "/" + trajectory + "/";
    for (size_t i = 0; i < screens.size(); i++, p += (WIDTH * HEIGHT)) {
        load_screen((prefix + screens[i]).c_str(), p);
    }

    const char *trajectory_str = trajectory.c_str();
    hsize_t dims[3] = {HEIGHT, WIDTH, screens.size()};
    herr_t status = write_dataset(screen_group, trajectory_str, 3, dims, H5T_NATIVE_UCHAR, buffer);

    dims[1] = 5;
    dims[0] = screens.size();
    agcd_frame_t *frames = &events[0];
    status = write_dataset(event_group, trajectory_str, 2, dims, H5T_NATIVE_INT, frames);

    free(buffer);
}

static inline int create_datasets(const std::string &game, const std::vector<std::string> &trajectories, const hid_t screen_group, const hid_t event_group) {
    for (size_t i = 0; i < trajectories.size(); i++) {
        std::vector<std::string> screens = agcd_listdir(("screens/" + game + "/" + trajectories[i]).c_str(), false, true);
        std::vector<agcd_frame_t> events = read_events(("trajectories/" + game + "/" + trajectories[i] + ".txt").c_str());

        if (screens.size() != events.size()) {
            fprintf(stderr, "Ignoring events %s. Screens and events have different numbers of frames.\n", trajectories[i].c_str());
            continue;
        }

        create_dataset(game, trajectories[i], screens, events, screen_group, event_group);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv[0]);
        exit(1);
    }

    if (chdir(argv[1]) != 0) {
        perror("Unable to process dataset. ");
        exit(1);
    }
    if (!path_exists("trajectories") || !path_exists("screens")) {
        printf("This doesn't seem like a valid AGCD dataset. Aborting.\n");
        exit(1);
    }
    if (path_exists(argv[2])) {
        printf("Will not overwrite existing file %s. Aborting.\n", argv[2]);
        exit(1);
    }

    std::vector<std::string> games = agcd_listdir("screens");

    hid_t file_id = H5Fcreate(argv[2], H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);

    hsize_t palette_dims[2] = {256, 3};
    write_dataset(file_id, "/palette", 2, palette_dims, H5T_NATIVE_UCHAR, NTSC_palette);

    for (int i = 0; i < games.size(); i++) {
        hid_t group_id = H5Gcreate(file_id, ("/" + games[i]).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        hid_t event_id = H5Gcreate(group_id, "trajectories", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        hid_t screen_id = H5Gcreate(group_id, "screens", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        create_datasets(games[i], agcd_listdir(("screens/" + games[i]).c_str(), false, true), screen_id, event_id);

        H5Gclose(group_id);
        H5Gclose(event_id);
        H5Gclose(screen_id);
    }

    H5Fclose(file_id);
}
