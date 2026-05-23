#pragma once

#define VIAL_KEYBOARD_UID {0x89, 0x36, 0x2A, 0xC7, 0xFA, 0xD8, 0x89, 0x45}
#define VIAL_UNLOCK_COMBO_ROWS {0, 0}
#define VIAL_UNLOCK_COMBO_COLS {0, 1}

// Tap-hold: 他キーが押された瞬間に hold 側として確定する (タップタームの満了を待たない)
// https://docs.qmk.fm/tap_hold#hold-on-other-key-press
#define HOLD_ON_OTHER_KEY_PRESS
