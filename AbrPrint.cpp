#include <iostream>
#include <fstream>

#define SDL_MAIN_HANDLED
#include <SDL.h>

using std::string;

int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not initialize, std_Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	/*
	SDL_Window* window = SDL_CreateWindow(
		"AbrPrint Test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1000, 800,
		SDL_WINDOW_SHOWN
	);

	if (!window) {
		std::cout << "SDL could not create window, std_Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	bool RUNNING = true;
	while (RUNNING) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN)
				if (event.key.keysym.sym == SDLK_q) RUNNING = false;
			if (event.type == SDL_QUIT) RUNNING = false;
		}
	}

	SDL_DestroyWindow(window);
	window = NULL;
	//*/

	SDL_Quit();

	return 0;
}