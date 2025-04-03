#ifndef UTILS_H
#define UTILS_H

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <string>

#include "./filectrl.h"
#include "./dataprocessing.h"

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
		if (!window) throw "util::generateWindow(): " + (string)SDL_GetError();

		return window;
	}

	/*Constructs an SDL_Renderer pointer that can be used to render images onto textures
	* 
	* Precondition: SDL2 must already be initialized AND window != nullptr
	* 
	* Param window is a pointer to the SDL2 window that the renderer will render to
	* Returns a pointer to the new SDL2 Renderer
	*/
	SDL_Renderer* generateRenderer(SDL_Window* window) {
		//Generate the SDL Renderer
		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);

		//Ensure the renderer was successfully created
		if (!renderer) throw "util::generateRenderer(): " + (string)SDL_GetError();

		return renderer;
	}

	/*Constructs an SDL_Testure pointer that will have all data printed onto it for
	* the duration of the program
	* 
	* Precondition: SDL2 must already be initialized AND renderer != nullptr
	* Postcondition: renderer will be retargeted to render to the window
	* 
	* Param renderer is the renderer that the texture will link to
	* Returns a pointer to a texture created, attached to the renderer
	*/
	SDL_Texture* generateTexture(SDL_Renderer* renderer) {
		//Generate the SDL Texture
		SDL_Texture* texture =
			SDL_CreateTexture(
				renderer,
				SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_TARGET,
				1000, 800);

		//Ensure that the texture was created successfully
		if (!texture) throw "util::generateTexture(): " + (string)SDL_GetError();

		return texture;
	}


	/*Renders a provided texture to the window's surface
	* 
	* Precondition: SDL2 is already initialized AND renderer != nullptr AND texture != nullptr
	* 
	* Param renderer is the renderer that will handle render actions to the display window
	* Param texture is the texture being rendered onto the display window0
	*/
	void renderTexture(SDL_Renderer* renderer, SDL_Texture* texture) {
		SDL_Rect rect;
		rect.x = 0; rect.y = 0;
		rect.w = 1000; rect.h = 800;
		
		try {
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			SDL_RenderPresent(renderer);
		} catch (...) {
			throw "util::renderTexture(): " + (string)SDL_GetError();
		}

		return;
	}

	struct lineSegment {
		int x0, y0;
		int x1, y1;
		int r, g, b, a;
	};

	/*
	*/
	void renderLine(SDL_Renderer* renderer, SDL_Texture* texture, lineSegment line) {
		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, line.r, line.g, line.b, line.a);
		
		if (SDL_RenderDrawLine(renderer, line.x0, line.y0, line.x1, line.y1) != 0)
			throw "util::renderTest(): " + (string)SDL_GetError();

		if (SDL_SetRenderTarget(renderer, NULL) != 0)
			throw "util::renderTest(): " + (string)SDL_GetError();

		return;
	}
}

#endif