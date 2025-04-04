#ifndef UTILS_H
#define UTILS_H


#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>

#include <string>
#include <iostream>


namespace util {
	using std::string;

	//This struct contains information about a color for rendering
	struct color_t {
		uint16_t r, g, b, a;
	};

	//This struct contains information about a polygon
	struct polygon_t {
		SDL_Point* pointArr;
		int numPoints;
	};

	static string ABR_TYPEFACE_DIR = "./typefaces/";
	static int IMG_W = 1000;
	static int IMG_H = 800;


	/*Constructs an SDL_Window pointer that can be used to display the working surface for
	* debugging purposes.
	*
	* Precondition: SDL2 must already be initialized
	*
	* Returns a pointer to an SDL2 window object
	*/
	static SDL_Window* generateWindow() {
		//Generate the SDL Window
		SDL_Window* window = SDL_CreateWindow(
			"AbrPrint",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			IMG_W, IMG_H,
			SDL_WINDOW_SHOWN
		);

		//Check the window was successfully created
		if (!window) throw "util::generateWindow(): " + (string)SDL_GetError();

		//Raise the window and return
		SDL_RaiseWindow(window);
		return window;
	}


	/*Constructs an SDL_Renderer pointer that can be used to render images onto textures
	* 
	* Precondition: SDL2 must already be initialized AND window != nullptr
	* 
	* Param window is a pointer to the SDL2 window that the renderer will render to
	* Returns a pointer to the new SDL2 Renderer
	*/
	static SDL_Renderer* generateRenderer(SDL_Window* window) {
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
	static SDL_Texture* generateTexture(SDL_Renderer* renderer) {
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
	static void renderTexture(SDL_Renderer* renderer, SDL_Texture* texture) {
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


	/*Renders a line segment onto the provided surface
	* 
	* Precondition: SDL2 must be initialized AND renderer != nullptr AND texture != nullptr
	* Postcondition: texture containes the line segment provided OW texture = #texture
	*		renderer = #renderer
	* 
	* Param renderer is a pointer to the SDL renderer that will print the line segment
	* Param texture is the texture that the line will be rendered onto
	* Param p0 is an SDL_Point that is the first in the line
	* Param p1 is an SDL_Point that is the second in the line
	*/
	static void drawLine(
		SDL_Renderer* renderer, SDL_Texture* texture, SDL_Point p0, SDL_Point p1, color_t color
		) {
		//Set the render target to the texture and set the render color
		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
		
		//Draw the line to the surface, exiting if this fails
		if (SDL_RenderDrawLine(renderer, p0.x, p0.y, p1.x, p1.y) != 0)
			throw "util::renderTest(): " + (string)SDL_GetError();

		//Reset the render target to the provided window, exiting if this fails
		if (SDL_SetRenderTarget(renderer, NULL) != 0)
			throw "util::renderTest(): " + (string)SDL_GetError();

		return;
	}


	/*Renders a polygon from a color set and a series of points
	* 
	* Precondition: SDL2 must be initialized AND renderer != nullptr AND
	*		texture != nullptr AND points != nullptr
	* Postcondition: texture contains the polygon outline OW texture = #texture
	*		renderer = #renderer
	* 
	* Param renderer is a pointer to the SDL renderer that will print the polygon
	* Param texture is the texture that the polygon will be rendered onto
	* Param polygon is a struct that stores the points contained in the polygon
	* Param color is the color of the lines on the polygon
	*/
	static void drawPolygon(
		SDL_Renderer* renderer, SDL_Texture* texture, polygon_t polygon, color_t color
		) {
		//Draw each line in the polygon
		for (int x = 0; x < polygon.numPoints - 1; x++) {
			drawLine(renderer, texture, polygon.pointArr[x], polygon.pointArr[x + 1], color);
		}

		return;
	}


	/*Renders a rectangle from a color set and an SDL rect
	*
	* Precondition: SDL2 must be initialized AND renderer != nullptr AND
	*		texture != nullptr AND points != nullptr
	* Postcondition: texture contains the polygon outline OW texture = #texture
	*		renderer = #renderer
	*
	* Param renderer is a pointer to the SDL renderer that will print the polygon
	* Param texture is the texture that the polygon will be rendered onto
	* Param rect is the SDL_Rect that will be drawn
	* Param color is the color of the lines on the polygon
	*/
	static void fillRect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect, color_t color) {
		//Set the render target to the texture and set the render color
		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

		if (SDL_RenderFillRect(renderer, &rect) != 0)
			throw "util::fillRect(): " + (string)SDL_GetError();

		if (SDL_SetRenderTarget(renderer, NULL) != 0)
			throw "util::fillRect(): " + (string)SDL_GetError();

		return;
	}


	/*Fills the screen with an input color
	* 
	* Precondition: SDL2 must be initialized AND renderer != nullptr AND
	*		texture != nullptr AND points != nullptr
	* Postcondition: texture is filled with the input color, renderer = #renderer
	* 
	* Param renderer is a pointer to the SDL renderer that will be coloring
	* Param texture is a pointer to the texture being colored
	* Param color is the color filling the surface
	*/
	static void fill(SDL_Renderer* renderer, SDL_Texture* texture, color_t color) {
		//Set the render target to the texture and set the render color
		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

		//Fill the texture with the color
		if (SDL_RenderFillRect(renderer, NULL) != 0)
			throw "util::fill(): " + (string)SDL_GetError();

		if (SDL_SetRenderTarget(renderer, NULL) != 0)
			throw "util::fill(): " + (string)SDL_GetError();

		return;
	}

	/*Accesses a font from the typeface directory
	* 
	* Precondition: SDL2 must be initialized AND SDL_TTF must be initialized AND
	*		there must be a Truetype Font in the typeface directory
	* 
	* Param fontName is the name of the font, matching the name of the Truetype
	*		File (without the extension)
	* Param size is the size of the font being loaded
	* Returns a pointer to the TTF_Font that contains the loaded typeface information
	*/
	static TTF_Font* getFont(string fontName, int size) {
		//Create the font pointer
		TTF_Font* font = TTF_OpenFont((ABR_TYPEFACE_DIR + fontName + ".ttf").c_str(), 24);
		if (!font) throw "util::getFont(): " + (string)TTF_GetError();

		//Return the font
		return font;
	}


	/*Prints input text onto a surface
	* 
	* Precondition: SDL2 must already be initialized AND SDL_TTF must already be initialized AND
	*		renderer != nullptr AND texture != nullptr, AND font != nullptr AND size > 0
	* Postcondition: texture contains the text placed at the positions provided and 
	*		size specified
	* 
	* Param renderer is the SDL_Renderer that will be rendering the text onto the texture
	* Param texture is the surface that the text will be rendered onto
	* Param text is the string that the text will be printed
	* Param x is the horizontal position of the text on the screen
	* Param y is the vertical position of the text on the screen
	* Param size is the height of the text on the screen
	* Param angle is the angle (in degrees) that the text is rotated
	* Param color is the color_t object containing the hue of the text (the alpha value
	*		will be omitted in this case)
	* Param font is the TTF_Font that the text will be printed on
	*/
	static void printText(
		SDL_Renderer* renderer, SDL_Texture* texture, string text,
		int x, int y, int size, int angle, color_t color, TTF_Font* font
		) {
		//Set the render target to the texture and set the render color
		SDL_SetRenderTarget(renderer, texture);

		//Convert the input color_t value to an SDL_Color object
		SDL_Color clr = { color.r, color.g, color.b };

		//Create a texture with the text rendered onto it
		SDL_Surface* finSurface = TTF_RenderText_Blended(font, text.c_str(), clr);
		if (!finSurface) throw "util::printText(): " + (string)TTF_GetError();
		SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, finSurface);
		if (!textTexture) throw "util::printText(): " + (string)SDL_GetError();

		//Print the text onto the input texture
		double ratio = (double)size / finSurface->h;
		SDL_Rect srcRect = { 0, 0, finSurface->w, finSurface->h };
		SDL_Rect destRect = { x, y, (int)(ratio * finSurface->w), size };
		//SDL_Point destCenter = { destRect.w / 2 + destRect.x, destRect.h / 2 + destRect.y };
		//SDL_Point destCenter = { srcRect.w / 2 + srcRect.x, srcRect.h / 2 + srcRect.y };
		SDL_Point destCenter = { 0, 0 };
		//if (SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect) != 0)
		//	throw "util::printText(): " + (string)SDL_GetError();
		if (SDL_RenderCopyEx(
			renderer, textTexture, &srcRect, &destRect, (double)angle, &destCenter, SDL_FLIP_NONE
			) != 0) throw "util::printText(): " + (string)SDL_GetError();

		//Cleanup the surfaces
		SDL_DestroyTexture(textTexture);
		SDL_FreeSurface(finSurface);

		//Redirect the renderer to the window
		SDL_SetRenderTarget(renderer, NULL);

		return;
	}

}


#endif