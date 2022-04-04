#include "graphics.h"
#include <stdio.h>

int main(const int argc, const char* argv[]){

    if(argc < 3){
        printf("Follow the format is: main bg_img_name front_img_name\n");
    }
    else{
        InitOverlaying(argv[1], argv[2]);
    }

    return 0;
}
