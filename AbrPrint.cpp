#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#include "./utils.h"
#include "./dataprocessing.h"
#include "./filectrl.h"


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
	std::string filename;
	try {
		//Take in the filename (within the abr_files directory) to process
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
		//Generate the necessary things for rendering the graph from the file
		window = util::generateWindow();
		renderer = util::generateRenderer(window);
		visualizer = util::generateTexture(renderer);
		font = util::getFont("Consolas", 24);

		//Fill the background and print the header of the graph
		util::fill(renderer, visualizer, proc::ABR_BKGD_COLOR);
		util::printText(
			renderer, visualizer, filename, 75, 10, 24, 0, proc::ABR_GRAPH_COLOR1, font, nullptr
			);

		//Discover the filename index
		size_t fileindex = 0;
		for (size_t x = 0; x < labels.size(); x++)
			if (labels[x] == "FILE") fileindex = x;
		
		//Store the position of the graph on the screen
		SDL_Rect framepos = { 75, 110, util::IMG_W - 125, util::IMG_H - 300 };

		//Initialize some graph information
		proc::graphData_t graphInfo;
		graphInfo.framepos = framepos;
		graphInfo.fileList = table[fileindex];
		graphInfo.vertDivisions = 10;
		//Calculate the range of markers on the graph data
		proc::getDataRange(table, &graphInfo);

		std::vector<proc::graphBar_t> barsList = proc::generateBars(graphInfo, labels, table);
		proc::focusShortBars(&barsList);

		//Draw each of the bars on under the graph
		for (proc::graphBar_t bar : barsList)
			util::fillRect(renderer, visualizer, bar.barRect, bar.color);

		//Print the graph frame that will show behind the data
		proc::printGraphFrame(renderer, visualizer, graphInfo, font);

		proc::printKeys(renderer, visualizer, labels, graphInfo, font);

		//Render the graph onto the window
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