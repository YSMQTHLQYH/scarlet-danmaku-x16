// This file is yoinked from a different game of mine made in SDL lmao
// Changed a bit ofc so it reads X16 joystick instead of PC keyboard
#ifndef INPUT_ACTION_H
#define INPUT_ACTION_H
//#include <SDL3/SDL_keyboard.h>
#include <stdint.h>

//  ---- raw joystick read
//(didn't want to make a separate file for just this)

// how many joysticks we are scanning (joystick 0 is keyboard, so if this is 2 we are only scanning keyboard and joystick 1)
#define JOYSTICK_SCAN_COUNT	2
typedef struct {
	uint8_t mask_0;
	uint8_t mask_1;
}_sJoystickRaw;
extern _sJoystickRaw joystick_raw[JOYSTICK_SCAN_COUNT];
extern uint8_t joysticks_present;


//  ---- actual input action part
typedef struct {
	uint8_t keybind_mask_0;
	uint8_t keybind_mask_1;
	uint8_t pressed : 1;
	uint8_t just_pressed : 1;
	uint8_t joystick_mapped : 3;

}_sInputAction;

typedef enum {
	ACTION_UP,
	ACTION_DOWN,
	ACTION_LEFT,
	ACTION_RIGHT,
	ACTION_START,


	//not actually an input, basically a macro for number of IDs
	//define more inputs above this
	ACTION_ID_COUNT
}_eInputActionId;

extern _sInputAction action[ACTION_ID_COUNT];
// this could be done at compile time with c99, sigh :/
void InputActionInit(uint8_t joystick_n);

//call from main loop
void HandleInputActions();

// if key is currently pressed
uint8_t IsActionPressed(_eInputActionId);

//if key was pressed between last frame and this one
uint8_t IsActionJustPressed(_eInputActionId id);

//  ---- draw pressed buttons on bitmap layer
void JoystickDrawToBitmap(uint8_t joystick_n, uint8_t buffer_n, uint8_t x, uint8_t y);




#endif