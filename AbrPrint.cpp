#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>


using std::string;


SDL_Window* generateWindow();
std::ifstream loadFile();
std::vector<string> makeLabels(std::ifstream*);
std::vector<std::vector<string>> makeTable(std::vector<string>, std::ifstream*);


int main(int argc, char** argv) {
	//Initialize SDL first. If SDL is going to fail on this run, you don't want it to happen
	// after the cycles have already been spent processing data
	try { if (SDL_Init(SDL_INIT_VIDEO) < 0) throw SDL_GetError(); }
	catch (const char* err) {
		std::cout << err << std::endl;
		return 1;
	}

	//Gather the file's labels and populate a table for the data
	std::vector<string> labels;
	std::vector<std::vector<string>> table;
	try {
		//Take in the filename (within the abr_files directory) to process
		std::ifstream src = loadFile();

		//Read in the entire header from the file being processed
		labels = makeLabels(&src);

		//Create a 2D vector table. The outer vector will be the columns, each identified
		// with the index of their title in the labels vector. The inner vector will be
		// the data itself
		table = makeTable(labels, &src);

		src.close();
	}
	catch (const char* err) {
		std::cout << err << std::endl;
		return 1;
	}

	/*
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
	if (!window) throw (string)SDL_GetError();

	return window;
}


/*Prompts the user for input to open a file for analyzation
* 
* Precondition: Input name must match the name of a file in the abr_files directory
* 
* Returns an ifstream referencing the file input by the user during the function
*/
std::ifstream loadFile() {
	//Take in the filename (within the abr_files directory) to process
	std::cout << "Please enter the file to process:" << std::endl << "?> ";
	string filename; std::cin >> filename;

	//Open the file and ensure that it's ready for reading
	std::ifstream src;
	src.open("./abr_files/" + filename, std::ios::in);
	if (!src.is_open())
		throw "Error opening file " + filename + "\n";

	return src;
}


/*Uses a file stream to access the file's headers, creating a list of labels for use in graphing
* 
* Precondition: src is not nullptr, file referenced by src is not empty
* Postcondition: The iterator of src is advanced to the file's data
* 
* Param src is a pointer to an opened ifstream
* Returns a string vector containing the data's column labels
*/
std::vector<string> makeLabels(std::ifstream* src) {
	//Read in the entire header from the file being processed
	char header[4096];
	if (!(*src).getline(header, sizeof(header))) throw "File appears to be empty";

	//This situation has no header to label the data in the file
	if (header[0] != '#') throw "Imma be real, I have no idea how to handle this situation";

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

	return labels;
}


/*Uses a list of labels and a file stream to populate a 2d table of data
* 
* Precondition: labels.size() equals the columns in the data file, src is not null
* Postcondition: src has been entirely processed, labels = #labels
* 
* Param labels is the list of labels for each of the columns on the table
* Param src is a pointer to an opened ifstream
* Returns a 2d vector, with the first index referencing the column (matching the index of
*   its respective label) and the second matching the row in the source file
*/
std::vector<std::vector<string>> makeTable(std::vector<string> labels, std::ifstream* src) {

	std::vector<std::vector<string>> table;
	for (int x = 0; x < labels.size(); x++) table.push_back(std::vector<string>());

	//Iterate through the file to populate the table
	std::string currEntry;
	int i = 0;
	while ((*src) >> currEntry) {
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
		while (currString[i - 1] != '/' && i > 0) i--;

		//Store the filename substring
		table[fileIndex][x] = currString.substr(i, currString.length());
	}

	return table;
}