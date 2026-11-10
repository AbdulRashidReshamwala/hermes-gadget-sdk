// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 ALTIO AI Private Limited
// JD9853 start-up registers for the Cheeko Gotchi v1.2's 2.01" 240x296 panel:
// JD9853_PANEL_INIT in firmware/hardware_test/main/cheeko_self_test.c and
// firmware/animated_face_runner/main/board.c of
// https://github.com/ALTIO-AI-PRIVATE-LIMITED/cheeko-gotchi, commit
// 813fb6d221cd09e2f492e7cb169bf183685ddeea (MIT, LICENSES/Cheeko-Gotchi-MIT.txt).
// Values, order and delays are unchanged; the payloads are written as C++ byte
// strings because the original's C compound literals are not valid C++. That
// repository does not say which panel vendor's sequence this is. See NOTICE.
//
// esp_lcd sends sleep-out, MADCTL and COLMOD before this table. The table's
// own 0x3A (RGB565) replaces the COLMOD, and its 0x11 and 0x29 end it.
#pragma once

#include "esp_lcd_jd9853.h"

namespace hgp {
constexpr jd9853_lcd_init_cmd_t kJd9853PanelInit[] = {
    {0xDF, "\x98\x53", 2, 0},
    {0xDE, "\x00", 1, 0},
    {0xB2, "\x25", 1, 0},
    {0xB7, "\x00\x29\x00\x51", 4, 0},
    {0xBB, "\x4F\x1A\x55\x73\x63\xF0", 6, 0},
    {0xC0, "\x44\xA4", 2, 0},
    {0xC1, "\x12", 1, 0},
    {0xC3, "\x7D\x07\x14\x06\xC8\x71\x6C\x77", 8, 0},
    {0xC4, "\x00\x00\x94\x79\x25\x0A\x16\x79\x25\x0A\x16\x82", 12, 0},
    {0xC8, "\x3F\x34\x2D\x26\x2B\x2B\x25\x24\x23\x22\x20\x17\x14\x0E\x06\x00"
           "\x3F\x34\x2D\x26\x2B\x2B\x25\x24\x23\x22\x20\x17\x14\x0E\x06\x00", 32, 0},
    {0xD0, "\x04\x06\x6B\x0F\x00", 5, 0},
    {0xD7, "\x00\x30", 2, 0},
    {0xE6, "\x10", 1, 0},
    {0xDE, "\x01", 1, 0},
    {0xB7, "\x03\x13\xEF\x35\x35", 5, 0},
    {0xC1, "\x14\x15\xC0", 3, 0},
    {0xC2, "\x06\x3A", 2, 0},
    {0xC4, "\x72\x12", 2, 0},
    {0xBE, "\x00", 1, 0},
    {0xDE, "\x00", 1, 0},
    {0x35, "\x00", 1, 0},
    {0x3A, "\x05", 1, 0},                // RGB565
    {0x2A, "\x00\x00\x00\xEF", 4, 0},    // columns 0..239
    {0x2B, "\x00\x00\x01\x27", 4, 0},    // rows 0..295
    {0x11, nullptr, 0, 120},             // sleep out
    {0x29, nullptr, 0, 10},              // display on
};
}  // namespace hgp
