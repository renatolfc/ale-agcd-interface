#include "ale_interface.hpp"

#include <iostream>

const Action SPACE_INVADERS_MINIMAL[] = {
        PLAYER_A_NOOP, PLAYER_A_LEFT, PLAYER_A_RIGHT, PLAYER_A_FIRE,
        PLAYER_A_LEFTFIRE, PLAYER_A_RIGHTFIRE
};

const Action MS_PACMAN_MINIMAL[] = {
        PLAYER_A_NOOP, PLAYER_A_UP, PLAYER_A_RIGHT, PLAYER_A_LEFT,
        PLAYER_A_DOWN, PLAYER_A_UPRIGHT, PLAYER_A_UPLEFT, PLAYER_A_DOWNRIGHT,
        PLAYER_A_DOWNLEFT
};

const Action Q_BERT_MINIMAL[] = {
        PLAYER_A_NOOP, PLAYER_A_FIRE, PLAYER_A_UP, PLAYER_A_RIGHT,
        PLAYER_A_LEFT, PLAYER_A_DOWN
};

const Action REVENGE_MINIMAL[] = {
        PLAYER_A_NOOP, PLAYER_A_FIRE, PLAYER_A_UP, PLAYER_A_RIGHT,
        PLAYER_A_LEFT, PLAYER_A_DOWN, PLAYER_A_UPRIGHT, PLAYER_A_UPLEFT,
        PLAYER_A_DOWNRIGHT, PLAYER_A_DOWNLEFT, PLAYER_A_UPFIRE,
        PLAYER_A_RIGHTFIRE, PLAYER_A_LEFTFIRE, PLAYER_A_DOWNFIRE,
        PLAYER_A_UPRIGHTFIRE, PLAYER_A_UPLEFTFIRE, PLAYER_A_DOWNRIGHTFIRE,
        PLAYER_A_DOWNLEFTFIRE,
};

const Action PINBALL_MINIMAL[] = {
        PLAYER_A_NOOP, PLAYER_A_FIRE, PLAYER_A_UP, PLAYER_A_RIGHT,
        PLAYER_A_LEFT, PLAYER_A_DOWN, PLAYER_A_UPFIRE, PLAYER_A_RIGHTFIRE,
        PLAYER_A_LEFTFIRE,
};

reward_t ALEInterface::act(Action action) {
    reward_t reward = 0;
    for (size_t i = 0; i < frame_skip; i++) {
        reward += atariState->getNextReward();
        atariState->step();
    }
    return reward;
}

ALEInterface::~ALEInterface() {
    if (atariState != NULL) {
        delete atariState;
    }
}

ALEInterface::ALEInterface() {
    theSettings.reset(new Settings);
    setInt("frame_skip", 1);
}

ALEInterface::ALEInterface(bool display_screen) {
    theSettings.reset(new Settings);
}

// Get the value of a setting.
std::string ALEInterface::getString(const std::string& key) {
    assert(theSettings.get());
    return theSettings->getString(key);
}

int ALEInterface::getInt(const std::string& key) {
    if (key == "current_action" && atariState != NULL) {
        return atariState->getCurrentAction();
    }
    if (key == "next_action" && atariState != NULL) {
        return atariState->getNextAction();
    }
    assert(theSettings.get());
    return theSettings->getInt(key);
}

bool ALEInterface::getBool(const std::string& key) {
    assert(theSettings.get());
    return theSettings->getBool(key);
}

float ALEInterface::getFloat(const std::string& key) {
    assert(theSettings.get());
    return theSettings->getFloat(key);
}

void ALEInterface::setString(const std::string& key, const std::string& value) {
    assert(theSettings.get());
    theSettings->setString(key, value);
    theSettings->validate();
}

void ALEInterface::setInt(const std::string& key, const int value) {
    assert(theSettings.get());
    theSettings->setInt(key, value);
    theSettings->validate();
}

void ALEInterface::setBool(const std::string& key, const bool value) {
    assert(theSettings.get());
    theSettings->setBool(key, value);
    theSettings->validate();
}

void ALEInterface::setFloat(const std::string& key, const float value) {
    assert(theSettings.get());
    theSettings->setFloat(key, value);
    theSettings->validate();
}

