// A generic HID gamepad as Switch input (front_mac.m reads the device; pure, so tests/t_pad_hid.c checks it without one). GameController gives no controller for a pad it does not
// know, such as an 8BitDo Ultimate 2C over Bluetooth LE, so the front end reads those by HID usage: Button n through the map below, X / Y the left stick, Z / Rz the right stick,
// the hat switch the d-pad. The default map is the button order of 8BitDo's Bluetooth LE (Android) mode, recorded on an Ultimate 2C: A 1, B 2, X 4, Y 5, LB 7, RB 8, LT 9, RT 10,
// Select 11, Start 12, Home 13, L3 14, R3 15. Its face buttons go by position, as on any non-Nintendo pad (pad_apple.h): right A, bottom B, top X, left Y. A pad with another
// order takes DM_PAD_HID_MAP: twelve HID button numbers for Switch A, B, X, Y, L, R, ZL, ZR, Minus, Plus, LS, RS.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "dm_input.h"

enum { PAD_HID_MAPPED = 12 };
typedef struct { uint32_t held; int32_t axis[4]; int hat; } pad_hid;   // held bit n - 1: HID button n; axis: lx, ly, rx, ry (up is positive); hat 0 (up) .. 7 clockwise, -1 centred
static const uint64_t pad_hid_bits[PAD_HID_MAPPED] = { DM_BTN_A, DM_BTN_B, DM_BTN_X, DM_BTN_Y, DM_BTN_L, DM_BTN_R, DM_BTN_ZL, DM_BTN_ZR, DM_BTN_MINUS, DM_BTN_PLUS, DM_BTN_LS, DM_BTN_RS };
static const uint8_t pad_hid_default_map[PAD_HID_MAPPED] = { 2, 1, 5, 4, 7, 8, 9, 10, 11, 12, 14, 15 };

// "2,1,5,..." into map; 0 (map unchanged) unless it is twelve numbers 1..32
static int pad_hid_parse_map(uint8_t map[PAD_HID_MAPPED], const char *s) {
    uint8_t m[PAD_HID_MAPPED];
    for (int i = 0; i < PAD_HID_MAPPED; i++) {
        char *end; long n = strtol(s, &end, 10);
        if (end == s || n < 1 || n > 32 || *end != (i == PAD_HID_MAPPED - 1 ? '\0' : ',')) return 0;
        m[i] = (uint8_t)n; s = end + 1;
    }
    for (int i = 0; i < PAD_HID_MAPPED; i++) map[i] = m[i];
    return 1;
}
static int32_t pad_hid_axis(long v, long min, long max) {   // min .. max onto -DM_STICK_MAX .. DM_STICK_MAX
    long a = (2 * (v - min) - (max - min)) * DM_STICK_MAX / (max - min);
    return (int32_t)(a < -DM_STICK_MAX ? -DM_STICK_MAX : a > DM_STICK_MAX ? DM_STICK_MAX : a);
}
// One element's value from the device (its usage page and usage, logical range)
static void pad_hid_value(pad_hid *h, uint32_t page, uint32_t usage, long v, long min, long max) {
    if (page == 9 && usage >= 1 && usage <= 32) { uint32_t b = 1u << (usage - 1); h->held = v ? h->held | b : h->held & ~b; return; }
    if (page != 1) return;
    if (usage == 0x39) { h->hat = v >= min && v <= max && max - min == 7 ? (int)(v - min) : -1; return; }   // out of range: the null state, centred
    if (max <= min) return;
    switch (usage) {
        case 0x30: h->axis[0] = pad_hid_axis(v, min, max); break;
        case 0x31: h->axis[1] = -pad_hid_axis(v, min, max); break;   // HID Y grows downwards
        case 0x32: h->axis[2] = pad_hid_axis(v, min, max); break;
        case 0x35: h->axis[3] = -pad_hid_axis(v, min, max); break;
    }
}
static uint64_t pad_hid_buttons(const pad_hid *h, const uint8_t map[PAD_HID_MAPPED]) {
    uint64_t m = 0;
    for (int i = 0; i < PAD_HID_MAPPED; i++) if (h->held & 1u << (map[i] - 1)) m |= pad_hid_bits[i];
    int t = h->hat;
    if (t == 7 || t == 0 || t == 1) m |= DM_BTN_UP;
    if (t >= 1 && t <= 3) m |= DM_BTN_RIGHT;
    if (t >= 3 && t <= 5) m |= DM_BTN_DOWN;
    if (t >= 5 && t <= 7) m |= DM_BTN_LEFT;
    return m;
}
