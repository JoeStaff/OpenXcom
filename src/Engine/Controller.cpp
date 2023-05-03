#include "Controller.h"
#include "Logger.h"
#include "Game.h"
#include "Options.h"
#include <map>
#include <SDL.h>
#include <typeinfo>
#include <fstream>
#include <string>
#include <vector>
#include <random>

namespace OpenXcom
{

namespace Controller
{
    
    JoystickData joystickData[MAX_CONTROLLERS];
    std::map<JoystickButton, int> joy2keyMap;
    std::map<int, int> JoystickMap;
    std::map<int, bool> Joy2KeyPresses;
    float mouseDirectionX, mouseDirectionY, mouseSpeed, masterVolume;
    Game *_game;

    void Initiate(Game *game){
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0){
			Log(LOG_ERROR) << SDL_GetError();
			Log(LOG_WARNING) << "Error initializing Joysticks.";
		}
		SDL_JoystickEventState(SDL_ENABLE);
        ConnectControllers();
        mouseSpeed=5.0;
        _game=game;
    }

    void ConnectControllers(){
        int numJoysticks = SDL_NumJoysticks();
        if (numJoysticks == 0)
        {
            Log(LOG_INFO) << "No joysticks available";
            return;
        }
        int numConnectedJoysticks = 0;
        for (int i = 0; i < numJoysticks && numConnectedJoysticks < MAX_CONTROLLERS; i++)
        {
            SDL_Joystick* joystick = SDL_JoystickOpen(i);
            if (joystick == NULL)
            {
                Log(LOG_ERROR) << "Failed to open joystick " << i;
                continue;
            }

            Log(LOG_INFO) << "Joystick " << i << " Name: " << SDL_JoystickName(SDL_JoystickIndex(joystick));

            JoystickMap[SDL_JoystickIndex(joystick)] = numConnectedJoysticks;
            joystickData[numConnectedJoysticks].joystick = joystick;
            MapButtons(numConnectedJoysticks);
            numConnectedJoysticks++;
        }
    }

    void MapButtons(int joystick){
        int profile=0;
        std::string controller_name=SDL_JoystickName(SDL_JoystickIndex(joystickData[joystick].joystick));
        
        if( controller_name.compare("RG35XX Gamepad") == 0 )
            profile=1;

        switch(profile){
            case 0: //STANDARD CONTROLLER
                joystickData[joystick].dpadMouse=false;
                joystickData[joystick].buttonMap = {
                    {0, JoystickButton::SOUTH},
                    {1, JoystickButton::EAST},
                    {2, JoystickButton::WEST},
                    {3, JoystickButton::NORTH},
                    {4, JoystickButton::L1},
                    {5, JoystickButton::R1},
                    {6, JoystickButton::Select},
                    {7, JoystickButton::Start},
                    {8, JoystickButton::L3},
                    {9, JoystickButton::R3},
                    {10,JoystickButton::L2},
                    {11,JoystickButton::R2},
                    {12,JoystickButton::Function}
                };
                joystickData[joystick].axisMap = {
                    {0, JoystickButton::Left_Stick_H},
                    {1, JoystickButton::Left_Stick_V},
                    {2, JoystickButton::L2},
                    {3, JoystickButton::Right_Stick_H},
                    {4, JoystickButton::Right_Stick_V},
                    {5, JoystickButton::R2}
                };
                break;
            case 1: //RG35XX CONTROLLER
                Log(LOG_INFO) << "RG35xx Controller Found!";
                joystickData[joystick].dpadMouse=true;
                joystickData[joystick].buttonMap = {
                    {0, JoystickButton::EAST},
                    {1, JoystickButton::SOUTH},
                    {2, JoystickButton::NORTH},
                    {3, JoystickButton::WEST},
                    {4, JoystickButton::Power},
                    {5, JoystickButton::L1},
                    {6, JoystickButton::R1},
                    {7, JoystickButton::Select},
                    {8, JoystickButton::Start},
                    {9, JoystickButton::Function},
                    {10,JoystickButton::Volume_Up},
                    {11,JoystickButton::Volume_Down}
                };
                joystickData[joystick].axisMap = {
                    {0, JoystickButton::Left_Stick_H},
                    {1, JoystickButton::Left_Stick_V},
                    {2, JoystickButton::L2},
                    {3, JoystickButton::Right_Stick_H},
                    {4, JoystickButton::Right_Stick_V},
                    {5, JoystickButton::R2}
                };
                break;
        }
    }