/*
 * Here we override ALE's loading code.
 * What we really want to load is the path to the trace files.
 * We follow the convention used in the Atari Grand Challenge Dataset.
 * So if, for example, we want to load mspacman's first trace, we would so
 * like this:
 * loadROM("/path/to/mspacman/trajectories/1.txt"). The code also assumes the
 * screens will be located at "/path/to/mspacman/screens/1/*". As a
 * reference, this is the tree during development:
 *
 * $ tree -L 2
 * .
 * ├── mspacman
 * │   ├── screens
 * │   └── trajectories
 * ├── pinball
 * │   ├── screens
 * │   └── trajectories
 * ├── qbert
 * │   ├── screens
 * │   └── trajectories
 * ├── revenge
 * │   ├── screens
 * │   └── trajectories
 * └── spaceinvaders
 *     ├── screens
 *     └── trajectories
 */
void ALEInterface::loadROM(std::string rom_file) {
    if (atariState != NULL) {
        delete atariState;
    }

    atariState = new AtariState(rom_file, getBool("color_averaging"));
    romPath = rom_file;
    minimalActions.clear();
    allActions.clear();

    frame_skip = getInt("frame_skip");
    if (frame_skip < 1) {
        frame_skip = 1;
    }

    for (int i = 0 ; i < PLAYER_B_NOOP ; i++) {
        allActions.push_back((Action) i);
    }

    if (rom_file.find("mspacman") != std::string::npos) {
        printf("Loading Ms. PacMan actions...\n");
        minimalActions = ActionVect(std::begin(MS_PACMAN_MINIMAL), std::end
                (MS_PACMAN_MINIMAL));
    } else if (rom_file.find("pinball") != std::string::npos) {
        printf("Loading Pinball actions...\n");
        minimalActions = ActionVect(std::begin(PINBALL_MINIMAL), std::end
                (PINBALL_MINIMAL));
    } else if (rom_file.find("qbert") != std::string::npos) {
        printf("Loading QBert actions...\n");
        minimalActions = ActionVect(std::begin(Q_BERT_MINIMAL), std::end
                (Q_BERT_MINIMAL));
    } else if (rom_file.find("revenge") != std::string::npos) {
        printf("Loading Montezuma's Revenge actions...\n");
        minimalActions = ActionVect(std::begin(REVENGE_MINIMAL), std::end
                (REVENGE_MINIMAL));
    } else if (rom_file.find("spaceinvaders") != std::string::npos) {
        printf("Loading Space Invaders actions...\n");
        minimalActions = ActionVect(std::begin(SPACE_INVADERS_MINIMAL),
                       std::end(SPACE_INVADERS_MINIMAL));
    } else {
        printf("Unknown ROM found. Loading default actions...\n");
        minimalActions = allActions;
    }
}

bool ALEInterface::game_over() const {
    if (atariState == NULL)
        return false;
    return atariState->isTerminal();
}

void ALEInterface::reset_game() {
    if (romPath.size() > 0) {
        if (atariState != NULL) {
            delete atariState;
        }
        atariState = new AtariState(romPath, getBool("color_averaging"));
    }
}

ActionVect ALEInterface::getLegalActionSet() {
    return allActions;
}

ActionVect ALEInterface::getMinimalActionSet() {
    return minimalActions;
}

int ALEInterface::getFrameNumber() {
    return atariState->getCurrentFrame();
}

int ALEInterface::getEpisodeFrameNumber() const {
    return atariState->getCurrentFrame();
}

const ALEScreen &ALEInterface::getScreen() {
    return atariState->getScreen();
}

void ALEInterface::getScreenGrayscale(std::vector<unsigned char> &grayscale_output_buffer) {

}

void ALEInterface::getScreenRGB(std::vector<unsigned char> &output_rgb_buffer) {

}

const ALERAM &ALEInterface::getRAM() {
    return fakeRam;
}

void ALEInterface::saveState() {

}

void ALEInterface::loadState() {

}

ALEState ALEInterface::cloneState() {
    return ALEState();
}

void ALEInterface::restoreState(const ALEState &state) {

}

ALEState ALEInterface::cloneSystemState() {
    return ALEState();
}

void ALEInterface::restoreSystemState(const ALEState &state) {

}

void ALEInterface::saveScreenPNG(const std::string &filename) {

}

std::string ALEInterface::welcomeMessage() {
    return "";
}

void ALEInterface::disableBufferedIO() {

}
