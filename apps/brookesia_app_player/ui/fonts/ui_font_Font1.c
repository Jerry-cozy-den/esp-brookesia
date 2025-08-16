/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --font C:/Users/jerry/SquareLine/assets/AlibabaPuHuiTi-2-35-Thin.ttf -o C:/Users/jerry/SquareLine/assets\ui_font_Font1.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#include "../ui.h"

#ifndef UI_FONT_FONT1
#define UI_FONT_FONT1 1
#endif

#if UI_FONT_FONT1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xa0,

    /* U+0022 "\"" */
    0xb6, 0xda,

    /* U+0023 "#" */
    0x11, 0x8, 0x84, 0x5f, 0xf1, 0x21, 0x10, 0x89,
    0xff, 0x24, 0x22, 0x11, 0x0,

    /* U+0024 "$" */
    0x10, 0x21, 0xf4, 0x89, 0x12, 0x14, 0x1c, 0x16,
    0x24, 0x48, 0xbf, 0xc2, 0x4, 0x8,

    /* U+0025 "%" */
    0x60, 0x92, 0x22, 0x48, 0x49, 0x9, 0x40, 0xcb,
    0x82, 0x88, 0x91, 0x12, 0x24, 0x44, 0x87, 0x0,

    /* U+0026 "&" */
    0x1c, 0x8, 0x82, 0x20, 0x88, 0x1c, 0xc, 0x4,
    0x8a, 0x12, 0x83, 0x30, 0xc7, 0xcc,

    /* U+0027 "'" */
    0xf8,

    /* U+0028 "(" */
    0x29, 0x29, 0x24, 0x92, 0x24, 0x88,

    /* U+0029 ")" */
    0x91, 0x22, 0x49, 0x24, 0xa5, 0x20,

    /* U+002A "*" */
    0x21, 0x3e, 0xa9, 0x0,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x8,

    /* U+002C "," */
    0x25, 0x20,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0x44, 0x21, 0x10, 0x84, 0x42, 0x11, 0x8,
    0x0,

    /* U+0030 "0" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xf0,

    /* U+0031 "1" */
    0x71, 0x11, 0x11, 0x11, 0x11, 0x10,

    /* U+0032 "2" */
    0x7c, 0x6, 0x2, 0x2, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40, 0xff,

    /* U+0033 "3" */
    0x7c, 0xc, 0x8, 0x23, 0x80, 0x80, 0x81, 0x2,
    0xf, 0xe0,

    /* U+0034 "4" */
    0x6, 0xa, 0xa, 0x12, 0x22, 0x42, 0x42, 0xff,
    0x2, 0x2, 0x2,

    /* U+0035 "5" */
    0x7c, 0x81, 0x4, 0x7, 0x80, 0xc0, 0x81, 0x2,
    0xb, 0xe0,

    /* U+0036 "6" */
    0x1c, 0x41, 0x5, 0xec, 0x70, 0x60, 0xc1, 0x82,
    0x89, 0xf0,

    /* U+0037 "7" */
    0xfe, 0x4, 0x10, 0x20, 0x81, 0x4, 0x8, 0x20,
    0x41, 0x0,

    /* U+0038 "8" */
    0x7d, 0x8e, 0xc, 0x17, 0xc8, 0xe0, 0xc1, 0x83,
    0x8d, 0xf0,

    /* U+0039 "9" */
    0x7d, 0x8a, 0xc, 0x18, 0x38, 0xde, 0x83, 0x4,
    0x11, 0xc0,

    /* U+003A ":" */
    0x80, 0x80,

    /* U+003B ";" */
    0x40, 0x1, 0x68,

    /* U+003C "<" */
    0x1, 0x6, 0x18, 0xe0, 0x80, 0x70, 0xc, 0x3,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0x0, 0xe0, 0x18, 0x6, 0x3, 0xc, 0x70, 0x80,

    /* U+003F "?" */
    0xf8, 0x30, 0x41, 0x4, 0x62, 0x8, 0x20, 0x2,
    0x0,

    /* U+0040 "@" */
    0xf, 0x81, 0x6, 0x20, 0x24, 0x79, 0xc8, 0x99,
    0x91, 0x91, 0x19, 0x11, 0x91, 0x19, 0x12, 0xce,
    0xc4, 0x0, 0x30, 0x1, 0xf8,

    /* U+0041 "A" */
    0xc, 0x3, 0x1, 0x40, 0x48, 0x22, 0x8, 0x43,
    0xf1, 0x4, 0x40, 0xa0, 0x28, 0x8,

    /* U+0042 "B" */
    0xf9, 0xa, 0x14, 0x28, 0x5f, 0x23, 0xc1, 0x83,
    0xf, 0xf0,

    /* U+0043 "C" */
    0x3f, 0x60, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80,
    0xc0, 0x60, 0x3e,

    /* U+0044 "D" */
    0xfc, 0x86, 0x83, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x82, 0x86, 0xf8,

    /* U+0045 "E" */
    0xfd, 0x2, 0x4, 0x8, 0x1f, 0xa0, 0x40, 0x81,
    0x3, 0xf8,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0x83, 0xf8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0047 "G" */
    0x3f, 0x60, 0xc0, 0x80, 0x80, 0x87, 0x81, 0x81,
    0xc1, 0x61, 0x3f,

    /* U+0048 "H" */
    0x81, 0x81, 0x81, 0x81, 0xff, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81,

    /* U+0049 "I" */
    0xff, 0xe0,

    /* U+004A "J" */
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e,

    /* U+004B "K" */
    0x83, 0xa, 0x24, 0x8e, 0x14, 0x24, 0x44, 0x85,
    0xa, 0x8,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+004D "M" */
    0xc0, 0xf0, 0x3a, 0x16, 0x85, 0x92, 0x64, 0x98,
    0xc6, 0x31, 0x80, 0x60, 0x18, 0x4,

    /* U+004E "N" */
    0xc0, 0xe0, 0x68, 0x32, 0x19, 0xc, 0x46, 0x13,
    0xd, 0x82, 0xc0, 0xe0, 0x20,

    /* U+004F "O" */
    0x3e, 0x20, 0xa0, 0x30, 0x18, 0xc, 0x6, 0x3,
    0x1, 0x80, 0xa0, 0x8f, 0x80,

    /* U+0050 "P" */
    0xfd, 0xe, 0xc, 0x18, 0x30, 0xff, 0x40, 0x81,
    0x2, 0x0,

    /* U+0051 "Q" */
    0x3e, 0x20, 0xa0, 0x30, 0x18, 0xc, 0x6, 0x3,
    0x1, 0x80, 0xa0, 0x8f, 0x80, 0x80, 0x20,

    /* U+0052 "R" */
    0xfc, 0x86, 0x82, 0x82, 0x86, 0xfc, 0x88, 0x84,
    0x84, 0x82, 0x81,

    /* U+0053 "S" */
    0x7d, 0x82, 0x4, 0x4, 0x7, 0x1, 0x81, 0x2,
    0xf, 0xf0,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+0055 "U" */
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0xc3, 0x7c,

    /* U+0056 "V" */
    0x81, 0x40, 0x90, 0x48, 0x44, 0x21, 0x10, 0x90,
    0x48, 0x14, 0xc, 0x2, 0x0,

    /* U+0057 "W" */
    0x80, 0xc, 0x10, 0x50, 0xc2, 0x8a, 0x24, 0x51,
    0x22, 0x48, 0x92, 0x45, 0x14, 0x28, 0xe1, 0x43,
    0x4, 0x18,

    /* U+0058 "X" */
    0x41, 0x20, 0x88, 0x82, 0x81, 0x40, 0x40, 0x50,
    0x48, 0x22, 0x20, 0x90, 0x40,

    /* U+0059 "Y" */
    0x80, 0xa0, 0x88, 0x84, 0x41, 0x40, 0x40, 0x20,
    0x10, 0x8, 0x4, 0x2, 0x0,

    /* U+005A "Z" */
    0xff, 0x1, 0x2, 0x4, 0x8, 0x8, 0x10, 0x20,
    0x40, 0x80, 0xff,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0x38,

    /* U+005C "\\" */
    0x81, 0x1, 0x2, 0x4, 0x4, 0x8, 0x8, 0x10,
    0x10, 0x20, 0x20, 0x40, 0x80,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x92, 0x78,

    /* U+005E "^" */
    0x10, 0xa2, 0x92, 0x46, 0x10,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0x89, 0x0,

    /* U+0061 "a" */
    0x3c, 0x4, 0x8, 0x17, 0xf0, 0x60, 0xc3, 0x7a,

    /* U+0062 "b" */
    0x81, 0x2, 0x5, 0xec, 0x70, 0x60, 0xc1, 0x83,
    0x7, 0x15, 0xe0,

    /* U+0063 "c" */
    0x7d, 0x8, 0x20, 0x82, 0x8, 0x10, 0x7c,

    /* U+0064 "d" */
    0x2, 0x4, 0xb, 0xdc, 0x70, 0x60, 0xc1, 0x83,
    0x7, 0x1b, 0xd0,

    /* U+0065 "e" */
    0x7c, 0x8e, 0xc, 0x1f, 0xf0, 0x20, 0x20, 0x7c,

    /* U+0066 "f" */
    0x1c, 0x82, 0x3e, 0x20, 0x82, 0x8, 0x20, 0x82,
    0x8,

    /* U+0067 "g" */
    0x7b, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7a,
    0x4, 0x13, 0xc0,

    /* U+0068 "h" */
    0x81, 0x2, 0x5, 0xec, 0x70, 0x60, 0xc1, 0x83,
    0x6, 0xc, 0x10,

    /* U+0069 "i" */
    0x9f, 0xf0,

    /* U+006A "j" */
    0x10, 0x1, 0x11, 0x11, 0x11, 0x11, 0x11, 0xe0,

    /* U+006B "k" */
    0x82, 0x8, 0x21, 0x8a, 0x4a, 0x30, 0xa2, 0x48,
    0xa1,

    /* U+006C "l" */
    0x92, 0x49, 0x24, 0x92, 0x70,

    /* U+006D "m" */
    0xb9, 0xd8, 0xc6, 0x10, 0xc2, 0x18, 0x43, 0x8,
    0x61, 0xc, 0x21, 0x84, 0x20,

    /* U+006E "n" */
    0xbd, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,

    /* U+006F "o" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42,
    0x7c,

    /* U+0070 "p" */
    0xbd, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe2, 0xbd,
    0x2, 0x4, 0x0,

    /* U+0071 "q" */
    0x7b, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7a,
    0x4, 0x8, 0x10,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88, 0x88, 0x80,

    /* U+0073 "s" */
    0x7c, 0x21, 0x6, 0xc, 0x21, 0xf0,

    /* U+0074 "t" */
    0x42, 0x3c, 0x84, 0x21, 0x8, 0x42, 0x1e,

    /* U+0075 "u" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7a,

    /* U+0076 "v" */
    0x83, 0x5, 0xa, 0x24, 0x45, 0xa, 0x14, 0x10,

    /* U+0077 "w" */
    0x82, 0x18, 0x61, 0x46, 0x14, 0x92, 0x49, 0x24,
    0x92, 0x30, 0xc3, 0xc, 0x30, 0x40,

    /* U+0078 "x" */
    0x82, 0x88, 0xa1, 0x41, 0x5, 0xa, 0x22, 0x82,

    /* U+0079 "y" */
    0x83, 0x5, 0xa, 0x22, 0x45, 0xa, 0xc, 0x10,
    0x20, 0xc3, 0x0,

    /* U+007A "z" */
    0xfc, 0x10, 0x84, 0x20, 0x84, 0x20, 0xfc,

    /* U+007B "{" */
    0x29, 0x24, 0x94, 0x49, 0x24, 0x88,

    /* U+007C "|" */
    0xff, 0xfe,

    /* U+007D "}" */
    0x89, 0x24, 0x91, 0x49, 0x24, 0xa0,

    /* U+007E "~" */
    0x71, 0x49, 0x86
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 61, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 84, .box_w = 1, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 100, .box_w = 3, .box_h = 5, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 5, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 140, .box_w = 7, .box_h = 16, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 32, .adv_w = 205, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 170, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 68, .box_w = 1, .box_h = 5, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 63, .adv_w = 79, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 69, .adv_w = 79, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 75, .adv_w = 98, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 79, .adv_w = 141, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 86, .adv_w = 66, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 88, .adv_w = 108, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 89, .adv_w = 66, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 118, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 99, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 140, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 140, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 140, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 87, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 87, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 202, .adv_w = 141, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 210, .adv_w = 141, .box_w = 7, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 214, .adv_w = 141, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 222, .adv_w = 108, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 221, .box_w = 12, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 252, .adv_w = 155, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 149, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 148, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 172, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 136, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 169, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 172, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 59, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 55, .box_w = 4, .box_h = 14, .ofs_x = -2, .ofs_y = -3},
    {.bitmap_index = 348, .adv_w = 144, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 121, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 201, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 182, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 185, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 185, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 432, .adv_w = 146, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 138, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 126, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 169, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 139, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 487, .adv_w = 218, .box_w = 13, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 144, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 135, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 531, .adv_w = 151, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 542, .adv_w = 74, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 548, .adv_w = 135, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 561, .adv_w = 74, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 567, .adv_w = 141, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 572, .adv_w = 128, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 573, .adv_w = 78, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 575, .adv_w = 151, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 583, .adv_w = 155, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 594, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 601, .adv_w = 155, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 141, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 620, .adv_w = 82, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 629, .adv_w = 155, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 640, .adv_w = 150, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 60, .box_w = 1, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 653, .adv_w = 60, .box_w = 4, .box_h = 15, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 661, .adv_w = 119, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 670, .adv_w = 63, .box_w = 3, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 675, .adv_w = 227, .box_w = 11, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 688, .adv_w = 150, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 696, .adv_w = 151, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 705, .adv_w = 155, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 716, .adv_w = 155, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 727, .adv_w = 87, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 116, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 738, .adv_w = 78, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 149, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 753, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 761, .adv_w = 199, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 775, .adv_w = 116, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 783, .adv_w = 120, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 794, .adv_w = 117, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 801, .adv_w = 76, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 807, .adv_w = 37, .box_w = 1, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 809, .adv_w = 76, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 815, .adv_w = 141, .box_w = 8, .box_h = 3, .ofs_x = 0, .ofs_y = 4}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    3, 3,
    3, 8,
    3, 13,
    3, 15,
    8, 3,
    8, 8,
    8, 13,
    8, 15,
    9, 75,
    13, 3,
    13, 8,
    13, 18,
    13, 24,
    13, 26,
    15, 3,
    15, 8,
    15, 18,
    15, 24,
    15, 26,
    16, 16,
    18, 13,
    18, 15,
    18, 27,
    18, 28,
    24, 13,
    24, 15,
    24, 27,
    24, 28,
    27, 18,
    27, 24,
    28, 18,
    28, 24,
    34, 3,
    34, 8,
    34, 36,
    34, 40,
    34, 48,
    34, 50,
    34, 53,
    34, 54,
    34, 55,
    34, 56,
    34, 58,
    34, 71,
    34, 77,
    34, 85,
    34, 87,
    34, 90,
    35, 13,
    35, 15,
    35, 36,
    35, 40,
    35, 48,
    35, 50,
    35, 53,
    35, 55,
    35, 57,
    35, 58,
    37, 13,
    37, 15,
    37, 34,
    37, 53,
    37, 55,
    37, 56,
    37, 57,
    37, 58,
    37, 59,
    37, 66,
    38, 75,
    39, 13,
    39, 15,
    39, 34,
    39, 66,
    40, 55,
    40, 58,
    43, 43,
    44, 34,
    44, 36,
    44, 40,
    44, 48,
    44, 50,
    44, 53,
    44, 54,
    44, 55,
    44, 56,
    44, 58,
    44, 68,
    44, 69,
    44, 70,
    44, 72,
    44, 77,
    44, 80,
    44, 82,
    44, 84,
    44, 85,
    44, 86,
    44, 87,
    44, 88,
    44, 90,
    45, 3,
    45, 8,
    45, 34,
    45, 36,
    45, 40,
    45, 48,
    45, 50,
    45, 53,
    45, 54,
    45, 55,
    45, 56,
    45, 58,
    45, 87,
    45, 88,
    45, 90,
    48, 13,
    48, 15,
    48, 34,
    48, 53,
    48, 55,
    48, 56,
    48, 57,
    48, 58,
    48, 59,
    48, 66,
    49, 13,
    49, 15,
    49, 34,
    49, 53,
    49, 55,
    49, 56,
    49, 57,
    49, 58,
    49, 59,
    49, 66,
    50, 13,
    50, 15,
    50, 34,
    50, 43,
    50, 53,
    50, 55,
    50, 56,
    50, 57,
    50, 58,
    50, 59,
    50, 66,
    51, 36,
    51, 40,
    51, 48,
    51, 50,
    51, 53,
    51, 54,
    51, 55,
    51, 56,
    51, 57,
    51, 58,
    51, 68,
    51, 69,
    51, 70,
    51, 72,
    51, 80,
    51, 82,
    51, 85,
    51, 86,
    51, 87,
    51, 89,
    51, 90,
    52, 13,
    52, 15,
    52, 55,
    52, 56,
    52, 58,
    52, 87,
    52, 88,
    52, 90,
    53, 13,
    53, 15,
    53, 34,
    53, 36,
    53, 40,
    53, 48,
    53, 50,
    53, 53,
    53, 55,
    53, 58,
    53, 66,
    53, 68,
    53, 69,
    53, 70,
    53, 72,
    53, 74,
    53, 75,
    53, 78,
    53, 79,
    53, 80,
    53, 81,
    53, 82,
    53, 83,
    53, 84,
    53, 85,
    53, 86,
    53, 87,
    53, 88,
    53, 89,
    53, 90,
    53, 91,
    54, 34,
    54, 57,
    55, 13,
    55, 15,
    55, 27,
    55, 28,
    55, 34,
    55, 36,
    55, 40,
    55, 48,
    55, 50,
    55, 52,
    55, 53,
    55, 66,
    55, 68,
    55, 69,
    55, 70,
    55, 72,
    55, 80,
    55, 82,
    55, 84,
    56, 13,
    56, 15,
    56, 27,
    56, 28,
    56, 34,
    56, 36,
    56, 40,
    56, 48,
    56, 50,
    56, 66,
    56, 68,
    56, 69,
    56, 70,
    56, 72,
    56, 80,
    56, 82,
    56, 84,
    57, 34,
    57, 36,
    57, 40,
    57, 48,
    57, 50,
    57, 53,
    57, 54,
    57, 55,
    57, 56,
    57, 58,
    57, 68,
    57, 69,
    57, 70,
    57, 72,
    57, 77,
    57, 80,
    57, 82,
    57, 84,
    57, 85,
    57, 86,
    57, 87,
    57, 88,
    57, 90,
    58, 13,
    58, 15,
    58, 27,
    58, 28,
    58, 34,
    58, 36,
    58, 40,
    58, 48,
    58, 50,
    58, 52,
    58, 53,
    58, 66,
    58, 68,
    58, 69,
    58, 70,
    58, 72,
    58, 74,
    58, 78,
    58, 79,
    58, 80,
    58, 81,
    58, 82,
    58, 83,
    58, 84,
    58, 86,
    58, 87,
    58, 89,
    58, 90,
    58, 91,
    59, 36,
    59, 40,
    59, 48,
    59, 50,
    60, 75,
    66, 53,
    66, 55,
    66, 58,
    66, 87,
    66, 88,
    66, 90,
    67, 13,
    67, 15,
    67, 53,
    67, 55,
    67, 56,
    67, 57,
    67, 58,
    67, 87,
    67, 89,
    67, 90,
    70, 13,
    70, 15,
    70, 53,
    70, 55,
    70, 58,
    70, 75,
    71, 11,
    71, 32,
    71, 53,
    71, 55,
    71, 56,
    71, 57,
    71, 58,
    71, 62,
    71, 66,
    71, 68,
    71, 69,
    71, 70,
    71, 72,
    71, 73,
    71, 74,
    71, 75,
    71, 76,
    71, 77,
    71, 80,
    71, 82,
    71, 84,
    71, 87,
    71, 88,
    71, 89,
    71, 90,
    73, 53,
    73, 58,
    73, 87,
    73, 90,
    74, 32,
    74, 53,
    74, 62,
    74, 94,
    75, 53,
    76, 53,
    76, 56,
    76, 66,
    76, 68,
    76, 69,
    76, 70,
    76, 72,
    76, 77,
    76, 80,
    76, 82,
    76, 86,
    77, 71,
    77, 87,
    77, 88,
    77, 90,
    78, 53,
    78, 58,
    78, 87,
    78, 90,
    79, 53,
    79, 58,
    79, 87,
    79, 90,
    80, 13,
    80, 15,
    80, 53,
    80, 55,
    80, 56,
    80, 57,
    80, 58,
    80, 87,
    80, 89,
    80, 90,
    81, 13,
    81, 15,
    81, 53,
    81, 55,
    81, 56,
    81, 57,
    81, 58,
    81, 87,
    81, 89,
    81, 90,
    82, 53,
    82, 58,
    82, 75,
    83, 13,
    83, 15,
    83, 66,
    83, 68,
    83, 69,
    83, 70,
    83, 71,
    83, 80,
    83, 82,
    83, 85,
    83, 87,
    83, 90,
    84, 53,
    84, 55,
    84, 56,
    84, 57,
    84, 58,
    84, 87,
    84, 89,
    84, 90,
    85, 85,
    86, 53,
    86, 58,
    87, 13,
    87, 15,
    87, 53,
    87, 57,
    87, 66,
    87, 68,
    87, 69,
    87, 70,
    87, 71,
    87, 80,
    87, 82,
    87, 85,
    88, 13,
    88, 15,
    88, 53,
    88, 57,
    88, 66,
    89, 53,
    89, 56,
    89, 66,
    89, 68,
    89, 69,
    89, 70,
    89, 72,
    89, 77,
    89, 80,
    89, 82,
    89, 86,
    90, 13,
    90, 15,
    90, 53,
    90, 57,
    90, 66,
    90, 68,
    90, 69,
    90, 70,
    90, 71,
    90, 80,
    90, 82,
    90, 85,
    91, 53,
    92, 75
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -29, -29, -41, -41, -29, -29, -41, -41,
    15, -50, -50, -31, -10, -1, -50, -50,
    -31, -10, -1, -31, -26, -26, -26, -26,
    -41, -41, -12, -12, -16, -2, -16, -2,
    -26, -26, -5, -5, -5, -5, -20, 0,
    -11, -8, -18, -1, -5, -1, -1, -1,
    -14, -14, 0, 0, 0, 0, -10, -5,
    -5, -5, -15, -15, -5, -9, -5, -5,
    -7, -8, -5, -1, 5, -41, -41, -10,
    -15, 0, -1, 1, -1, -7, -7, -7,
    -7, -3, -1, -6, -6, -6, -6, -6,
    -6, -10, -6, -6, -6, -1, 0, -6,
    -11, -6, -11, -26, -26, 5, -11, -11,
    -11, -11, -20, -1, -16, -10, -21, -10,
    -5, -10, -15, -15, -5, -9, -5, -5,
    -7, -8, -5, -1, -51, -51, -15, -5,
    -3, -3, -8, -5, -8, -15, -15, -15,
    -5, 2, -9, -5, -5, -7, -8, -5,
    -1, -3, -3, -3, -3, -9, 0, -1,
    0, -5, -6, -3, -3, -3, -1, -3,
    -3, -1, 0, -3, 5, -3, -5, -5,
    -3, -3, -1, -1, 0, -1, -40, -40,
    -20, -5, -5, -5, -5, 5, 2, 2,
    -20, -15, -15, -15, -15, -5, -5, -15,
    -15, -15, -15, -15, -15, -11, 5, -16,
    -6, -1, -6, -6, -11, 0, -1, -36,
    -36, -10, -10, -11, -5, -5, -5, -5,
    -3, 2, -10, -5, -5, -5, -5, -5,
    -5, -5, -25, -25, -6, -6, -8, -5,
    -5, -5, -5, -1, -5, -5, -5, -5,
    -5, -5, -5, -1, -7, -7, -7, -7,
    -3, -1, -6, -6, -6, -6, -6, -6,
    -10, -6, -6, -6, -1, 0, -6, -11,
    -6, -11, -36, -36, -15, -15, -18, -8,
    -8, -8, -8, -5, 2, -21, -11, -11,
    -11, -24, -1, -1, -1, -11, -6, -11,
    -1, -6, -6, -1, -1, -1, -1, -1,
    -1, -1, -1, 20, -15, -1, -6, -3,
    -3, -3, -10, -10, -15, -5, -5, -6,
    -11, -3, -3, -3, -5, -5, -15, -5,
    -10, 5, 15, 10, 10, 10, 5, 5,
    9, 5, -8, -5, -5, -5, -8, -5,
    -5, -5, -5, -5, -5, -5, -5, -2,
    -2, -5, -2, -9, -1, -3, -3, 9,
    -5, 1, 1, -5, -10, -1, -1, -3,
    -3, -3, -5, -3, -3, -3, 0, -1,
    -1, 0, -1, -9, -1, -3, -3, -9,
    -1, -3, -3, -10, -10, -15, -5, -5,
    -6, -11, -3, -3, -3, -10, -10, -15,
    -5, -5, -6, -11, -3, -3, -3, -5,
    -1, 10, -25, -25, -10, -5, -5, -5,
    5, -5, -5, 5, 5, 5, -11, -5,
    -5, -1, -6, -1, 0, -1, 2, -15,
    -1, -25, -25, -6, -10, -8, 0, 0,
    0, 2, 0, 0, 2, -16, -16, -1,
    -6, -1, -10, -1, -1, -3, -3, -3,
    -5, -3, -3, -3, 0, -25, -25, -6,
    -10, -8, 0, 0, 0, 2, 0, 0,
    2, -11, 20
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 467,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_Font1 = {
#else
lv_font_t ui_font_Font1 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_FONT1*/

