#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>

#include <png.h>

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>

#include  <random>
#include  <iterator>

#include "ale_interface.hpp"
#include "agcd_interface.hpp"

#ifdef WIN32
const char path_separator = '\\';
#else
const char path_separator = '/';
#endif

static const char* SCREENS = "screens";

typedef struct {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers;
} png_data_t;

typedef unsigned char pixel_t;

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

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

static inline int path_to_number(const char *path) {
    int ret;
    register int start = 0, end = strlen(path);
    for (; path[end] != '.' && end >= 0; end--);
    if (path[end] != '.') {
        printf("Found invalid path %s", path);
        abort();
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
    return path_to_number(a.c_str()) < path_to_number(b.c_str());
}

static inline void process_png_file(std::vector<pixel_t> &screen, png_data_t
data) {
    for (register int y = 0; y < data.height; y++) {
        png_bytep row = data.row_pointers[y];
        for (register int x = 0; x < data.width; x++) {
            png_bytep px = &(row[x * 4]);
            pixel_t pixel = rgb_to_ntsc_index(px);
            screen.push_back(pixel);
        }
    }
}

static inline std::vector<std::string> listdir(const char *path) {
    DIR *dir;
    struct dirent *ent;
    std::string prefix(path);
    std::vector<std::string> ret;
    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            std::string filename(ent->d_name);
            ret.push_back(prefix + "/" + filename);
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
    }

    return ret;
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

static inline bool file_exists(const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

static std::string abspath(const std::string &path) {
    if (path.find(path_separator) == 0) {
        return path;
    }
    char *cwd = get_current_dir_name();
    std::string ret = std::string(cwd) + path_separator + path;
    free(cwd);
    return ret;
}

static inline int episode_number(const std::string &path) {
    size_t offset;
    for (offset = path.size() - 1;
            offset != 0 && path[offset] != path_separator;
            offset--
        );
    if (!offset) {
        return -1;
    }
    return atoi(path.c_str() + offset + 1);
}

AtariState::AtariState(const std::string &path) : base_path(abspath(path)),
                                                  current_frame(0),
                                                  screen(210, 160)
{
    std::vector<std::string> screens;
    if (strstr(base_path.c_str(), SCREENS) == NULL) {
        screens = listdir((base_path + path_separator + SCREENS).c_str());
    } else {
        screens = listdir(base_path.c_str());
    }

    auto screen_path = *select_randomly(screens.begin(), screens.end());
    int episode = episode_number(screen_path);
    assert(episode >= 0);

    char episode_str[16];
    sprintf(episode_str, "%d", episode);

    if (strstr(base_path.c_str(), SCREENS) == NULL) {
        base_path = base_path + path_separator + "trajectories" +
                path_separator + episode_str + ".txt";
    }

    std::cout << "Reading episode " << base_path << std::endl;

    char *tmp = strdup(base_path.c_str());
    if (base_path.size() && tmp[base_path.size()-1] == '/') {
        tmp[base_path.size()-1] = '\0';
    }
    strcpy(base_name, basename(tmp));

    // Kill the file extension
    register size_t i;
    for (i = strlen(base_name) - 1; base_name[i] != '.' && i > 0; i--);
    base_name[i] = '\0';

    char *dir = dirname(tmp);
    snprintf(
            screen_path_template,
            MAX_PATH_LENGTH,
            "%s%c..%cscreens%c%s%c%s",
            dir, path_separator, path_separator, path_separator,
            base_name, path_separator, "%d.png"
    );

    free(tmp);

    read_data();
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

inline void AtariState::read_data() {
    char c;
    char buffer[512];
    FILE *fp = fopen(base_path.c_str(), "r");
    /* skip 2 lines {{{ */
    get_line(buffer, 512, fp);
    get_line(buffer, 512, fp);
    /* }}} */
    std::string last_accessible_path;

    int i;
    size_t j = 0;
    while (get_line(buffer, 512, fp) != NULL) {
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
        current.frame += 1;
        char screen_path[MAX_PATH_LENGTH];
        sprintf(screen_path, screen_path_template, current.frame);
        if (file_exists(screen_path)) {
            current.frame_path = screen_path;
            last_accessible_path = screen_path;
        } else {
            if (!last_accessible_path.empty()) {
                current.frame_path = last_accessible_path;
            } else {
                // This is the first file and it doesn't exist. Bummer.
                abort();
            }
        }
        frames.push_back(current);
        loadScreen(j++);
    }
    fclose(fp);
}

size_t AtariState::getCurrentFrame() {
    return current_frame;
}

Action AtariState::getCurrentAction() {
    return frames[current_frame].action;
}

Action AtariState::getNextAction() {
    if (current_frame < frames.size() - 2) {
        return frames[current_frame + 1].action;
    } else {
        return PLAYER_A_NOOP;
    }
}

reward_t AtariState::getNextReward() {
    return frames[current_frame].reward;
}

bool AtariState::isTerminal() {
    return current_frame == frames.size() - 1;
}

ALEScreen &AtariState::getScreen() {
    return screen;
}

inline void AtariState::loadScreen(size_t offset) {
    std::vector<pixel_t> screen;
    png_data_t data = read_png_file(frames[offset].frame_path.c_str());
    process_png_file(screen, data);
    this->screen.m_pixels = screen;
    free_png_data(data);
}

void AtariState::step() {
    if (current_frame < frames.size() - 1) {
        current_frame += 1;
    }
}
