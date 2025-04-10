#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <utility>
#include <SDL_Image.h>
#include <cctype>

#include "../filectrl.h"
#include "../utils.h"


namespace filectrl {
	using std::ifstream; using std::string; using std::ios;

	/*Takes in a directory and filename, and opens an input stream to read data
	*  from the file.
	* 
	* Param directory is the directory that the file is found in. Can be absolute
	*  or relative to the executable
	* Param filename is the name of the file within the directory to open
	* 
	* Return a filestream object that allows the system to read data from the file
	*/
	ifstream filectrl::loadFile(string directory, string filename) {

		std::string fullPath = directory + filename;

		std::ifstream src;
		src.open(fullPath);

		if (!src.is_open())
			throw "filectr::loadFile(): Error opening file " + fullPath;

		return src;
	}


	/*Takes in a location (file path, absolute/relative directory) and finds a
	*  list of file names associated with it. If given a file path, it gives only
	*  the name of the file and stores its location in the value of 'directory'. If
	*  given a directory, it stores that value in 'directory' and generates a list
	*  of files present in the passed-in directory
	* 
	* Param loc is the location being searched. Can be a file path or an absolute/relative
	*  path to a directory
	* Param directory is a pointer to a string that will be populated with the path
	*  to the files returned from the function
	* 
	* Returns a vector of strings containing the filenames present in the
	*  provided location
	*/
	std::vector<std::string> gatherFilenames(std::string loc, std::string* directory) {
		//Create a vector to store the list of filenames
		std::vector<std::string> filenames;

		//If the input was specified as a raw batch generation, parse through the
		// filenames here
		if (util::raw && util::batch) {
			//The absolute path was passed in, so set the path directly to it
			std::string path = loc;

			//Iterate through each file in the path,
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				//Convert the path to a string
				std::string currname = entry.path().string();

				//Move backward until the point where the name ends
				int x; for (x = currname.size() - 1; x >= 0; x--)
					if (currname[x] == '/') break;

				//Push the parsed filename onto the list
				filenames.push_back(currname.substr(x + 1, currname.size()));
			}

			*directory = path;

		}
		//If the input was a raw path, extract just the filename from it
		else if (util::raw) {
			//Move backward until the point where the name ends
			int x; for (x = loc.size() - 1; x >= 0; x--)
				if (loc[x] == '/') break;
			//Push the parsed filename onto the list
			filenames.push_back(loc.substr(x + 1, loc.size()));

			*directory = loc.substr(0, x+1);

		}
		//If the input was a batch path, extract the file names using the default path
		else if (util::batch) {
			//Find the path with any potential extra directories
			std::string path = util::ABR_INPUT_DIR + loc;

			//Iterate through each file in the path
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				//Convert the current path name into a string
				std::string currname = entry.path().string();


				//Move backward until the point where the name ends
				int x; for (x = currname.size() - 1; x >= 0; x--)
					if (currname[x] == '/') break;
				//Push the parsed filename onto the list
				filenames.push_back(currname.substr(x + 1, currname.size()));
			}

			*directory = path;
		}
		else if (!util::flagsUsed) {
			std::string path = util::ABR_INPUT_DIR + loc;

			//Move backward until the point where the name ends
			int x; for (x = path.size() - 1; x >= 0; x--)
				if (path[x] == '/') break;
			//Push the parsed filename onto the list
			filenames.push_back(path.substr(x + 1, path.size()));

			*directory = path.substr(0, x + 1);
		}
		else throw "ABR_PLEASE_EXIT";

		//Return the parsed list of filenames
		return filenames;
	}


	/*
	*/
	void saveGraphToFile(
		SDL_Renderer* renderer,
		std::string sourceName, std::string fileType, std::string directory,
		std::string graphType, SDL_Texture* graph
	) {
		//Exclude the original extension from the file name
		int x; for (x = 0; x < sourceName.length(); x++)
			if (sourceName[x] == '.') break;
		std::string filename = sourceName.substr(0, x);

		//Add the graph type to the end of the filename
		filename += "_" + graphType + ".";
		//Add the file extension to the end of the filename
		for (char c : fileType) filename += std::tolower(c);
		
		//Create the full path to the file using the directory and name
		std::string fullpath = directory + filename;

		
		//Check whether the directory being saved to exists
		std::filesystem::path outdir = util::ABR_OUTPUT_DIR;
		bool exists = std::filesystem::is_directory(outdir);

		//If the output directory does not already exist, attempt to instantiate it
		if (!exists) {
			if (!std::filesystem::create_directory(outdir))
				throw "filectrl::saveGraphToFile(): Failed to create output directory";
		}

		//Store encoding data about the passed-in graph texture
		Uint32 format; int width, height;
		SDL_QueryTexture(graph, &format, NULL, &width, &height);

		//Create an SDL Surface to store the data
		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, format);

		//Clone the texture onto the surface
		if (SDL_SetRenderTarget(renderer, graph) < 0)
			throw "saveGraphToFile(): Failed to direct renderer to graph output texture";
		if (SDL_RenderReadPixels(renderer, NULL, format, surface->pixels, surface->pitch) < 0)
			throw "saveGraphToFile(): Failed to copy graph data to render surface";

		//Write the surface to the proper file type
		if (util::ABR_OUTPUT_EXT == "PNG") {
			;
		}
		else if (util::ABR_OUTPUT_EXT == "JPEG") {
			;
		}
		else
			throw "saveGraphToFile(): Unrecognized file extension detected, failed to save";

		return;
	}

}