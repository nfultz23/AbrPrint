#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#define CLEAN_EXIT_CODE 0;
#define SDL_EXIT_CODE 1;
#define FILE_EXIT_CODE 2;
#define DATA_EXIT_CODE 3;

using std::string;

int main(int argc, char** argv) {
	//Initialize SDL first. If SDL is going to fail on this run, you don't want it to happen
	// after the cycles have already been spent processing data
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not initialize, std_Error: " << SDL_GetError() << std::endl;
		return SDL_EXIT_CODE;
	}

	//Take in the filename (within the abr_files directory) to process
	std::cout << "Please enter the file to process:" << std::endl << "?> ";
	string filename; std::cin >> filename;

	//Open the file and ensure that it's ready for reading
	std::ifstream src;
	src.open("./abr_files/" + filename, std::ios::in);
	if (!src.is_open()) {
		std::cout << "Error opening " << filename << ", exiting..." << std::endl;
		return FILE_EXIT_CODE;
	} else
		std::cout << "File " << filename << " successfully opened, reading headers..." << std::endl << std::endl;

	//Read in the entire header from the file being processed
	char header[4096];
	if (!src.getline(header, sizeof(header))) {
		std::cout << filename << " appears to be empty, exiting..." << std::endl;
		return DATA_EXIT_CODE;
	}

	//This situation has no header to label the data in the file
	if (header[0] != '#') {
		std::cout << "Imma be real, I have no idea how to handle this situation" << std::endl;
		return DATA_EXIT_CODE;
	}

	//Parse through the header line and create a vector to store the header labels
	std::vector<string> labels; string currHeader = "";
	for (int x = 1; x < strlen(header); x++) {
		//If a delimiter is found, separate it and push it into the label vector
		if (header[x] == '\t' || x == strlen(header) - 1) {
			labels.push_back(currHeader);
			currHeader = "";
			continue;
		}
		//Otherwise, add the character to the compiled header
		currHeader += header[x];
	}

	//Create a 2D vector table. The outer vector will be the columns, each identified
	// with the index of their title in the labels vector. The inner vector will be
	// the data itself
	std::vector<std::vector<string>> table;
	for (int x = 0; x < labels.size(); x++) table.push_back(std::vector<string>());

	//Iterate through the file to populate the table
	std::string currEntry;
	int i = 0;
	while (src >> currEntry) {
		table[i].push_back(currEntry);
		i++;
		i %= labels.size();
	}

	//Clean up the absolute file paths from the source file
	int fileIndex = 0;
	for (int x = 0; x < labels.size(); x++) {
		if (labels[x] == "FILE") fileIndex = x; break;
	}

	//Iterate through the file column and erase the absolute file paths
	for (int x = 0; x < table[fileIndex].size(); x++) {
		//Find the index of the string where the file path ends
		string currString = table[fileIndex][x];
		int i = currString.length() - 1;
		while (currString[i-1] != '/' && i > 0) i--;

		//Store the filename substring
		table[fileIndex][x] = currString.substr(i, currString.length());
	}

	src.close();

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

	return CLEAN_EXIT_CODE;
}