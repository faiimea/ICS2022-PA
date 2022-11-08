#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  int am_keybrd = inl(KBD_ADDR);
	kbd->keydown = (am_keybrd & KEYDOWN_MASK) ? 1 : 0;
  kbd->keycode = AM_KEY_NONE & (~KEYDOWN_MASK);
}
