#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#include "./utils.h"
#include "./filectrl.h"
#include "./dataprocessing.h"


using std::string; using std::vector; using std::ifstream;


int main(int argc, char** argv) {
	//Initialize SDL first. If SDL is going to fail on this run, you don't want it to happen
	// after the cycles have already been spent processing data
	try { if (SDL_Init(SDL_INIT_VIDEO) < 0) throw SDL_GetError(); }
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

	/*
	SDL_Window* window = util::generateWindow(); 
	
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