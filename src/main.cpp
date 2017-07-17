//
// Created by renatoc on 7/12/17.
//

#include "ale_interface.hpp"

int main(int argc, char **argv) {
    ALEInterface ale;
    ale.loadROM("/mnt/main/atari-grand-challenge/mspacman/trajectories/1"
                        ".txt");
    printf("action: %d\n", ale.getInt("current_action"));
    ale.getScreen();
    printf("Got screen!\n");
}