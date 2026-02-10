#include "input_action.h"
#include "x16.h"
#include "zp_utils.h"
#include "bitmap_layer.h"

//  ---- joystick raw read
_sJoystickRaw joystick_raw[JOYSTICK_SCAN_COUNT] = { 0 };
uint8_t joystick_present = 0; // mask of joysticks connected, bit 0 for joystick 0, bit 1 for joystick 1, ...

static void JoysticksReadRaw() {
	uint8_t i, p, m0, m1;
	joystick_present = 0;
	for (i = 0; i < JOYSTICK_SCAN_COUNT; i++) {
		p = KernalJoystickGet(i, &m0, &m1);
		if (p == 0) { joystick_present += (1 << i); }
		joystick_raw[i].mask_0 = ~m0;
		joystick_raw[i].mask_1 = ~m1;
	}
}

//  ---- input action

_sInputAction action[ACTION_ID_COUNT] = { 0 };

void InputActionInit(uint8_t joystick_n) {
	uint8_t i;
	action[ACTION_UP].keybind_mask_0 = JOYSTICK_M0_UP;
	action[ACTION_DOWN].keybind_mask_0 = JOYSTICK_M0_DOWN;
	action[ACTION_LEFT].keybind_mask_0 = JOYSTICK_M0_LEFT;
	action[ACTION_RIGHT].keybind_mask_0 = JOYSTICK_M0_RIGHT;
	action[ACTION_START].keybind_mask_0 = JOYSTICK_M0_START;
	action[ACTION_DEBUG].keybind_mask_0 = JOYSTICK_M0_SELECT;

	for (i = 0; i < ACTION_ID_COUNT; i++) {
		action[i].joystick_mapped = joystick_n;
	}
}

// TODO: optimeze this
void HandleInputActions() {
	//const uint8_t* key_states = SDL_GetKeyboardState(NULL);
	uint8_t i, btn_pressed_0, btn_pressed_1, last, joystick_n;
	JoysticksReadRaw();
	for (i = 0; i < ACTION_ID_COUNT; i++) {
		//if (action[i].keybind_1 == SDL_SCANCODE_UNKNOWN) { key_1 = false; } // we don't have 500 keycodes to worry about, checking is overkill
		joystick_n = action[i].joystick_mapped;
		btn_pressed_0 = action[i].keybind_mask_0 & joystick_raw[joystick_n].mask_0;
		btn_pressed_1 = action[i].keybind_mask_1 & joystick_raw[joystick_n].mask_1;

		last = action[i].pressed;

		action[i].pressed = (btn_pressed_0 || btn_pressed_1) != 0;

		action[i].just_pressed = action[i].pressed && (!last);
	}
}

uint8_t IsActionPressed(_eInputActionId id) {
	return action[id].pressed;
}

uint8_t IsActionJustPressed(_eInputActionId id) {
	return action[id].just_pressed;
}


// ---- draw pressed buttons on bitmap layer

#define COLOR_PRESSED		1
#define COLOR_NOT_PRESSED	2
#define COLOR_JUST_PRESSED	3
#if (BITMAP_BPP == 2)
#define FILL_FULL(a)	(a << 6) | (a << 4) | (a << 2) | a
#define FILL_HALF(a)	(a << 4) | (a << 2)
#elif (BITMAP_BPP == 4)
#define FILL_FULL(a)	(a << 4) | a
#define FILL_HALF(a)	(a << 4)
#endif

