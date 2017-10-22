#include "ale_interface.hpp"

#include <iostream>
#include <utility>

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

static void split_rom_game_path(const std::string &rom_file, std::string &h5file, std::string &game) {
    for (size_t i = 1; i < rom_file.size(); i++) {
        if (rom_file[i] == path_separator) {
            if (file_exists(rom_file.substr(0, i))) {
                // We have a file. Assume the rest is the game name
                h5file = rom_file.substr(0, i);
                if (rom_file.size()-1 > i+1) {
                    game = rom_file.substr(i+1, rom_file.size()-1);
                } else {
                    game = "";
                }
                return;
            }
        }
    }
    // If we're here, no substrings are valid paths...
    h5file = rom_file;
    game = "";
}

reward_t ALEInterface::act(Action action) {
    reward_t reward = 0;
    for (size_t i = 0; i < frame_skip; i++) {
        reward += atariState->getNextReward();
        atariState->step();
    }
    if (displayScreen) {
        displayScreen->display_screen();
    }
    return reward;
}

ALEInterface::~ALEInterface() {
    if (atariState != NULL) {
        delete atariState;
    }
    if (displayScreen != NULL) {
        delete displayScreen;
    }
    if (h5Wrapper != NULL) {
        delete h5Wrapper;
    }
}

ALEInterface::ALEInterface() {
    theSettings.reset(new Settings);
    setInt("frame_skip", 1);
}

ALEInterface::ALEInterface(bool display_screen) : display_screen(display_screen) {
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
        int ret = atariState->getNextAction();
        printf("Action: %d - Minimal: %d\n", ret, minimalActionCache[ret]);
        if (minimalActionCache[ret]) {
            return ret;
        } else {
            return PLAYER_A_NOOP;
        }
    }
    assert(theSettings.get());
    return theSettings->getInt(key);
}

bool ALEInterface::getBool(const std::string& key) {
    if (key == "loadedLast") {
        if (atariState != NULL) {
            return atariState->hasLoadedLastEpisode();
        } else {
            return true;
        }
    }
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
    if (key == "sequential_processing") {
        sequential = true;
        return;
    }
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
    if (h5Wrapper != NULL) {
        delete h5Wrapper;
    }

    split_rom_game_path(rom_file, romPath, gameName);
    h5Wrapper = new H5Wrapper(romPath.c_str());

    current_episode = 0;
    if (sequential) {
        atariState = new AtariState(romPath, gameName, getBool("color_averaging"), *h5Wrapper, phosphor, 0);
    } else {
        atariState = new AtariState(romPath, gameName, getBool("color_averaging"), *h5Wrapper, phosphor);
    }

    memset(&minimalActionCache, 0, sizeof(minimalActionCache));
    minimalActions.clear();
    allActions.clear();

    frame_skip = getInt("frame_skip");
    if (frame_skip < 1) {
        frame_skip = 1;
    }

    for (int i = 0 ; i < PLAYER_B_NOOP ; i++) {
        allActions.push_back((Action) i);
    }

    if (gameName == "mspacman") {
        printf("Loading Ms. PacMan actions...\n");
        minimalActions = ActionVect(std::begin(MS_PACMAN_MINIMAL), std::end
                (MS_PACMAN_MINIMAL));
    } else if (gameName == "pinball") {
        printf("Loading Pinball actions...\n");
        minimalActions = ActionVect(std::begin(PINBALL_MINIMAL), std::end
                (PINBALL_MINIMAL));
    } else if (gameName == "qbert") {
        printf("Loading QBert actions...\n");
        minimalActions = ActionVect(std::begin(Q_BERT_MINIMAL), std::end
                (Q_BERT_MINIMAL));
    } else if (gameName == "revenge") {
        printf("Loading Montezuma's Revenge actions...\n");
        minimalActions = ActionVect(std::begin(REVENGE_MINIMAL), std::end
                (REVENGE_MINIMAL));
    } else if (gameName == "spaceinvaders") {
        printf("Loading Space Invaders actions...\n");
        minimalActions = ActionVect(std::begin(SPACE_INVADERS_MINIMAL),
                       std::end(SPACE_INVADERS_MINIMAL));
    } else {
        printf("Unknown ROM found. Loading default actions...\n");
        minimalActions = allActions;
    }

    for (register size_t i = 0; i < minimalActions.size(); i++) {
        // Fill our cache for computing minimal actions
        minimalActionCache[minimalActions[i]] = true;
    }

    setString("rom_file", rom_file);

    if (display_screen) {
        palette.setPalette("standard", "NTSC");
        displayScreen = new DisplayScreen(atariState, palette);
    }
}

bool ALEInterface::game_over() const {
    if (atariState == NULL)
        return false;
    return atariState->isTerminal() || atariState->hasLoadedLastEpisode();
}

void ALEInterface::reset_game() {
    if (romPath.size() > 0) {
        if (atariState != NULL) {
            if (atariState->hasLoadedLastEpisode()) {
                // We want the agent to stop now
                return;
            }
            delete atariState;
        }
        if (sequential) {
            atariState = new AtariState(romPath, gameName, getBool("color_averaging"), *h5Wrapper, phosphor, ++current_episode);
        } else {
            atariState = new AtariState(romPath, gameName, getBool("color_averaging"), *h5Wrapper, phosphor);
        }
        if (displayScreen != NULL) {
            delete displayScreen;
            displayScreen = new DisplayScreen(atariState, palette);
        }
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
