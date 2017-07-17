#include "ale_interface.hpp"

#include <iostream>

reward_t ALEInterface::act(Action action) {
    reward_t reward = atariState->getNextReward();
    atariState->step();
    return reward;
}

ALEInterface::~ALEInterface() {
    if (atariState != NULL) {
        delete atariState;
    }
}

ALEInterface::ALEInterface() {
    theSettings.reset(new Settings);
    for (int i = 0 ; i < PLAYER_B_NOOP ; i++)
        actions.push_back((Action)i);
}

ALEInterface::ALEInterface(bool display_screen) {
    theSettings.reset(new Settings);
    for (int i = 0 ; i < PLAYER_B_NOOP ; i++)
        actions.push_back((Action)i);
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
    atariState = new AtariState(rom_file);
    romPath = rom_file;
}

bool ALEInterface::game_over() const {
    if (atariState == NULL)
        return false;
    return atariState->isTerminal();
}

void ALEInterface::reset_game() {
    if (romPath.size() > 0) {
        atariState = new AtariState(romPath);
    }
}

ActionVect ALEInterface::getLegalActionSet() {
    return actions;
}

ActionVect ALEInterface::getMinimalActionSet() {
    return actions;
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
    return std::__cxx11::string();
}

void ALEInterface::disableBufferedIO() {

}
