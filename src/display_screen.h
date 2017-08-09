/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare, 
 *   Matthew Hausknecht, and the Reinforcement Learning and Artificial Intelligence 
 *   Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  diplay_screen.cpp 
 *
 *  Supports displaying the screen via SDL. 
 **************************************************************************** */

#ifndef DISPLAY_SCREEN_H
#define DISPLAY_SCREEN_H

#include <stdio.h>
#include <stdlib.h>

#include "agcd_interface.hpp"
#include "Constants.h"
#include "ColourPalette.hpp"

typedef uint32_t	Uint32;

class DisplayScreen {
public:
    DisplayScreen(AtariState *atariState, ColourPalette &palette); 
    virtual ~DisplayScreen();

    // Displays the current frame buffer from the mediasource.
    void display_screen();

    // Has the user engaged manual control mode?
    bool manual_control_engaged() { return manual_control_active; }

    // Captures the keypress of a user in manual control mode.
    Action getUserAction();

protected:
    // Checks for SDL events.
    void poll();

    // Handle the SDL_Event.
    void handleSDLEvent(const void *event);

protected:
    // Dimensions of the SDL window (4:3 aspect ratio)
    static const int window_height = 321;
    static const int window_width = 428;
    // Maintains the paused/unpaused state of the game
    bool manual_control_active;
    AtariState* atariState;
    ColourPalette &colour_palette;
    int screen_height, screen_width;
    void *screen, *image;
    float yratio, xratio;
    Uint32 delay_msec;
    // Used to calibrate delay between frames
    Uint32 last_frame_time;
};

#endif // DISPLAY_SCREEN
