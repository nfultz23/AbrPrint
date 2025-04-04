#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#include "./utils.h"


using std::string; using std::vector; using std::ifstream;


int main(int argc, char** argv) {
	//Initialize SDL first. If SDL is going to fail on this run, you don't want it to happen
	// after the cycles have already been spent processing data
	try {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) throw "main(): " + (string)SDL_GetError();
		if (TTF_Init() < 0) throw "main(): " + (string)TTF_GetError();
	}
	catch (const char* err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (string err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (...) {
		std::cout << "Unknown error occurred" << std::endl;
		return 1;
	}

	//Gather the file's labels and populate a table for the data
	vector<string> labels;
	vector<vector<string>> table;
	try {
		//Take in the filename (within the abr_files directory) to process
		std::string filename;
		ifstream src = filectrl::loadFile(&filename);

		//Read in the entire header from the file being processed
		labels = proc::makeLabels(filename, &src);

		//Create a 2D vector table. The outer vector will be the columns, each identified
		// with the index of their title in the labels vector. The inner vector will be
		// the data itself
		table = proc::makeTable(filename, labels, &src);

		src.close();
	}
	catch (const char* err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (string err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (...) {
		std::cout << "Unknown error occurred" << std::endl;
		return 1;
	}

	SDL_Window* window; SDL_Renderer* renderer; SDL_Texture* visualizer; TTF_Font* font;
	try {
		window = util::generateWindow();
		renderer = util::generateRenderer(window);
		visualizer = util::generateTexture(renderer);

		font = util::getFont("Consolas", 60);
		util::printText(
			renderer, visualizer, "Hello world!", 100, 100, 14, { 255,255,255,255 }, font
			);

		util::renderTexture(renderer, visualizer);
	}
	catch (const char* err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (string err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (...) {
		std::cout << "Unknown error occurred" << std::endl;
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

	SDL_DestroyTexture(visualizer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);

	TTF_Quit();
	SDL_Quit();

	return 0;
}