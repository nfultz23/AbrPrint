#ifndef UTILS_H
#define UTILS_H

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <string>

namespace util {
	using std::string;

	/*Constructs an SDL_Window pointer that can be used to display the working surface for
	* debugging purposes.
	*
	* Precondition: SDL2 must already be initialized
	*
	* Returns a pointer to an SDL2 window object
	*/
	SDL_Window* generateWindow() {
		//Generate the SDL Window
		SDL_Window* window = SDL_CreateWindow(
			"AbrPrint Test",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			1000, 800,
			SDL_WINDOW_SHOWN
		);

		//Check the window was successfully created
		if (!window) throw ("util::generateWindow(): " + (string)SDL_GetError());

		return window;
	}
}

#endif