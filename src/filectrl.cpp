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
		util::debug(1, "loadFile():");

		//Gat the file path from the input data
		util::debug(1, "  Attempting to open file for reading");
		std::string fullPath = directory + filename;
		//Attempt to open an input stream from the file
		std::ifstream src;
		src.open(fullPath);

		//Ensure that the input stream was properly opened
		if (!src.is_open())
			throw "filectr::loadFile(): Error opening file " + fullPath;

		util::debug(1, "  File stream successfully opened, returning...");
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
		util::debug(1, "gatherFilenames():");

		//Create a vector to store the list of filenames
		std::vector<std::string> filenames;

		//If the input was specified as a raw batch generation, parse through the
		// filenames here
		if (util::raw && util::batch) {
			util::debug(1, "  Raw path batch job provided");

			//The absolute path was passed in, so set the path directly to it
			std::string path = loc;
			util::debug(1, "  Parsing through entries in " + path);

			//Iterate through each file in the path,
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				//Convert the path to a string
				std::string currname = entry.path().string();
				util::debug(1, "    Detected file " + currname + ", extracting filename");

				//Move backward until the point where the name ends
				int x; for (x = currname.size() - 1; x >= 0; x--)
					if (currname[x] == '/') break;

				//Push the parsed filename onto the list
				util::debug(1, "    Filename found to be " + currname.substr(x + 1, currname.size()));
				filenames.push_back(currname.substr(x + 1, currname.size()));
			}

			util::debug(1, "  Storing the directory path in the passed-in location");
			*directory = path;

		}
		//If the input was a raw path, extract just the filename from it
		else if (util::raw) {
			util::debug(1, "  Single file raw path provided, extracting file name from path");

			//Move backward until the point where the name ends
			int x; for (x = loc.size() - 1; x >= 0; x--)
				if (loc[x] == '/') break;
			//Push the parsed filename onto the list
			filenames.push_back(loc.substr(x + 1, loc.size()));
			util::debug(1, "  Filename found to be " + loc.substr(x + 1, loc.size()));

			util::debug(1, "  Storing the directory path in the passed-in location");
			*directory = loc.substr(0, x+1);

		}
		//If the input was a batch path, extract the file names using the default path
		else if (util::batch) {
			util::debug(1, "  Batch job provided");

			//Find the path with any potential extra directories
			std::string path = util::ABR_INPUT_DIR + loc;
			util::debug(1, "  Parsing through entries in " + path);

			//Iterate through each file in the path
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				//Convert the current path name into a string
				std::string currname = entry.path().string();
				util::debug(1, "    Detected file " + currname + ", extracting filename");

				//Move backward until the point where the name ends
				int x; for (x = currname.size() - 1; x >= 0; x--)
					if (currname[x] == '/') break;

				//Push the parsed filename onto the list
				util::debug(1, "    Filename found to be " + currname.substr(x + 1, currname.size()));
				filenames.push_back(currname.substr(x + 1, currname.size()));
			}

			util::debug(1, "  Storing the directory path in the passed-in location");
			*directory = path;
		}
		else if (!util::flagsUsed) {
			util::debug(1, "  Single file location provided");

			std::string path = util::ABR_INPUT_DIR + loc;
			util::debug(1, "  Extracting file from " + path);

			//Move backward until the point where the name ends
			int x; for (x = path.size() - 1; x >= 0; x--)
				if (path[x] == '/') break;
			//Push the parsed filename onto the list
			filenames.push_back(path.substr(x + 1, path.size()));
			util::debug(1, "  Filename found to be " + path.substr(x + 1, loc.size()));

			util::debug(1, "  Storing the directory path in the passed-in location");
			*directory = path.substr(0, x + 1);
		}
		else {
			util::debug(1, "  Only configuration flags provided, returning...");
			throw "ABR_PLEASE_EXIT";
		}

		//Return the parsed list of filenames
		util::debug(1, "  File names extracted, returning...");
		return filenames;
	}


	/*Saves a generated graph to a file
	* 
	* Param renderer is the SDL_Renderer that will render the graph onto it
	* Param sourceName is the name of the file that produced the graph
	* Param fileType is the extension of the file
	* Param directory is the directory that the file is being written to
	* Param graphType is the kind of graph being made (currently only supports
	*   bargraph)
	* Param graph is the actual graph being written to the file
	*/
	void saveGraphToFile(
		SDL_Renderer* renderer, std::string sourceName, std::string fileType,
		std::string directory, std::string graphType, SDL_Texture* graph
		) {
		util::debug(1, "saveGraphToFile():");

		//Exclude the original extension from the file name
		util::debug(1, "  Trimming the original extension from the source name");
		int x; for (x = 0; x < sourceName.length(); x++)
			if (sourceName[x] == '.') break;
		std::string filename = sourceName.substr(0, x);
		util::debug(1, "    Resulting name: " + filename);

		//Add the graph type to the end of the filename
		util::debug(1, "  Appending graph type and file extension to filename");
		filename += "_" + graphType + ".";
		//Add the file extension to the end of the filename
		for (char c : util::ABR_OUTPUT_EXT) filename += std::tolower(c);
		util::debug(1, "    Resulting name: " + filename);
		
		//Create the full path to the file using the directory and name
		util::debug(1, "  Appending filename to the output directory");
		std::string fullpath = util::ABR_OUTPUT_DIR + filename;
		util::debug(1, "    Resulting path: " + fullpath);
		
		//Check whether the directory being saved to exists
		util::debug(1, "  Checking whether output path exists");
		std::filesystem::path outdir = util::ABR_OUTPUT_DIR;
		bool exists = std::filesystem::is_directory(outdir);

		//If the output directory does not already exist, attempt to instantiate it
		if (!exists) {
			util::debug(1, "    Output path does not exist, instantiating output path");
			if (!std::filesystem::create_directory(outdir))
				throw "filectrl::saveGraphToFile(): Failed to create output directory";
		}

		//Store encoding data about the passed-in graph texture
		util::debug(1, "  Gathering graph metadata");
		Uint32 format; int width, height;
		SDL_QueryTexture(graph, &format, NULL, &width, &height);

		//Create an SDL Surface to store the data
		util::debug(1, "  Creating render surface from graph metadata");
		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, format);

		//Clone the texture onto the surface
		util::debug(1, "  Copying graph to render surface");
		if (SDL_SetRenderTarget(renderer, graph) < 0)
			throw "saveGraphToFile(): Failed to direct renderer to graph output texture";
		if (SDL_RenderReadPixels(renderer, NULL, format, surface->pixels, surface->pitch) < 0)
			throw "saveGraphToFile(): Failed to copy graph data to render surface";

		//Write the surface to the proper file type
		util::debug(1, "  Beginning to save graph to file");
		if (util::ABR_OUTPUT_EXT == "PNG") {
			util::debug(1, "    Image extension is PNG, saving as PNG");
			if (IMG_SavePNG(surface, fullpath.c_str()) < 0)
				throw "saveGraphToFile(): Failed to save image as " + fullpath;
		}
		else if (util::ABR_OUTPUT_EXT == "JPEG") {
			util::debug(1, "    Image extension is JPEG, saving as JPEG");
			if (IMG_SaveJPG(surface, fullpath.c_str(), 50) < 0)
				throw "saveGraphToFile(): Failed to save image as " + fullpath;
		}
		else
			throw "saveGraphToFile(): Unrecognized file extension detected, failed to save";

		util::debug(1, "  Grpah successfully saved to file, returning...");
		return;
	}

}