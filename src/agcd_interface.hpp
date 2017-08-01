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

#include "ale_screen.hpp"
#include "Constants.h"

#define MAX_PATH_LENGTH 2048
#define MAX_BASE_LENGTH 256

class AtariState {

private:
    struct agcd_frame_t {
        unsigned frame;
        reward_t reward;
        int score;
        bool terminal;
        Action action;
        std::string frame_path;
        ALEScreen screen;
        agcd_frame_t() : screen(210, 160) {};
    };

    AtariState();
    std::string base_path ;
    char base_name[MAX_BASE_LENGTH];
    char screen_path_template[MAX_PATH_LENGTH];
    std::vector<agcd_frame_t> frames;
    size_t current_frame;
    inline void read_data(bool);
    std::vector<pixel_t> previousScreen;
    inline void loadScreen(size_t offset);
    bool loadedLast = false;

public:
    AtariState(const std::string &path, bool average, int episodeIndex=-1);
    ~AtariState() {
        frames.clear();
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
    return (stat (name.c_str(), &buffer) == 0);
}

#endif //ALE_ATARI_GRAND_CHALLENGE_ATARI_GRAND_CHALLENGE_INTERFACE_HPP