#define FILL_IF_PRESSED_FULL(cond, col)	if (cond) { col = FILL_FULL(COLOR_PRESSED); } else { col = FILL_FULL(COLOR_NOT_PRESSED); }
#define FILL_IF_PRESSED_HALF(cond, col)	if (cond) { col = FILL_HALF(COLOR_PRESSED); } else { col = FILL_HALF(COLOR_NOT_PRESSED); }
void JoystickDrawToBitmap(uint8_t joystick_n, uint8_t buffer_n, uint8_t x, uint8_t y) {
	uint8_t p, c[12];
	_uConv16 addr;
	// read
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_UP;
	FILL_IF_PRESSED_HALF(p, c[0]);
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_DOWN;
	FILL_IF_PRESSED_HALF(p, c[1]);
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_LEFT;
	FILL_IF_PRESSED_HALF(p, c[2]);
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_RIGHT;
	FILL_IF_PRESSED_HALF(p, c[3]);
	p = joystick_raw[joystick_n].mask_1 & JOYSTICK_M1_A;
	FILL_IF_PRESSED_HALF(p, c[4]);
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_B;
	FILL_IF_PRESSED_HALF(p, c[5]);
	p = joystick_raw[joystick_n].mask_1 & JOYSTICK_M1_X;
	FILL_IF_PRESSED_HALF(p, c[6]);
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_Y;
	FILL_IF_PRESSED_HALF(p, c[7]);
	p = joystick_raw[joystick_n].mask_1 & JOYSTICK_M1_L;
	FILL_IF_PRESSED_FULL(p, c[8]);
	p = joystick_raw[joystick_n].mask_1 & JOYSTICK_M1_R;
	FILL_IF_PRESSED_FULL(p, c[9]);
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_START;
	FILL_IF_PRESSED_HALF(p, c[10]);
	p = joystick_raw[joystick_n].mask_0 & JOYSTICK_M0_SELECT;
	FILL_IF_PRESSED_HALF(p, c[11]);

	// setup vera
	VERA_CTRL = 0;
	VERA_ADDRx_H = buffer_n | ADDR_INC_1;
	addr.h = MEM_BITMAP_1_ADDR_M;
	addr.l = x;
	addr.w += lookup_bitmap_y[y];

	// -- draw
	// L and R
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[8];
	VERA_DATA0 = c[8];
	VERA_DATA0 = c[8];
	VERA_DATA0 = 0;
	VERA_DATA0 = 0;
	VERA_DATA0 = c[9];
	VERA_DATA0 = c[9];
	VERA_DATA0 = c[9];

	// top row (UP and X)
	addr.w += BITMAP_WIDTH_BYTES + BITMAP_WIDTH_BYTES + 1;
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[0];
	VERA_DATA0 = 0;
	VERA_DATA0 = 0;
	VERA_DATA0 = 0;
	VERA_DATA0 = 0;
	VERA_DATA0 = c[6];
	addr.w += BITMAP_WIDTH_BYTES;
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[0];
	VERA_DATA0 = 0;
	VERA_DATA0 = 0;
	VERA_DATA0 = 0;
	VERA_DATA0 = 0;
	VERA_DATA0 = c[6];

	// middle row (LEFT, RIGHT, SELECT, START, Y, A)
	addr.w += BITMAP_WIDTH_BYTES + BITMAP_WIDTH_BYTES - 1;
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[2];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[3];
	VERA_DATA0 = c[11];
	VERA_DATA0 = c[10];
	VERA_DATA0 = c[7];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[4];
	addr.w += BITMAP_WIDTH_BYTES;
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[2];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[3];
	VERA_DATA0 = c[11];
	VERA_DATA0 = c[10];
	VERA_DATA0 = c[7];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[4];

	// bottom row (DOWN, SELECT, START, B)
	addr.w += BITMAP_WIDTH_BYTES + BITMAP_WIDTH_BYTES + 1;
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[1];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[11];
	VERA_DATA0 = c[10];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[5];
	addr.w += BITMAP_WIDTH_BYTES;
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[1];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[11];
	VERA_DATA0 = c[10];
	VERA_DATA0 = 0;
	VERA_DATA0 = c[5];

	// middle slice of SELECT and START
	addr.w -= BITMAP_WIDTH_BYTES + BITMAP_WIDTH_BYTES - 2;
	VERA_ADDRx_M = addr.h;
	VERA_ADDRx_L = addr.l;
	VERA_DATA0 = c[11];
	VERA_DATA0 = c[10];
}

void InputActionDrawToBitmap(uint8_t buffer_n, uint8_t x, uint8_t y) {
	uint8_t i, c;
	_uConv16 addr;
	// setup vera
	VERA_CTRL = 0;
#if (BITMAP_BPP == 2)
	VERA_ADDRx_H = buffer_n | ADDR_INC_80;
#elif (BITMAP_BPP == 4)
	VERA_ADDRx_H = buffer_n | ADDR_INC_160;
#endif
	addr.h = MEM_BITMAP_1_ADDR_M;
	addr.l = x;
	addr.w += lookup_bitmap_y[y];

	for (i = 0; i < ACTION_ID_COUNT; i++) {
		// read input
		if (IsActionJustPressed(i)) {
			c = FILL_FULL(COLOR_JUST_PRESSED);
		} else {
			FILL_IF_PRESSED_FULL(IsActionPressed(i), c);
		}
		// draw
		VERA_ADDRx_M = addr.h;
		VERA_ADDRx_L = addr.l;
		VERA_DATA0 = c;
		VERA_DATA0 = c;
		VERA_DATA0 = c;
		VERA_DATA0 = c;
		addr.w += 2;
	}

}