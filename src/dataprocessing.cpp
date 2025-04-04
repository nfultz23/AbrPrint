#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#include "../dataprocessing.h"

namespace proc {
	using std::ifstream; using std::string; using std::vector; using std::ios;

	/*Uses a file stream to access the file's headers, creating a list of labels for use in graphing
	*
	* Precondition: src is not nullptr, file referenced by src is not empty
	* Postcondition: The iterator of src is advanced to the file's data
	*
	* Param src is a pointer to an opened ifstream
	* Returns a string vector containing the data's column labels
	*/
	std::vector<string> makeLabels(string filename, ifstream* src) {
		//Read in the entire header from the file being processed
		char header[4096];
		if (!(*src).getline(header, sizeof(header)))
			throw "proc::makeLabels(): " + filename + " appears to be empty";

		//This situation has no header to label the data in the file
		if (header[0] != '#')
			throw "proc::makeLabels(): " + filename + " does not contain a header to process";

		//Parse through the header line and create a vector to store the header labels
		std::vector<string> labels; string currHeader = "";
		for (size_t x = 1; x < strlen(header); x++) {
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
	vector<vector<string>> makeTable(string filename, vector<string> labels, ifstream* src) {

		vector<vector<string>> table;
		for (size_t x = 0; x < labels.size(); x++) table.push_back(vector<string>());

		//Iterate through the file to populate the table
		string currEntry;
		int i = 0;
		while ((*src) >> currEntry) {
			table[i].push_back(currEntry);
			i++;
			i %= labels.size();
		}

		//Clean up the absolute file paths from the source file
		int fileIndex = 0;
		for (size_t x = 0; x < labels.size(); x++) {
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
		) {
		//Wipe the screen for drawing the graph frame
		util::fill(renderer, texture, ABR_BKGD_COLOR);

		//Store the points for the outer edge of the graph
		SDL_Point points[3] = {
			{graphInfo.framepos.x - ABR_GRAPH_THICKNESS,
			 graphInfo.framepos.y},
			
			{graphInfo.framepos.x - ABR_GRAPH_THICKNESS,
			 graphInfo.framepos.y + graphInfo.framepos.h + ABR_GRAPH_THICKNESS},

			{graphInfo.framepos.x + graphInfo.framepos.w,
			 graphInfo.framepos.y + graphInfo.framepos.h + ABR_GRAPH_THICKNESS}
		};
		util::polygon_t boundary = { points, 3 };
		
		for (int x = 0; x < ABR_GRAPH_THICKNESS; x++) {
			//Draw the current boundary
			util::drawPolygon(renderer, texture, boundary, ABR_GRAPH_COLOR1);

			//Shift the top-left boundary point
			boundary.pointArr[0].x += 1;

			//Shift the corner boundary point
			boundary.pointArr[1].x += 1;
			boundary.pointArr[1].y -= 1;
			
			//Shift the bottom-right boundary point
			boundary.pointArr[2].y -= 1;
		}

		//Draw boundaries between the file columns
		int colWidth = graphInfo.framepos.w / graphInfo.fileList.size();
		for (size_t x = 0; x < graphInfo.fileList.size(); x++) {
			util::printText(
				renderer, texture,
				graphInfo.fileList[x],
				graphInfo.framepos.x + x * colWidth + 20,
				graphInfo.framepos.y + graphInfo.framepos.h + 5 + ABR_GRAPH_THICKNESS,
				14, 50,
				ABR_GRAPH_COLOR1,
				font
			);

			///* //This section is for the debugging purposes, don't leave this in
			SDL_Point top = {
				graphInfo.framepos.x + (x + 1) * colWidth,
				graphInfo.framepos.y
			};
			SDL_Point bottom = {
				graphInfo.framepos.x + (x + 1) * colWidth,
				graphInfo.framepos.y + graphInfo.framepos.h
			};
			util::drawLine(renderer, texture, top, bottom, ABR_GRAPH_COLOR2);
			//*/
		}
		util::drawLine(
			renderer, texture,
			{ graphInfo.framepos.x, graphInfo.framepos.y },
			{ graphInfo.framepos.x + graphInfo.framepos.w, graphInfo.framepos.y },
			ABR_GRAPH_COLOR2
			);

		//Draw the graph height markers and labels
		int rowHeight = graphInfo.framepos.h / graphInfo.vertDivisions;
		for (int x = 0; x <= graphInfo.vertDivisions; x++) {
			SDL_Point left = {
				graphInfo.framepos.x,
				graphInfo.framepos.y + x * rowHeight
			};
			SDL_Point right = {
				graphInfo.framepos.x + graphInfo.framepos.w,
				graphInfo.framepos.y + x * rowHeight
			};

			util::drawLine(renderer, texture, left, right, ABR_GRAPH_COLOR2);

			double index = (graphInfo.rangeMax - graphInfo.rangeMin) / graphInfo.vertDivisions;
			index *= graphInfo.vertDivisions - x;
			index += graphInfo.rangeMin;

			string hLabel = std::to_string(index);
			hLabel = hLabel.substr(0, hLabel.length() - 4);

			util::printText(
				renderer, texture,
				hLabel,
				graphInfo.framepos.x - 20 - 5 * hLabel.length() - ABR_GRAPH_THICKNESS,
				graphInfo.framepos.y + x * rowHeight - 5,
				14, 0,
				ABR_GRAPH_COLOR1,
				font
				);
		}

		return;
	}
}