//
// Created by renatoc on 7/11/17.
//

#ifndef ALE_ATARI_GRAND_CHALLENGE_ATARI_GRAND_CHALLENGE_INTERFACE_HPP
#define ALE_ATARI_GRAND_CHALLENGE_ATARI_GRAND_CHALLENGE_INTERFACE_HPP

#include <vector>
#include "ale_screen.hpp"
#include "Constants.h"

#define MAX_PATH_LENGTH 2048
#define MAX_BASE_LENGTH 256

class AtariState {

private:
    typedef struct {
        unsigned frame;
        reward_t reward;
        int score;
        bool terminal;
        Action action;
        std::string frame_path;
    } agcd_frame_t;

    AtariState();
    std::string base_path ;
    char base_name[MAX_BASE_LENGTH];
    char screen_path_template[MAX_PATH_LENGTH];
    std::vector<agcd_frame_t> frames;
    size_t current_frame;
    ALEScreen screen;
    inline void read_data();
    inline void loadScreen(size_t offset);

public:
    AtariState(const std::string &path);
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
};

#endif //ALE_ATARI_GRAND_CHALLENGE_ATARI_GRAND_CHALLENGE_INTERFACE_HPP
