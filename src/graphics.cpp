#include "graphics.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <time.h>
#include <assert.h>
#include <immintrin.h>

using namespace sf;

//----------------------LOCAL-FUNCTIONS-DECLARATION-----------------------//

void draw_fps(RenderWindow* window, uint framerate);
void draw_bg(Vertex* pixels, Image* bg_img);
void draw_front(Vertex* pixels, Image* bg_img, Image* front_img, uint x0, uint y0);
//----------------------PUBLIC-FUNCTIONS-DEFINITIONS----------------------//

int InitOverlaying(const char* bg_img_name, const char* front_img_name){

    assert(bg_img_name    != NULL);
    assert(front_img_name != NULL);

    Texture texture;
    Image bg_img;
    Image front_img;

    if(!bg_img.loadFromFile(bg_img_name)){
        printf("Error opening %s\n", bg_img_name);
        return 1;
    }
    
    if(!front_img.loadFromFile(front_img_name)){
        printf("Error opening %s\n", front_img_name);
        return 1;
    }

    Vector2u bg_img_size    = bg_img.getSize();
    Vector2u front_img_size = front_img.getSize();
    
    RenderWindow window(VideoMode(bg_img_size.x, bg_img_size.y), "KEKIS");

    uint framerate_counter  = 0;
    uint cur_framerate      = 0;
    bool is_overlaying      = false;

    clock_t init_time = clock();

    Vertex* pixels = (Vertex*)calloc(bg_img_size.x * bg_img_size.y, sizeof(Vertex));

        for(int y = 0; y < bg_img_size.y; y++){
        for(int x = 0; x < bg_img_size.x; x++){

            pixels[y * bg_img_size.x + x] = Vertex(Vector2f(x, y), Color::Red);
        }
    }

    while (window.isOpen()){

        is_overlaying = false;
        Event cur_event;

        while (window.pollEvent(cur_event)){
    
            if(cur_event.type == Event::Closed) window.close();
            if(cur_event.type == Event::KeyPressed){
                if(Keyboard::isKeyPressed(Keyboard::Q)){
                    is_overlaying = true;
                }
            }
        }
    
        window.clear();
        draw_bg(pixels, &bg_img);
        draw_front(pixels, &bg_img, &front_img, 300, 250);

        window.draw(pixels, bg_img_size.x * bg_img_size.y, Points);

        clock_t end_time = clock();
        framerate_counter++;

        if(end_time - init_time >= CLOCKS_PER_SEC){
            cur_framerate = framerate_counter;
            framerate_counter = 0;
            init_time = clock();
        }
        
        draw_fps(&window, cur_framerate);

        window.display();
    }

    return 0;
}
//--------------------------------------------//

//----------------------LOCAL-FUNCTIONS-DEFINITIONS----------------------//

void draw_fps(RenderWindow* window, uint framerate){
    
    assert(window != NULL);

    char buffer[BUF_SIZE] = "";
    snprintf(buffer, sizeof(buffer), "%u\n", framerate);
    Font font;
    
    font.loadFromFile(FONT_NAME);

    Text text(buffer, font);
    text.setCharacterSize(FONT_SIZE);
    text.setStyle(Text::Bold);
    text.setFillColor(Color::Red);

    window->draw(text);

    return;
}
//--------------------------------------------//

void draw_bg(Vertex* pixels, Image* bg_img){
    
    assert(pixels != NULL);
    assert(bg_img != NULL);
    
    Vector2u bg_img_size = bg_img->getSize();

    for(int y = 0; y < bg_img_size.y; y++){
        for(int x = 0; x < bg_img_size.x; x++){
            pixels[y * bg_img_size.x + x].color = bg_img->getPixel(x, y);
        }
    }
    return;
}
//--------------------------------------------//

void draw_front(Vertex* pixels, Image* bg_img, Image* front_img, uint x0, uint y0){

    assert(pixels    != NULL);
    assert(bg_img    != NULL);
    assert(front_img != NULL);

    Vector2u bg_img_size = bg_img->getSize();
    Vector2u front_img_size = front_img->getSize();

    for(int y = y0; y < front_img_size.y + y0; y++){
        for(int x = x0; x < front_img_size.x + x0; x++){

            Color bg_pix    = bg_img->getPixel(x, y);
            Color front_pix = front_img->getPixel(x - x0, y - y0);

            Color new_pix   = Color(
                (bg_pix.r * (255 - front_pix.a) + front_pix.r * front_pix.a) >> 8,
                (bg_pix.b * (255 - front_pix.a) + front_pix.b * front_pix.a) >> 8,
                (bg_pix.g * (255 - front_pix.a) + front_pix.g * front_pix.a) >> 8);

            pixels[y * bg_img_size.x + x].color = new_pix;
        }
    }

    return;
}
//--------------------------------------------//

void draw_front_sse(Vertex* pixels, Image* bg_img, Image* front_img, uint x0, uint y0){
    
    assert(pixels    != NULL);
    assert(bg_img    != NULL);
    assert(front_img != NULL);
    
    Vector2u bg_img_size = bg_img->getSize();
    Vector2u front_img_size = front_img->getSize();

    for(int y = y0; y < front_img_size.y + y0; y++){
        for(int x = x0; x < front_img_size.x + x0; x+=4){

            uint bg_pixels_arr[4] = {bg_pix.r}
            Color new_pix   = Color(
                (bg_pix.r * (255 - front_pix.a) + front_pix.r * front_pix.a) >> 8,
                (bg_pix.b * (255 - front_pix.a) + front_pix.b * front_pix.a) >> 8,
                (bg_pix.g * (255 - front_pix.a) + front_pix.g * front_pix.a) >> 8);

            pixels[y * bg_img_size.x + x].color = new_pix;
        }
    }

    return;
}
//--------------------------------------------//

uint* get_pixel_arr(Image* img){

    assert(img    != NULL);
    assert(arr    != NULL);

    Vector2u img_size = img->getSize();

    uint* arr = (uint*)calloc(img_size.x * img_size.y, sizeof(uint));
    for(uint y = 0; y < img_size.y; y++){
        for(uint x = 0; x < img_size.x; x++){

            Color cur_pix = img->GetPixel(x, y);

            arr[y * img_size.x + x] = 
                    cur_pix.b + (cur_pix.g << 8) + (cur_pix.r << 16) + (cur_pix.a << 24);
        }
    }
    return;
}