    void Joy2Key(SDL_Event &ev){
        int x, y;
        switch(ev.type){
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                switch (joystickData[ev.jbutton.which].buttonMap[ev.jbutton.button]){
                    //Left Click
                    case JoystickButton::SOUTH:
                            if(ev.type==SDL_JOYBUTTONDOWN)SimulateMousePress(true, SDL_BUTTON_LEFT);
                            else SimulateMousePress(false, SDL_BUTTON_LEFT);
                        break;
                    //Right Click
                    case JoystickButton::EAST:
                            if(ev.type==SDL_JOYBUTTONDOWN)SimulateMousePress(true, SDL_BUTTON_RIGHT);
                            else SimulateMousePress(false, SDL_BUTTON_RIGHT);
                        break;
                    case JoystickButton::NORTH:
                            if(ev.type==SDL_JOYBUTTONDOWN)SimulateKeyPress(true, SDLK_BACKSPACE);
                            else SimulateKeyPress(false, SDLK_BACKSPACE);
                        break;
                    case JoystickButton::WEST:
                            if(ev.type==SDL_JOYBUTTONDOWN)SimulateKeyPress(true, SDLK_m);
                            else SimulateKeyPress(false, SDLK_m);
                        break;

                    //Start/Select = ENTER/ESCAPE
                    case JoystickButton::Start:
                            if(ev.type==SDL_JOYBUTTONDOWN)
                                SimulateKeyPress(true, SDLK_RETURN);
                            else
                                SimulateKeyPress(false, SDLK_RETURN);
                        break;
                    case JoystickButton::Select:
                            if(ev.type==SDL_JOYBUTTONDOWN)
                                SimulateKeyPress(true, SDLK_ESCAPE);
                            else
                                SimulateKeyPress(false, SDLK_ESCAPE);
                        break;

                    //Function Keys currently being used for controllers with DPADMouse controls
                    case JoystickButton::L1:
                        if(joystickData[ev.jhat.which].dpadMouse)
                        {
                            if(ev.type==SDL_JOYBUTTONDOWN)
                                joystickData[ev.jhat.which].L1Function=true;
                            else
                                joystickData[ev.jhat.which].L1Function=false;
                        }
                        break;
                    case JoystickButton::R1:
                        if(joystickData[ev.jhat.which].dpadMouse)
                        {
                            if(ev.type==SDL_JOYBUTTONDOWN)
                                joystickData[ev.jhat.which].R1Function=true;
                            else
                                joystickData[ev.jhat.which].R1Function=false;
                        }
                        break;

                    //Menu/Function key == Quit Game
                    case JoystickButton::Function:
                            if(joystickData[ev.jbutton.which].L1Function)
                                ev.type=SDL_QUIT;
                        break;

                    case JoystickButton::Volume_Up:
                        if(ev.type==SDL_JOYBUTTONDOWN){
                            if(Options::soundVolume+10>128 ||
                            Options::musicVolume+10>128 ||
                            Options::uiVolume+10>128)
                                break;
                            Options::soundVolume+=10;
                            Options::musicVolume+=10;
                            Options::uiVolume+=10;
                            _game->setVolume(Options::soundVolume, Options::musicVolume,Options::uiVolume);
                        }
                        break;
                    case JoystickButton::Volume_Down:
                        if(ev.type==SDL_JOYBUTTONDOWN){
                            if(Options::soundVolume-10<-10 &&
                            Options::musicVolume-10<-10 &&
                            Options::uiVolume-10<-10)
                                break;
                            Options::soundVolume-=10;
                            Options::musicVolume-=10;
                            Options::uiVolume-=10;
                            _game->setVolume(Options::soundVolume, Options::musicVolume,Options::uiVolume);
                        }
                        break;
                }
                break;
            case SDL_JOYHATMOTION:
                /*
                Controls:
                L1+DPAD == Arrow Keys
                R1+DPAD left/right == Left Shift and Tab
                R1+DPAD up/down == Page Up and Page Down
                DPAD == Mouse Movement
                */

                //UP=1, RIGHT=2, DOWN=4, LEFT=8

                //L1 Held, Up Pressed
                dpadFunctionPressKey(ev,1,SDLK_UP,joystickData[ev.jhat.which].L1Function);
                //L1 Held, Right Pressed
                dpadFunctionPressKey(ev,2,SDLK_RIGHT,joystickData[ev.jhat.which].L1Function);
                //L1 Held, Down Pressed
                dpadFunctionPressKey(ev,4,SDLK_DOWN,joystickData[ev.jhat.which].L1Function);
                //L1 Held, Left Pressed
                dpadFunctionPressKey(ev,8,SDLK_LEFT,joystickData[ev.jhat.which].L1Function);

                //R1 Held, Up Pressed
                dpadFunctionPressKey(ev,1,SDLK_PAGEUP,joystickData[ev.jhat.which].R1Function);
                //R1 Held, Right Pressed
                dpadFunctionPressKey(ev,2,SDLK_TAB,joystickData[ev.jhat.which].R1Function);
                //R1 Held, Down Pressed
                dpadFunctionPressKey(ev,4,SDLK_PAGEDOWN,joystickData[ev.jhat.which].R1Function);
                //R1 Held, Left Pressed
                dpadFunctionPressKey(ev,8,SDLK_LSHIFT,joystickData[ev.jhat.which].R1Function);
                
                //Neither L1 nor R1 are pressed
                if(!joystickData[ev.jhat.which].L1Function && !joystickData[ev.jhat.which].R1Function)
                {
                    if(ev.jhat.value&1)
                        mouseDirectionY=-1.0;
                    else if (ev.jhat.value&4)
                        mouseDirectionY=1.0;
                    else
                        mouseDirectionY=0;
                    if(ev.jhat.value&2)
                        mouseDirectionX=+1.0;
                    else if (ev.jhat.value&8)
                        mouseDirectionX=-1.0;
                    else
                        mouseDirectionX=0;
                }else{
                    mouseDirectionX=0;
                    mouseDirectionY=0;
                }
                break;
            case SDL_JOYAXISMOTION:
                switch (joystickData[ev.jaxis.which].axisMap[ev.jaxis.axis]){
                    case JoystickButton::L2:
                        if(ev.jaxis.value>16383){
                            SimulateKeyPress(true, SDLK_q);
                        }else if(ev.jaxis.value<-16383){
                            SimulateKeyPress(false, SDLK_q);
                        }
                        break;
                    case JoystickButton::R2:
                        if(ev.jaxis.value>16383){
                            SimulateKeyPress(true, SDLK_e);
                        }else if(ev.jaxis.value<-16383){
                            SimulateKeyPress(false, SDLK_e);
                        }
                        break;

                    case JoystickButton::Left_Stick_H:
                    case JoystickButton::Right_Stick_H:
                        mouseDirectionX=ev.jaxis.value/32768;
                        break;
                    case JoystickButton::Left_Stick_V:
                    case JoystickButton::Right_Stick_V:
                        mouseDirectionY=ev.jaxis.value/32768;
                        break;
                break;
                }
        }
    }

