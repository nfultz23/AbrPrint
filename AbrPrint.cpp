#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <utility>

#include "./utils.h"
#include "./dataprocessing.h"
#include "./filectrl.h"


using std::string; using std::vector; using std::ifstream;

namespace util {
	std::string ABR_INPUT_DIR = "./";
	std::string ABR_TYPEFACE_DIR = "./";
	std::string ABR_TYPEFACE_NAME = "Consolas";
	std::string ABR_OUTPUT_DIR = "./";
	std::string ABR_OUTPUT_EXT = "PNG";
	bool ABR_RUN_DEBUG = false;

	bool batch = false;
	bool raw = false;
	bool flagsUsed = false;
}


int main(int argc, char** argv) {
	std::string source = "";

	try {
		//Store the command line arguments, ensuring that they were properly parsed
		int res = util::AbrPrint_HandleCLO(
			argc, argv, &source, &util::batch, &util::raw, &util::flagsUsed
			);

		if (res == 1) return 0;
		else if (res != 0) throw "AbrPrint: Error handling command-line arguments";
	}
	catch (std::string err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (const char* err) {
		std::cout << err << std::endl;
		return 1;
	}
	catch (...) {
		std::cout << "Unknown error occurred" << std::endl;
		return 1;
	}


	//Initialize SDL first. If SDL is going to fail on this run, you don't want it to happen
	// after the cycles have already been spent processing data
	try {
		//Initialize the AbrPrint configuration values
		util::AbrPrint_Init();

		//Initialize the graphics and text library
		util::debug(1, "Initializing graphics libraries");
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


	//Gather the filename list from the input directory information
	std::string directory;
	std::vector<std::string> filenameList;
	util::debug(1, "Beginning to gather list of filenames");
	try {
		if (source == "" && !util::flagsUsed) util::batch = true;

		try {
			filenameList = filectrl::gatherFilenames(source, &directory);
		}
		catch (std::string err) {
			if (err == "ABR_PLEASE_EXIT") return 0;
			else {
				std::cout << "main(): " << err << std::endl;
				return 1;
			}
		}
		catch (const char* err) {
			std::string currerr(err);
			if (currerr == "ABR_PLEASE_EXIT") return 0;
			else {
				std::cout << "main(): " << err << std::endl;
				return 1;
			}
		}
		catch (...) {
			std::cout << "Unknown error occurred" << std::endl;
			return 1;
		}

		util::debug(1, "File names gathered:");
		for (std::string name : filenameList)
			util::debug(1, "- " + name);
		util::debug(1, "");
	}
	catch (...) {
		std::cout << "Unknown error occurred" << std::endl;
		return 1;
	}


	//Generate some of the things necessary for creating graphs
	SDL_Surface* surf; SDL_Renderer* renderer; SDL_Texture* visualizer; TTF_Font* font;
	util::debug(1, "Creating necessary SDL2 graphics elements");
	try {
		util::debug(1, "  Generating the renderer");
		renderer = util::generateRenderer(&surf);

		util::debug(1, "  Generating the visualizer");
		visualizer = util::generateTexture(renderer);

		util::debug(1, "  Generating the typeface");
		font = util::getFont("Consolas", 24);
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
	util::debug(1, "  All elements successfully created\n");

	//Iterate through the list of filenames from the list
	util::debug(1, "Beginning graph generation");
	for (std::string filename : filenameList) {
		util::debug(1, "Processing file " + filename);

		//Gather the file's labels and populate a table for the data
		vector<string> labels;
		vector<vector<string>> table;
		util::debug(1, "Parsing data from file");
		try {
			//Use the filename to open the source file
			util::debug(1, "Opening input stream");
			ifstream src = filectrl::loadFile(directory, filename);

			//Read in the entire header from the file being processed
			util::debug(1, "Gathering data labels from the file");
			labels = proc::makeLabels(filename, &src);

			//Create a 2D vector table. The outer vector will be the columns, each identified
			// with the index of their title in the labels vector. The inner vector will be
			// the data itself
			util::debug(1, "Gathering data from the file");
			table = proc::makeTable(filename, labels, &src);
			
			util::debug(1, "Closing file input stream");
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
		util::debug(1, "Data successfully parsed");


		util::debug(1, "Generating graph from parsed data");
		try {
			//Fill the background and print the header of the graph
			util::debug(1, "Filling background and print graph header");
			util::fill(renderer, visualizer, util::ABR_BKGD_COLOR);
			util::printText(
				renderer, visualizer, filename, 75, 10, 24, 0,
				util::ABR_GRAPH_COLOR1, font, nullptr
			);

			//Discover the filename index
			util::debug(1, "Scanning for filename index");
			size_t fileindex = 0;
			for (size_t x = 0; x < labels.size(); x++)
				if (labels[x] == "FILE") fileindex = x;

			//Store the position of the graph on the screen
			util::debug(1, "Establishing graph frame position");
			SDL_Rect framepos = { 75, 110, util::IMG_W - 125, util::IMG_H - 300 };

			//Initialize some graph information
			util::debug(1, "Initializing graph metadata");
			proc::graphData_t graphInfo;
			graphInfo.framepos = framepos;
			//graphInfo.fileList = { table[fileindex], 0 };
			graphInfo.fileList = std::vector<std::pair<std::string, int>>();
			for (std::string file : table[fileindex])
				graphInfo.fileList.push_back(std::pair<std::string, int>(file, 0));
			graphInfo.vertDivisions = 10;
			//Calculate the range of markers on the graph data
			proc::getDataRange(table, &graphInfo);

			//Print the graph frame that will show behind the data
			util::debug(1, "Rendering graph frame to visualizer");
			proc::printGraphFrame(renderer, visualizer, &graphInfo, font);

			util::debug(1, "Generating graph bars from parsed data");
			std::vector<proc::graphBar_t> barsList =
				proc::generateBars(graphInfo, labels, table);
			proc::focusShortBars(&barsList);

			//Print the color keys at the top of the graph frame
			util::debug(1, "Rendering graph key to visualizer");
			proc::printKeys(renderer, visualizer, labels, graphInfo, font);

			//Draw each of the bars on under the graph
			util::debug(1, "Rendering graph bars to visualizer");
			proc::printBars(renderer, visualizer, barsList, font, false);

			//Render the graph onto the window
			util::debug(1, "Rendering texture to visual surface");
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
		util::debug(1, "Graph generation complete");


		//Save the graph to a file
		util::debug(1, "Saving finished graph to file");
		try {
			filectrl::saveGraphToFile(
				renderer, filename, util::ABR_OUTPUT_EXT,
				util::ABR_OUTPUT_DIR, "bargraph", visualizer
			);
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
		util::debug(1, "Graph saved to file\n");

	}


	//Clean up the dynamically allocated objects
	util::debug(1, "Cleaning current texture");
	SDL_DestroyTexture(visualizer);
	//SDL_DestroyWindow(window);
	util::debug(1, "Releasing the font");
	TTF_CloseFont(font);

	//Safely exit the graphics and text libraries.
	util::debug(1, "Freeing renderer's generated surface");
	SDL_FreeSurface(surf);

	util::debug(1, "Closing down graphics libraries");
	TTF_Quit();
	SDL_Quit();

	util::debug(1, "Making clean exit");
	return 0;
}