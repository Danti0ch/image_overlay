#ifndef GRAPHICS_H
#define GRAPHICS_H

const int BUF_SIZE  = 256;
const int FONT_SIZE = 25;

const char FONT_NAME[1 << 8] = "arial.ttf";

typedef unsigned int uint;

const uint FRONT_X0 = 300;
const uint FRONT_Y0 = 250;

const char I          = 255,
           BIG1       = 0x80,
           LOW_A_POS  = 0x06,
           HIGH_A_POS = 0x0E;

int InitOverlaying(const char* bg_img_name, const char* front_img_name);

#endif //GRAPHICS_H
 