    SDL_Event tempEvent;

    void SimulateKeyPress(bool down, SDLKey key){
        tempEvent.type = (down)?SDL_KEYDOWN:SDL_KEYUP;
        //keyUpEvent.key.keysym.scancode = SDL_SCANCODE_PAGEUP; //SDL2 bullshit
        tempEvent.key.keysym.mod = KMOD_NONE;
        tempEvent.key.keysym.unicode = 0;
        tempEvent.key.keysym.sym = key;
        SDL_PushEvent(&tempEvent);
    }
    void SimulateMousePress(bool down, int key){
        tempEvent.type = (down)?SDL_MOUSEBUTTONDOWN:SDL_MOUSEBUTTONUP;
        tempEvent.button.button= key;
        int x,y;
        SDL_GetMouseState(&x,&y);
        tempEvent.button.x=x;
        tempEvent.button.y=y;
        SDL_PushEvent(&tempEvent);
    }

    
    void MoveMouse(int FPS){
        int x,y;
        SDL_GetMouseState(&x,&y);
        x+=static_cast<int>(mouseDirectionX*mouseSpeed*((double)Options::FPS/(double)FPS));
        y+=static_cast<int>(mouseDirectionY*mouseSpeed*((double)Options::FPS/(double)FPS));
        SDL_WarpMouse(x,y);
    }

    void dpadFunctionPressKey(SDL_Event ev, int hatDirection, SDLKey key, bool &function){
        if( ( ev.jhat.value & hatDirection ) && !Joy2KeyPresses[key] && function ){
            SimulateKeyPress(true, key);
            Joy2KeyPresses[key]=true;
        }else if (Joy2KeyPresses[key] && ( !( ev.jhat.value & hatDirection ) || !function )){
            SimulateKeyPress(false, key);
            Joy2KeyPresses[key]=false;
        }
    }

    std::string TextEditPressed(std::string textBox){

        std::string result;
        printf("Text Input: %s\n",textBox.c_str());
        char buffer[64];
        sprintf(buffer,"Unknown Text Field");
        if(textBox=="Save Name")
            sprintf(buffer, "Save %s",getRandomLineFromFile("SaveNamesList.txt").c_str());
        else if(textBox=="Base Name")
            sprintf(buffer, "Base %s %s",getRandomLineFromFile("BaseNamesList.txt").c_str(),getRandomLineFromFile("BaseNamesList.txt").c_str());

        result = buffer;
        return result;
    }

    std::string getRandomLineFromFile(const std::string& filename) {
        std::ifstream file(filename);
        std::vector<std::string> lines;
        std::string line;

        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, lines.size() - 1);
        int index = dist(rng);
        return lines[index];
    }

}

}