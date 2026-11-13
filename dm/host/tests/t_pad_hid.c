// Generic HID gamepad mapping (pad_hid.h) with the values an 8BitDo Ultimate 2C sends over Bluetooth LE; no device is needed.
#include "../pad_hid.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    const uint8_t *map = pad_hid_default_map;
    pad_hid h = { .hat = -1 };
    assert(pad_hid_buttons(&h, map) == 0);
    pad_hid_value(&h, 9, 1, 1, 0, 1);   // the pad's A (bottom) is the Switch's B
    assert(pad_hid_buttons(&h, map) == DM_BTN_B);
    pad_hid_value(&h, 9, 2, 1, 0, 1); pad_hid_value(&h, 9, 1, 0, 0, 1);   // its B (right) is A
    assert(pad_hid_buttons(&h, map) == DM_BTN_A);
    pad_hid_value(&h, 9, 2, 0, 0, 1); pad_hid_value(&h, 9, 9, 1, 0, 1); pad_hid_value(&h, 9, 12, 1, 0, 1);
    assert(pad_hid_buttons(&h, map) == (DM_BTN_ZL | DM_BTN_PLUS));
    pad_hid_value(&h, 9, 9, 0, 0, 1); pad_hid_value(&h, 9, 12, 0, 0, 1); pad_hid_value(&h, 9, 13, 1, 0, 1);   // Home: no Switch button
    assert(pad_hid_buttons(&h, map) == 0);
    pad_hid_value(&h, 1, 0x39, 1, 0, 7); assert(pad_hid_buttons(&h, map) == (DM_BTN_UP | DM_BTN_RIGHT));
    pad_hid_value(&h, 1, 0x39, 6, 0, 7); assert(pad_hid_buttons(&h, map) == DM_BTN_LEFT);
    pad_hid_value(&h, 1, 0x39, 8, 0, 7); assert(pad_hid_buttons(&h, map) == 0);   // null state
    pad_hid_value(&h, 1, 0x30, 255, 0, 255); assert(h.axis[0] == DM_STICK_MAX);
    pad_hid_value(&h, 1, 0x31, 0, 0, 255); assert(h.axis[1] == DM_STICK_MAX);   // pushed up
    pad_hid_value(&h, 1, 0x35, 255, 0, 255); assert(h.axis[3] == -DM_STICK_MAX);
    pad_hid_value(&h, 1, 0x32, 128, 0, 255); assert(h.axis[2] > 0 && h.axis[2] < DM_STICK_MAX / 100);   // centre: about zero
    uint8_t m[PAD_HID_MAPPED]; for (int i = 0; i < PAD_HID_MAPPED; i++) m[i] = map[i];
    assert(!pad_hid_parse_map(m, "1,2,3") && m[0] == 2);
    assert(!pad_hid_parse_map(m, "1,2,3,4,5,6,7,8,9,10,11,33"));
    assert(pad_hid_parse_map(m, "1,2,4,5,7,8,9,10,11,12,14,15") && m[0] == 1 && m[11] == 15);
    printf("t_pad_hid: ok\n");
    return 0;
}
