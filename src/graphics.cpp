#include "graphics.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <time.h>
#include <assert.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <string.h>

#pragma GCC optimize("Ofast")
//#pragma GCC target("avx, avx2, fma")

using namespace sf;

#define max(a, b) (a) > (b) ? (a) : (b)
#define min(a, b) (b) > (a) ? (a) : (b)

const char I = 255u,
           Z = 0x80u,
           meow1 =  0x00,
           meow2 =  0x08;

const __m128i   _0 =                  _mm_set_epi8 (0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
const __m128i _255 =                  _mm_set_epi8 (I,I,I,I, I,I,I,I, I,I,I,I, I,I,I,I);

//----------------------LOCAL-FUNCTIONS-DECLARATION-----------------------//

void fill_img(Uint8* arr, Image* img, Vector2u* arr_size, uint x_pos, uint y_pos);
void draw_fps(RenderTexture* window, uint framerate, uint max_framerate, uint  min_framerate);
void draw_bg(Vertex* pixels, Vertex* bg_pixels, Vector2u* bg_img_size);
void draw_front(Vertex* pixels, Vertex* bg_pixels, Vertex* front_pixels, Vector2u bg_img_size, Vector2u front_img_size, uint x0, uint y0);
void set_bg_arr(Uint8* dest, Uint8* src, Vector2u* size);
void update_pixels(Uint8* dest, Uint8* bg_pixels, Uint8* front_pixels, Vector2u* size, bool is_overlaying);
void update_pixels_simd(Uint8* dest, Uint8* bg_pixels, Uint8* front_pixels, Vector2u* size, bool is_overlaying);
//----------------------PUBLIC-FUNCTIONS-DEFINITIONS----------------------//

// TODO: запилить simd часть
// TODO: рефактор

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
    
    Uint8* pixels = (Uint8*)calloc(bg_img_size.x * bg_img_size.y << 2, sizeof(Uint8));
    
    Uint8* bg_pixels    = (Uint8*)calloc(bg_img_size.x * bg_img_size.y << 2, sizeof(Uint8));
    Uint8* front_pixels = (Uint8*)calloc(bg_img_size.x * bg_img_size.y << 2, sizeof(Uint8));

    fill_img(bg_pixels, &bg_img, &bg_img_size, 0, 0);
    fill_img(front_pixels, &front_img, &bg_img_size, 300, 250);

    sf::RenderTexture texture_to_draw;
    texture_to_draw.create(bg_img_size.x, bg_img_size.y);
    
    Texture img_texture;
    img_texture.loadFromImage(bg_img);

    Sprite img_sprite;

    RenderWindow window(VideoMode(bg_img_size.x, bg_img_size.y), "KEKIS");

    uint framerate_counter  = 0;
    
    uint max_framerate      = 0;
    uint min_framerate      = 0;
    
    uint cur_framerate      = 0;
    bool is_overlaying      = false;

    clock_t init_time = clock();

    while (window.isOpen()){

        Event cur_event;

        while (window.pollEvent(cur_event)){
            if(cur_event.type == Event::Closed) window.close();
            if(cur_event.type == Event::KeyPressed){
                if(Keyboard::isKeyPressed(Keyboard::Q)){
                    is_overlaying = true;
                }
                else if(Keyboard::isKeyPressed(Keyboard::E)){
                    is_overlaying = false;
                }
            }
        }
    
        update_pixels_simd(pixels, bg_pixels, front_pixels, &bg_img_size, is_overlaying);

        //clock_t end_time = clock();
        framerate_counter++;

        if(clock() - init_time >= CLOCKS_PER_SEC){
            cur_framerate = framerate_counter;
            framerate_counter = 0;
            init_time = clock();
            max_framerate = max(max_framerate, cur_framerate);
            //min_framerate = min(min_framerate, cur_framerate);
            printf("cur: %u\nmax: %u\n===================\n", cur_framerate, max_framerate);
        }
        
        img_texture.update(pixels);
        img_sprite.setTexture(img_texture);
        texture_to_draw.draw(img_sprite);

        //draw_fps(&texture_to_draw, cur_framerate, max_framerate, min_framerate);

        texture_to_draw.display();

        window.clear();

        sf::Sprite sprite(texture_to_draw.getTexture());
        window.draw(sprite);

        window.display();
    }

    free(bg_pixels);
    free(front_pixels);
    free(pixels);

    return 0;
}
//--------------------------------------------//

