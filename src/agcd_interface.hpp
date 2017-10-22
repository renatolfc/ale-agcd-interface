//
// Created by renatoc on 7/11/17.
//

#ifndef ALE_ATARI_GRAND_CHALLENGE_ATARI_GRAND_CHALLENGE_INTERFACE_HPP
#define ALE_ATARI_GRAND_CHALLENGE_ATARI_GRAND_CHALLENGE_INTERFACE_HPP

#include <vector>
#include <algorithm>

#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>

#include "phosphor_blend.hpp"
#include "hdf5_wrapper.hpp"
#include "ale_screen.hpp"
#include "Constants.h"

#define MAX_PATH_LENGTH 2048
#define MAX_BASE_LENGTH 256

class AtariState {

private:
    AtariState();
    std::string base_path;
    char base_name[MAX_BASE_LENGTH];
    char screen_path_template[MAX_PATH_LENGTH];
    trajectory_t trajectory;
    size_t current_frame;
    std::vector<pixel_t> previousScreen;
    bool loadedLast = false;
    H5Wrapper &h5Wrapper;
    ALEScreen aleScreen;
    PhosphorBlend &phosphor;

public:
    AtariState(const std::string &path, const std::string &game, bool average, H5Wrapper &h5Wrapper, PhosphorBlend &phosphor, int episodeIndex=-1);
    ~AtariState() {
    }

    size_t getCurrentFrame();
    Action getCurrentAction();
    Action getNextAction();
    reward_t getNextReward();
    bool isTerminal();
    ALEScreen &getScreen();
    void step();
    bool hasLoadedLastEpisode() {
        return loadedLast;
    }
    int height() {
        return 210;
    }
    int width() {
        return 160;
    }
};

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
    return path_to_number(a.c_str()) < path_to_number(b.c_str());
}

static inline std::vector<std::string> agcd_listdir(const char *path) {
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

    std::sort(ret.begin(), ret.end(), numeric_file_name_comparison);

    return ret;
}

static inline bool file_exists(const std::string& name) {
    struct stat buffer;
    if (stat(name.c_str(), &buffer) == 0) {
        return S_ISREG(buffer.st_mode);
    }
    return false;
}

#endif //ALE_ATARI_GRAND_CHALLENGE_ATARI_GRAND_CHALLENGE_INTERFACE_HPP
