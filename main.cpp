#define SDL_MAIN_HANDLED

#include <chrono>
#include <fstream>
#include <iostream>
#include "include/SDL.h"
#include "include/SDL_image.h"
#include "include/json.hpp"
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
		sprite = load(path);
		this->rect = rect;
	}

	~Sprite() {
		if (sprite) SDL_DestroyTexture(sprite);
    }

	void display() {
		if (sprite != nullptr) SDL_RenderCopy(renderer, sprite, nullptr, &rect);
    }
};


class CursorTrail {
private:
    SDL_Renderer* renderer;

    Sprite* sprite;

public:
    CursorTrail(SDL_Renderer* renderer, const int x, const int y, const int size) {
        this->renderer = renderer;
        const int center = size / 2;
        sprite = new Sprite(renderer, "img/cursortrail.png", {x - center, y - center, size, size});     
    }

    void display() {
        sprite->display();
    }
};


class Cursor {
private:
    int winW, winH, spawnTime;
    int outX = 0;
    int outY = 0;
    int size = 100;
    float center = size / 2;
    std::vector<CursorTrail> trail;       

    SDL_Renderer* renderer;
 
    Sprite* sprite;

public:
    double timer = 0;

    Cursor(SDL_Renderer* renderer, const int trailElementsCount, const int spawnTime) {
        this->renderer = renderer;
        this->spawnTime = spawnTime;
        sprite = new Sprite(renderer, "img/cursor.png", {0, 0, size, size});
        SDL_GetRendererOutputSize(renderer, &winW, &winH);   
        for (int i = trailElementsCount; i; i--) {
            trail.push_back(CursorTrail(renderer, 0, 0, 0));
        }
    }

    void move() {
        int x, y;
        SDL_GetMouseState(&x, &y);

        sprite->rect.x = x - center;
        sprite->rect.y = y - center;

        if (timer > spawnTime) {
            trail.erase(trail.begin());
            trail.push_back(CursorTrail(renderer, x, y, size));
            timer = 0;
        }
    }

    void display() {
        for (CursorTrail& cursorTrail : trail) {
            cursorTrail.display();
        }
        sprite->display();        
    }
};


class Window {	
private:
    int width, height, cursorSize, trailElementsCount, trailSpawntime;
    double deltaTime;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    Cursor* cursor;

    void loadConf() {
        std::fstream confFile("config.json");
        nlohmann::json conf;
        confFile >> conf;
        width = conf["windowWidth"];
        height = conf["windowWidth"]; 
        trailElementsCount = conf["cursorTrailElements"];
        trailSpawntime = conf["cursorTrailSpawnTime"];
    }

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

        window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN); 
        if (!window) {
            printError(SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            printError(SDL_GetError());
        }   
    }

    void loadSprites() {
		cursor = new Cursor(renderer, trailElementsCount, trailSpawntime); 	
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit();
            }
        }
    }

    void update() {
        cursor->move();
        cursor->timer += deltaTime / 1000;
    }

    void displaySprites() {
		cursor->display();
    }

    void display() {
		SDL_RenderClear(renderer);
		handleEvents();
		displaySprites();
		SDL_RenderPresent(renderer);    	
    }

    void loop() {
    	std::chrono::steady_clock::time_point frameStart;
        while (1) {
        	frameStart = std::chrono::steady_clock::now();
            update();
            handleEvents();
            display();
            deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - frameStart).count());
        }
    }

public:
    Window() {
        loadConf();        
    	init();
    	loadSprites();

        SDL_ShowCursor(SDL_DISABLE);
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
