#ifndef GRAPHICS_H
#define GRAPHICS_H

const int BUF_SIZE  = 256;
const int FONT_SIZE = 25;

const char FONT_NAME[1 << 8] = "arial.ttf";

int InitOverlaying(const char* bg_img_name, const char* front_img_name);

#endif //GRAPHICS_H
