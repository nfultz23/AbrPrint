#ifndef PROC_H
#define PROC_H


#include <fstream>
#include <string>
#include <vector>

#include "./utils.h"

#define HEADER_BUFFER_SIZE 4096


namespace proc {
	using std::vector; using std::string; using std::ifstream;

	static util::color_t ABR_BKGD_COLOR = { 220, 235, 240, 255 };
	static util::color_t ABR_GRAPH_COLOR1 = { 40, 50, 80, 255 };
	static util::color_t ABR_GRAPH_COLOR2 = { 185, 200, 225, 255 };
	static int ABR_GRAPH_PADDING = 150;
	static int ABR_GRAPH_THICKNESS = 5;

	struct graphData_t {
		SDL_Rect framepos;
		vector<string> fileList;
		int vertDivisions;
		int rangeMin, rangeMax;
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

}


#endif