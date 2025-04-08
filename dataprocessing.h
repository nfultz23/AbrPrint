#ifndef PROC_H
#define PROC_H


#include <fstream>
#include <string>
#include <vector>

#include "./utils.h"


#define HEADER_BUFFER_SIZE 4096


namespace proc {
	using std::vector; using std::string; using std::ifstream;

	static const util::color_t ABR_BKGD_COLOR = { 220, 235, 240, 255 };
	static const util::color_t ABR_GRAPH_COLOR1 = { 40, 50, 80, 255 };
	static const util::color_t ABR_GRAPH_COLOR2 = { 185, 200, 225, 255 };

	static const util::color_t ABR_BAR_COLORS[] = {
		{160,  60,  60, 255},
		{190, 120,  40, 255},
		{160, 170,  40, 255},
		{ 60, 150,  40, 255},
		{ 60, 200, 195, 255},
		{ 50,  60, 145, 255},
		{120,  60, 170, 255},
		{150,  60, 150, 255}
	};

	static const int ABR_GRAPH_PADDING = 150;
	static const int ABR_GRAPH_THICKNESS = 5;

	struct graphData_t {
		SDL_Rect framepos;
		std::vector<std::string> fileList;
		int vertDivisions;
		double rangeMin, rangeMax;
	};

	struct graphBar_t {
		string label;
		double value;
		SDL_Rect barRect;
		util::color_t color;
	};

	/*Uses a file stream to access the file's headers, creating a list of
	*  labels for use in graphing
	*
	* Precondition: src is not nullptr, file referenced by src is not empty
	* Postcondition: The iterator of src is advanced to the file's data
	*
	* Param src is a pointer to an opened ifstream
	* Returns a string vector containing the data's column labels
	*/
	vector<string> makeLabels(string filename, ifstream* src);


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
	vector<vector<string>> makeTable(string filename, vector<string> labels, ifstream* src);


	/*Creates the bounds of a graph based on the size of a window
	*
	* Precondition: SDL2 must already be initialized
	*
	* Param renderer is the renderer that is printing the graph frame on the screen
	* Param texture is the surface that the graph frame is being rendered onto
	* Param img_w is the total width of the image
	* Param img_h is the total height of the image
	*/
	void printGraphFrame(
		SDL_Renderer* renderer, SDL_Texture* texture, graphData_t graphInfo, TTF_Font* font
		);


	/*Gathers the appropriate data range from the graph based on the data given
	*
	* Precondition: graphdata != nullptr
	* Postcondition: graphdata.rangeMax and graphdata.rangeMin will be populated with appropriate values
	*
	* Param table is the data table generated from the input file
	* Param graphdata is the a data structure whose range values will be populated
	* Returns an ordered pair with the min and max values of the range
	*/
	void getDataRange(vector<vector<string>> table, graphData_t* graphdata);


	/*Generates a list of renderable bars to place on the graph from data and labels
	*
	* Precondition: labels.size() == table.size() AND all entries in table have the same length AND
	*		graphdata range values are already populated
	* Postcondition: graphdata = #graphdata AND labels = #labels AND table = #table
	*
	* Param graphdata is a struct containing graph positioning and range data
	* Param labels is the list of database labels matching entries in the 'table' structure
	* Param table is the 2D list of string data from the parsed file
	* Returns a vector of graph bars reflecting the confidence of hits from the database
	*/
	vector<graphBar_t> generateBars(
		graphData_t graphdata, vector<string> labels, vector<vector<string>> table
		);


	/*Sorts a graphBar_t list by bar height in descending order
	*
	* Precondition: Each element in barsList has fully populated fields
	* Postcondition: The elements in barsList are sorted by height in
	*		descending order
	*
	* Param barsList is a list of graphBar_t objects with populated data
	* Returns nothing
	*/
	void focusShortBars(vector<graphBar_t>* barsList);


	/*Prints the key to the graph, explaining which database corresponds to which color
	*
	* Precondition: labels.size() > 2 AND renderer != nullptr AND texture != nullptr AND
	*		all graphinfo fields are populated AND font != nullptr
	* Postcondition: texture will have the graph key rendered onto it above the graph frame
	*
	* Param renderer is the SDL_Renderer that will be printing to the provided SDL_Texture
	* Param texture is the SDL_Texture that will be receiving the graph key
	* Param labels is the list of labels parsed from the input data table, contains the database
	*	names that correspond to the printed data
	* Param graphinfo is the struct containing position/sizing information for the graph frame
	* Param font is the TTL_Font that the labels will be printed in
	*/
	void printKeys(
		SDL_Renderer* renderer, SDL_Texture* texture, vector<string> labels,
		graphData_t graphinfo, TTF_Font* font
	);


	/*
	*/
	void printBars(
		SDL_Renderer* renderer, SDL_Texture* texture, vector<graphBar_t> barsList,
		TTF_Font* font, bool printVals
	);

}

#endif