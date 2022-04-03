#include "graphics.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <time.h>
#include <assert.h>
#include <immintrin.h>

using namespace sf;

int InitOverlaying(const char* bg_img_name, const char* front_img_name)){

    assert(bg_img_name    != NULL);
    assert(front_img_name != NULL);

    Texture texture;
    Image bg_img;
    Image front_img;

    if(!img.loadFromFile(bg_img_name)){
        printf("Error opening %s\n", bg_img_name);
        return 1;
    }
    
    if(!img.loadFromFile(front_img_name)){
        printf("Error opening %s\n", front_img_name);
        return 1;
    }


    sf::RenderWindow window(sf::VideoMode(img.getSize().x, img.getSize().y), "LOL");
    
    texture.loadFromImage(img);
    sf::Sprite sprite;

    sprite.setTexture(texture);

    sf::Image Image;
    if (!Image.loadFromFile("AskhatCat.bmp"))
    {
        // Error...
    }

        sf::Texture texture2;
    texture2.loadFromImage(Image);
    sf::Sprite sprite2;

    sprite2.setTexture(texture2);

    for(int y = 0; y < Image.getSize().y; y++){
        for(int x = 0; x < Image.getSize().x, x++){

        }
    }
    
    while(window.isOpen()){
        sf::Event cur_event;

        while(window.pollEvent(cur_event)){

            if(cur_event.type == sf::Event::Closed) window.close();
        }


        window.clear();
        window.draw(sprite);
        window.draw(sprite2);
        window.display();
    }

    return 0;
}
