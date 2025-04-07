#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

#include "../utils.h"
#include "../dataprocessing.h"

namespace proc {
	/*Uses a file stream to access the file's headers, creating a list of labels for use in graphing
	*
	* Precondition: src is not nullptr, file referenced by src is not empty
	* Postcondition: The iterator of src is advanced to the file's data
	*
	* Param src is a pointer to an opened ifstream
	* Returns a string vector containing the data's column labels
	*/
	vector<string> makeLabels(string filename, ifstream* src) {
		//Read in the entire header from the file being processed
		char header[4096];
		if (!(*src).getline(header, sizeof(header)))
			throw "proc::makeLabels(): " + filename + " appears to be empty";

		//This situation has no header to label the data in the file
		if (header[0] != '#')
			throw "proc::makeLabels(): " + filename + " does not contain a header to process";

		//Parse through the header line and create a vector to store the header labels
		vector<string> labels; string currHeader = "";
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
			if (labels[x] == "FILE") { fileIndex = x; break; }
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
		
		for (int x = 0; x <= ABR_GRAPH_THICKNESS; x++) {
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
				14, 40,
				ABR_GRAPH_COLOR1,
				font
				);

			///* //This section is for the debugging purposes, don't leave this in
			SDL_Point top = {
				graphInfo.framepos.x + (x + 1) * colWidth + ABR_GRAPH_THICKNESS,
				graphInfo.framepos.y
			};
			SDL_Point bottom = {
				graphInfo.framepos.x + (x + 1) * colWidth + ABR_GRAPH_THICKNESS,
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
			//Get the endpoints of a horizontal line across the screen
			SDL_Point left = {
				graphInfo.framepos.x,
				graphInfo.framepos.y + x * rowHeight
			};
			SDL_Point right = {
				graphInfo.framepos.x + graphInfo.framepos.w,
				graphInfo.framepos.y + x * rowHeight
			};
			//Draw the horizontal line
			util::drawLine(renderer, texture, left, right, ABR_GRAPH_COLOR2);

			//Store the numeric value of the horizontal line as a double
			double index = (graphInfo.rangeMax - graphInfo.rangeMin) / graphInfo.vertDivisions;
			index *= graphInfo.vertDivisions - x;
			index += graphInfo.rangeMin;
			//Convert the numeric value of the mark, clipping it to 2 decimal points
			string hLabel = std::to_string(index);
			hLabel = hLabel.substr(0, hLabel.length() - 4);

			//Print the text label for the horizontal mark
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


	/*Gathers the appropriate data range from the graph based on the data given
	*
	* Precondition: graphdata != nullptr AND graphData.vertDivisions > 2
	* Postcondition: graphdata.rangeMax and graphdata.rangeMin will be populated with appropriate values
	* 
	* Param table is the data table generated from the input file
	* Param graphdata is the a data structure whose range values will be populated
	* Returns an ordered pair with the min and max values of the range
	*/
	void getDataRange(vector<vector<string>> table, graphData_t* graphdata) {
		if (util::debug) std::cout << "getDataRange(): " << std::endl;
		//Initialize the minimum and maximum values to null values
		if (util::debug)
			std::cout << "  Initializing min and max to 0" << std::endl;
		double min = 0, max = 0;

		//Set a value to track the initialization of the min/max values
		bool first = true;
		for (int x = 2; x < table.size(); x++) {
			for (int y = 0; y < table[x].size(); y++) {
				
				//If no value was found in the Abricate processing, skip past the entry
				if (table[x][y] == ".") {
					if (util::debug)
						std::cout << "  null value found, moving on" << std::endl;
					continue;
				}

				//This section converts a string entry to a double value
				if (util::debug)
					std::cout << "  Converting the current value to a double" << std::endl;
				double currVal = 0;
				//If multiple values have been found, take the first and convert it to a double
				// This will probably change later when I decide on how to handle multiple hits
				if (util::contains(table[x][y].c_str(), ';', table[x][y].size())) {
					int endpt = 0;
					for (char c : table[x][y]) {
						if (c == ';') break;
						endpt++;
					}

					currVal = std::stod(table[x][y].substr(0, endpt));
				}
				//If a single value has been found, convert it to a double directly
				else currVal = std::stod(table[x][y]);
				if (util::debug)
					std::cout << "  currVal == " << currVal << std::endl;

				//If this is the first value found, initialize the min and max values
				if (first) {
					std::cout << "  First nonzero value found, updating min and max..." << std::endl;
					min = currVal; max = currVal;
					first = false;
					std::cout << "  min == " << min << std::endl;
					std::cout << "  max == " << max << std::endl;
				}

				//Check whether the maximum value is greater than the current maximum
				if (currVal > max) max = currVal;
				if (util::debug) {
					std::cout << "  Checking for increased maximum" << std::endl;
					std::cout << "  max == " << max << std::endl;
				}
				//Check whether the minimum value is less than the current minimum
				if (currVal < min) min = currVal;
				if (util::debug) {
					std::cout << "  Checking for decreased minimum" << std::endl;
					std::cout << "  min == " << min << std::endl;
				}
			}
		}

		/*
		//Calculate a 25% buffer on either side of the range
		double margin = (max - min) * 0.25;
		//Give the range values 25% padding on either side, limited by the maximum and minimum values
		graphdata->rangeMax = (max + margin < 100.0 ? max + margin : 100.0);
		graphdata->rangeMin = (min - margin > 000.0 ? min - margin : 100.0);
		*/

		double margin = 0.0;
		if (max == min) margin = 5.0;
		else margin = (max - min) * 0.25;

		graphdata->rangeMax = (max + margin < 100.0 ? max + margin : 100.0);
		graphdata->rangeMin = (min - margin > 000.0 ? min - margin : 100.0);
		
		if (util::debug) {
			std::cout << "  margin == " << margin << std::endl;
			std::cout << "  graphdata->rangeMax == " << graphdata->rangeMax << std::endl;
			std::cout << "  graphdata->rangeMin == " << graphdata->rangeMin << std::endl;
			std::cout << std::endl << std::endl;
		}

		return;
	}


	/*
	*/
	vector<graphBar_t> generateBars(
		graphData_t graphdata, vector<string> labels, vector<vector<string>> table
		) {
		if (util::debug) std::cout << "generateBars(): " << std::endl;

		//Process the table into raw data
		if (util::debug) std::cout << "Compiling the raw data" << std::endl;
		vector<vector<double>> rawdata;
		for (int x = 2; x < labels.size(); x++) {
			
			vector<double> currCol;
			for (string s : table[x]) {
				//If there were no hits found, add a zero to the value list
				if (s == ".")
					currCol.push_back(0.0);
				//If there is only a single hit in the table, convert it directly for the table
				else if (!util::contains(s.c_str(), ';', s.length()))
					currCol.push_back(std::stod(s));
				//if there are several hits in the table, take the first (fix this later)
				else {
					int endpt = 0;
					for (char c : s) {
						if (c == ';') break;
						endpt++;
					}
					currCol.push_back( std::stod(s.substr(0, endpt)) );
				}
			}
			rawdata.push_back(currCol);

			if (util::debug) {
				std::cout << labels[x] << ": ";
				util::printVec(table[x]);
				std::cout << labels[x] << ": ";
				util::printVec(rawdata[x - 2]);
				std::cout << std::endl;
			}
		}

		//Calculate the width of a bar on the screen so it only has to be done once
		const int entryWidth = (graphdata.framepos.w / table[0].size());
		if (util::debug)
			std::cout << "entryWidth == " << entryWidth << std::endl;
		int barwidth = entryWidth - (entryWidth * 0.1);
		if (rawdata[0].size() >= 2)
			barwidth = (barwidth * (rawdata[0].size() / 2 + 1)) / rawdata[0].size();
		else if (rawdata[0].size() == 2)
			barwidth = (barwidth * 2) / 3;
		if (util::debug)
			std::cout << "barwidth == " << barwidth << std::endl;

		//Create a vector to store the bars, they will be graphed label-by-label
		vector<graphBar_t> graphList;
		//Parse through the data to create bars for rendering
		if (util::debug)
			std::cout << "Generating bars for display" << std::endl;
		int xoffset = 0;
		for (int x = 0; x < rawdata.size(); x++) {
			if (util::debug) {
				std::cout << "Working with label \'" << labels[x + 2];
				std::cout << "\'" << std::endl;
			}
			for (int y = 0; y < rawdata[x].size(); y++) {
				//If the current entry is zero, move on
				//if (rawdata[x][y] == 0) continue;
				if (util::debug)
					std::cout << "  rawdata[x][y] == " << rawdata[x][y] << std::endl;

				//Update the horizontal position based on the file being displayed
				int xpos = ABR_GRAPH_THICKNESS + graphdata.framepos.x;
				xpos += (entryWidth * y) + (entryWidth * 0.05) + xoffset;
				if (y == 0) xpos -= ABR_GRAPH_THICKNESS;
				if (util::debug)
					std::cout << "  xpos == " << xpos << std::endl;

				//Calculate the height of the bar
				double range = graphdata.rangeMax - graphdata.rangeMin;
				if (util::debug)
					std::cout << "  range == " << range << std::endl;
				int height = 0; //=
				if (rawdata[x][y] != 0)
					height = graphdata.framepos.h * ((double)(rawdata[x][y] - graphdata.rangeMin) / range);
				if (util::debug)
					std::cout << "  height == " << height << std::endl;

				//Calculate the top position of the bar
				int ypos = graphdata.framepos.y + (graphdata.framepos.h - height);
				if (util::debug)
					std::cout << "  ypos == " << ypos << std::endl << std::endl;

				//Gather the bar's rect
				SDL_Rect barRect = { xpos, ypos, barwidth, height };
				if (util::debug) {
					std::cout << "  barRect == {" << std::endl;
					std::cout << "    " << barRect.x << "," << std::endl;
					std::cout << "    " << barRect.y << "," << std::endl;
					std::cout << "    " << barRect.w << "," << std::endl;
					std::cout << "    " << barRect.h << "," << std::endl;
					std::cout << "  };" << std::endl;
					std::cout << std::endl << std::endl;
				}

				graphList.push_back({ labels[x+2], barRect, ABR_BAR_COLORS[x] });
			}
			//Increase the offset so the bars are all equally visible
			xoffset += (entryWidth - barwidth) / rawdata.size() * x * (2.0 / 3.0);
			if (util::debug) std::cout << "  xoffset == " << xoffset << std::endl;
			if (util::debug) std::cout << std::endl;
		}

		//Return the compiled list of bars
		return graphList;
	}

	/*Determines if the height of ba rB is less than that of bar A*/
	bool isShorter(graphBar_t barA, graphBar_t barB) { return barA.barRect.h > barB.barRect.h; }


	void focusShortBars(vector<graphBar_t>* barsList) {
		std::sort(barsList->begin(), barsList->end(), isShorter);
		return;
	}

}