//----------------------LOCAL-FUNCTIONS-DEFINITIONS----------------------//

void draw_fps(RenderTexture* window, uint framerate, uint max_framerate, uint  min_framerate){
    
    assert(window != NULL);

    char buffer[BUF_SIZE] = "";
    snprintf(buffer, sizeof(buffer), "cur: %u\nmax: %u\nmin: %u\n", framerate, max_framerate, min_framerate);
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

void draw_bg(Vertex* pixels, Vertex* bg_pixels, Vector2u* bg_img_size){
    
    assert(pixels != NULL);
    assert(bg_pixels != NULL);
    
    int maxY = bg_img_size->y;
    int maxX = bg_img_size->x;

    for(int y = 0; y < maxY; y++){
        for(int x = 0; x < maxX; x++){
            pixels[y * maxX + x].color = bg_pixels[y * maxX + x].color;
        }
    }

    return;
}
//--------------------------------------------//

void fill_img(Uint8* arr, Image* img, Vector2u* arr_size, uint x_pos, uint y_pos){

    assert(arr != NULL);
    assert(img != NULL);

    const Uint8* p_img_arr = img->getPixelsPtr();

    Vector2u img_size = img->getSize();
    
    for(uint y = y_pos; y < img_size.y + y_pos; y++){

        Uint8* dest = arr + ((y * arr_size->x + x_pos) << 2);
        const Uint8* src  = p_img_arr + (((y - y_pos) * img_size.x) << 2);

        memcpy(dest, src, img_size.x * 4);
    }

    return;
}
//--------------------------------------------//

void set_bg_arr(Uint8* dest, Uint8* src, Vector2u* size){

    assert(dest != NULL);
    assert(src  != NULL);
    assert(size != NULL);

    memcpy(dest, src, size->x * size->y * 4);

    return;
}
//--------------------------------------------//

void update_pixels(Uint8* dest, Uint8* bg_pixels, Uint8* front_pixels, Vector2u* size, bool is_overlaying){

    assert(dest         != NULL);
    assert(bg_pixels    != NULL);
    assert(front_pixels != NULL);
    assert(size  != NULL);

    if(!is_overlaying){
        memcpy(dest, bg_pixels, size->x * size->y * 4);
        return;
    }

    for(uint y = 0; y < size->y; y++){
        for(uint x = 0; x < size->x; x++){

            uint offset = (y * size->x + x) << 2;

            Uint8 cur_a = front_pixels[offset + 3];

            Uint8 r = bg_pixels[offset] * (255 - cur_a) + front_pixels[offset] * cur_a >> 8;
            Uint8 g = bg_pixels[offset + 1] * (255 - cur_a) + front_pixels[offset + 1] * cur_a >> 8;
            Uint8 b = bg_pixels[offset + 2] * (255 - cur_a) + front_pixels[offset + 2] * cur_a >> 8;
            
            dest[offset] = r;
            dest[offset + 1] = g;
            dest[offset + 2] = b;
        }
    }

    return;
}

//--------------------------------------------//


void update_pixels_simd(Uint8* dest, Uint8* bg_pixels, Uint8* front_pixels, Vector2u* size, bool is_overlaying){

    assert(dest         != NULL);
    assert(bg_pixels    != NULL);
    assert(front_pixels != NULL);
    assert(size  != NULL);

    if(!is_overlaying){
        memcpy(dest, bg_pixels, size->x * size->y * 4);
        return;
    }

    for(uint y = 0; y < size->y; y++){
        for(uint x = 0; x < size->x; x+=4){

            uint offset = (y * size->x + x) << 2;

            // uint8* -> _m128i
            __m128i bg_pixels_low    = _mm_loadu_si128((__m128i*)(bg_pixels + offset));
            __m128i front_pixels_low = _mm_loadu_si128((__m128i*)(front_pixels + offset));

            // bg_h = bg_l >> 16
            __m128i bg_pixels_high    = (__m128i)_mm_movehl_ps((__m128)_0, (__m128)bg_pixels_low);
            __m128i front_pixels_high = (__m128i)_mm_movehl_ps((__m128)_0, (__m128)front_pixels_low);
            
            // bytes -> words
            bg_pixels_low    = _mm_cvtepi8_epi16(bg_pixels_low);
            front_pixels_low = _mm_cvtepi8_epi16(front_pixels_low);

            bg_pixels_high      = _mm_cvtepi8_epi16(bg_pixels_high);
            front_pixels_high   = _mm_cvtepi8_epi16(front_pixels_high);

            // make [0, a, 0, a, ...]
            static const __m128i moveA = _mm_set_epi8(Z, meow1, Z, meow1, Z, meow1, Z, meow1,
                                                  Z, meow2, Z, meow2, Z, meow2, Z, meow2);

            __m128i a = _mm_shuffle_epi8 (front_pixels_low,  moveA);
            __m128i A = _mm_shuffle_epi8 (front_pixels_high, moveA);

            // a * front + (255 - a) * bg
            front_pixels_low  = _mm_mullo_epi16(front_pixels_low, a);
            front_pixels_high = _mm_mullo_epi16(front_pixels_high, a);
            
            bg_pixels_low  = _mm_mullo_epi16(bg_pixels_low, _mm_sub_epi16(_255, a));
            bg_pixels_high = _mm_mullo_epi16(bg_pixels_high, _mm_sub_epi16(_255, A));
            
            __m128i sum = _mm_add_epi16(front_pixels_low, bg_pixels_low); 
            __m128i SUM = _mm_add_epi16(front_pixels_high, bg_pixels_high);
            
            // words -> bytes

            static const __m128i moveSum = _mm_set_epi8 (Z, Z, Z, Z, Z, Z, Z, Z,
            0x01, 0x03, 0x05, 0x07, 0x09, 0xB, 0xD, 0xF);

            sum = _mm_shuffle_epi8 (sum,  moveA);
            SUM = _mm_shuffle_epi8 (SUM,  moveA);
            
            // to_color

            __m128i color = (__m128i) _mm_movelh_ps((__m128) sum, (__m128) SUM);

            _mm_store_si128((__m128i*) (dest + offset), color);

            
        }
    }

    return;
}
//--------------------------------------------//

/*
void draw_front_sse(Vertex* pixels, Image* bg_img, Image* front_img, uint x0, uint y0){
    
    assert(pixels    != NULL);
    assert(bg_img    != NULL);
    assert(front_img != NULL);
    
    Vector2u bg_img_size = bg_img->getSize();
    Vector2u front_img_size = front_img->getSize();

    for(int y = y0; y < front_img_size.y + y0; y++){
        for(int x = x0; x < front_img_size.x + x0; x+=4){

            uint bg_pixels_arr[4] = {bg_img[x]->GetPixel().toInteger(),
                                     bg_img[x + 1]->GetPixel().toInteger(),
                                     bg_img[x + 2]->GetPixel().toInteger(),
                                     bg_img[x + 3]->GetPixel().toInteger()}

            uint front_pixels_arr[4] = {bg_img[x]->GetPixel().toInteger(),
                                        bg_img[x + 1]->GetPixel().toInteger(),
                                        bg_img[x + 2]->GetPixel().toInteger(),
                                        bg_img[x + 3]->GetPixel().toInteger()}

            __m128i bg_pixels_low    = _mm_loadu_si128((__m128i*)bg_pixels_arr);
            __m128i front_pixels_low = _mm_loadu_si128((__m128i*)front_pixels_arr);

            __m128i bg_pixels_high    = _mm_srai_epi32(bg_pixels_low, 8*8);
            __m128i front_pixels_high = _mm_srai_epi32(front_pixels_low, 8*8);

            bg_pixels_low = _mm_cvtepi8_epi16(bg_pixels_low);
            front_pixels_low = _mm_cvtepi8_epi16(front_pixels_low);

            bg_pixels_high = _mm_cvtepi8_epi16(bg_pixels_high);
            front_pixels_high = _mm_cvtepi8_epi16(front_pixels_high);

            __m128i moveA = _mm_set_epi8(0x80u, )
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

*/
