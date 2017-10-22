#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdlib>
#include <cstdio>
#include <string.h>

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include  <random>
#include  <iterator>

#include "hdf5_wrapper.hpp"
#include "ale_interface.hpp"
#include "agcd_interface.hpp"

static const char* SCREENS = "screens";

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

AtariState::AtariState(const std::string &path, const std::string &game, bool
        average, H5Wrapper &h5Wrapper, PhosphorBlend &phosphor, int episodeIndex) :
        base_path(abspath(path)), current_frame(0),
        h5Wrapper(h5Wrapper), aleScreen(210, 160), phosphor(phosphor) {

    game_vector_pair_t trajectories = h5Wrapper.get_trajectories(game);
    auto trajectoryId = *select_randomly(trajectories.begin(), trajectories.end());

    if (episodeIndex >= 0) {
        if (episodeIndex >= trajectories.size() - 1) {
            episodeIndex = trajectories.size() - 1;
            printf("episodeIndex = %d, screens.size() = %d\n", episodeIndex, trajectories.size());
            loadedLast = true;
        }
        trajectoryId = trajectories[episodeIndex];
    }

    std::cout << "Reading episode " << trajectoryId.first
              << " with " << trajectoryId.second << " frames..." << std::endl;

    trajectory = h5Wrapper.get_trajectory(game, trajectoryId.first);

    if (average) {
        std::cout << "Performing color averaging...";
        screen_t previous = trajectory.first[0];
        for (size_t j = 1; j < trajectory.first.size(); j++) {
            screen_t tmp = trajectory.first[j];
            phosphor.process(trajectory.first[j], previous, tmp);
            previous = tmp;
        }
    }

    std::cout << " done! " << std::endl;
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

size_t AtariState::getCurrentFrame() {
    return current_frame;
}

Action AtariState::getCurrentAction() {
    return static_cast<Action>(trajectory.second[current_frame].action);
}

Action AtariState::getNextAction() {
    if (current_frame < trajectory.first.size() - 2) {
        return static_cast<Action>(trajectory.second[current_frame + 1].action);
    } else {
        return PLAYER_A_NOOP;
    }
}

reward_t AtariState::getNextReward() {
    return trajectory.second[current_frame].reward;
}

bool AtariState::isTerminal() {
    return current_frame == trajectory.first.size() - 1;
}

ALEScreen &AtariState::getScreen() {
    aleScreen.m_pixels = trajectory.first[current_frame];
    return aleScreen;
}

void AtariState::step() {
    if (current_frame < trajectory.first.size() - 1) {
        current_frame += 1;
    }
}
