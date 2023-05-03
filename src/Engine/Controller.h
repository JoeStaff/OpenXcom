#pragma once

#include <SDL.h>
#include <map>
#include "Game.h"

#ifndef MAX_CONTROLLERS
    #define MAX_CONTROLLERS 16
#endif

namespace OpenXcom
{


namespace Controller
{
    // Enum for joystick button labels
    enum class JoystickButton
    {
        SOUTH,  //A     B
        EAST,   //B     A
        WEST,   //X     Y
        NORTH,  //Y     X
        Start,
        Select,
        L1,
        R1,
        L2,
        R2,
        L3,
        R3,
        Volume_Up,
        Volume_Down,
        Function,
        Power,
        Left_Stick_H,
        Left_Stick_V,
        Right_Stick_H,
        Right_Stick_V
    };
    using ButtonMap = std::map<int, JoystickButton>;
    using AxisMap = std::map<int, JoystickButton>;
    // Structure to hold a joystick and its button map
    struct JoystickData
    {
        SDL_Joystick* joystick;
        bool dpadMouse;
        bool L1Function;
        bool R1Function;
        ButtonMap buttonMap;
        AxisMap axisMap;
    };

    void Initiate(Game *game);
    void ConnectControllers();
    void Joy2Key(SDL_Event &ev);
    void MoveMouse(int FPS);
    void SimulateKeyPress(bool down, SDLKey key);
    void SimulateMousePress(bool down, int key);
    void dpadFunctionPressKey(SDL_Event ev, int hatDirection, SDLKey key, bool &function);
    void MapButtons(int joystick);
    std::string TextEditPressed(std::string textBox);
    std::string getRandomLineFromFile(const std::string& filename);
}

}