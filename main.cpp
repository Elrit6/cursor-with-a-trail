#define SDL_MAIN_HANDLED

#include <chrono>
#include <iostream>
#include "include/SDL.h"
#include "include/SDL_image.h"
#include <cassert>
#include <vector>


void printError(const std::string& error) {
	std::cerr << error << std::endl;
	std::cin.get();    
	return;    
}


class Sprite {
private:
	SDL_Texture* sprite;
	SDL_Renderer* renderer;

    SDL_Texture* load(const std::string& path) {
        SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
        if (!texture) {
            printError(IMG_GetError());
            return nullptr;
        }
        return texture;
    }	

public:
	SDL_Rect rect;

	Sprite(SDL_Renderer* renderer, const std::string& path, SDL_Rect rect) {
		this->renderer = renderer;
		sprite = load("sprites/" + path);
		this->rect = rect;
	}

	~Sprite() {
		if (sprite) SDL_DestroyTexture(sprite);
    }

	void display() {
		assert(sprite != nullptr); 
		SDL_RenderCopy(renderer, sprite, nullptr, &rect);
    }
};


class Window {
private:
	int deltaTime;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    Sprite* testSprite;

    void quit() {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        IMG_Quit();
        std::exit(0);    
    }

    void init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            printError(SDL_GetError());
        }

        if (IMG_Init(IMG_INIT_PNG) == 0) {
            printError(IMG_GetError());
        }

        window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN); 
        if (!window) {
            printError(SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            printError(SDL_GetError());
        }   
    }

    void loadSprites() {
		testSprite = new Sprite(renderer, "test.png", {0, 0, 80, 80});    	
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit();
            }
        }
    }

    void displaySprites() {
		testSprite->display();
    }

    void display() {
		SDL_RenderClear(renderer);
		handleEvents();
		displaySprites();
		SDL_RenderPresent(renderer);    	
    }

    void loop() {
    	std::chrono::steady_clock::time_point frameStart, frameEnd;
        while (1) {
        	frameStart = std::chrono::steady_clock::now();
            handleEvents();
            display();
            frameEnd = std::chrono::steady_clock::now();
            deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count();
        }
    }

public:
    Window() {
    	init();
    	loadSprites();

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

        loop();   
    }

	~Window() {
		quit();
	}	
};


int main(int argc, char const *argv[]) {
    Window window;
    return 0;
}
