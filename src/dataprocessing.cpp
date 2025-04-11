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
		util::debug(1, "makeLabels():");

		//Read in the entire header from the file being processed
		util::debug(1, "  Ensuring that the file has a header to read");
		char header[4096];
		if (!(*src).getline(header, sizeof(header)))
			throw "proc::makeLabels(): " + filename + " appears to be empty";

		//This situation has no header to label the data in the file
		if (header[0] != '#')
			throw "proc::makeLabels(): " + filename + " does not contain a header to process";

		//Parse through the header line and create a vector to store the header labels
		util::debug(1, "  Parsing through the header to store labels");
		vector<string> labels; string currHeader = "";
		for (size_t x = 1; x < strlen(header); x++) {
			//If a delimiter is found, separate it and push it into the label vector
			if (header[x] == '\t' || x == strlen(header) - 1) {
				labels.push_back(currHeader);
				util::debug(1, "    Label found to be " + currHeader);
				currHeader = "";
				continue;
			}
			//Otherwise, add the character to the compiled header
			currHeader += header[x];
		}

		util::debug(1, "  Header parsed, returning...");
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
		util::debug(1, "makeTable():");

		//Create an empty vector for each label for population later
		util::debug(1, "  Creating a list of data tables to populate");
		vector<vector<string>> table;
		for (size_t x = 0; x < labels.size(); x++) table.push_back(vector<string>());

		//Iterate through the file to populate the table
		util::debug(1, "  Populating data table from input file");
		string currEntry;
		int i = 0;
		while ((*src) >> currEntry) {
			table[i].push_back(currEntry);
			i++;
			i %= labels.size();
		}

		//Clean up the absolute file paths from the source file
		util::debug(1, "  Discovering the file index");
		int fileIndex = 0;
		for (size_t x = 0; x < labels.size(); x++) {
			if (labels[x] == "FILE") { fileIndex = x; break; }
		}

		//Iterate through the file column and erase the absolute file paths
		util::debug(1, "  Triming paths from the filenames");
		for (int x = 0; x < table[fileIndex].size(); x++) {
			//Find the index of the string where the file path ends
			string currString = table[fileIndex][x];
			int i = currString.length() - 1;
			while (currString[i - 1] != '/' && i > 0) i--;

			//Store the filename substring
			table[fileIndex][x] = currString.substr(i, currString.length());
		}

		util::debug(1, "  Table populated, returning...");
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
		SDL_Renderer* renderer, SDL_Texture* texture, graphData_t* graphInfo, TTF_Font* font
		) {
		util::debug(1, "printGraphFrame():");

		//Draw boundaries between the file columns
		util::debug(1, "  Generating horizontal divisions");
		int colWidth = graphInfo->framepos.w / graphInfo->fileList.size();
		for (size_t x = 0; x < graphInfo->fileList.size(); x++) {
			util::debug(1, "  Handling label " + graphInfo->fileList[x].first);
			util::debug(1, "    Printing label text");
			util::printText(
				renderer, texture,
				graphInfo->fileList[x].first,
				graphInfo->framepos.x + x * colWidth + 20,
				graphInfo->framepos.y + graphInfo->framepos.h + 5 + util::ABR_GRAPH_THICKNESS,
				14, 40,
				util::ABR_GRAPH_COLOR1,
				font,
				nullptr
				);
			graphInfo->fileList[x].second = graphInfo->framepos.x + x * colWidth + 20;

			util::debug(1, "    Drawing vertical division");
			SDL_Point top = {
				graphInfo->framepos.x + (x + 1) * colWidth + util::ABR_GRAPH_THICKNESS,
				graphInfo->framepos.y
			};
			SDL_Point bottom = {
				graphInfo->framepos.x + (x + 1) * colWidth + util::ABR_GRAPH_THICKNESS,
				graphInfo->framepos.y + graphInfo->framepos.h
			};
			util::drawLine(renderer, texture, top, bottom, util::ABR_GRAPH_COLOR2);

		}
		util::debug(1, "  Drawing graph cap line");
		util::drawLine(
			renderer, texture,
			{ graphInfo->framepos.x, graphInfo->framepos.y },
			{ graphInfo->framepos.x + graphInfo->framepos.w, graphInfo->framepos.y },
			util::ABR_GRAPH_COLOR2
			);

		//Draw the graph height markers and labels
		util::debug(1, "  Drawing graph height markers");
		int rowHeight = graphInfo->framepos.h / graphInfo->vertDivisions;
		for (int x = 0; x <= graphInfo->vertDivisions; x++) {
			util::debug(1, "    Drawing horizontal line across the graph frame");
			//Get the endpoints of a horizontal line across the screen
			SDL_Point left = {
				graphInfo->framepos.x,
				graphInfo->framepos.y + x * rowHeight
			};
			SDL_Point right = {
				graphInfo->framepos.x + graphInfo->framepos.w,
				graphInfo->framepos.y + x * rowHeight
			};
			//Draw the horizontal line
			util::drawLine(renderer, texture, left, right, util::ABR_GRAPH_COLOR2);

			//Store the numeric value of the horizontal line as a double
			util::debug(1, "    Calculating numeric value of horizontal division");
			double index =
				(graphInfo->rangeMax - graphInfo->rangeMin) / graphInfo->vertDivisions;
			index *= graphInfo->vertDivisions - x;
			index += graphInfo->rangeMin;
			//Convert the numeric value of the mark, clipping it to 2 decimal points
			string hLabel = std::to_string(index);
			hLabel = hLabel.substr(0, hLabel.length() - 4);

			//Print the text label for the horizontal mark
			util::debug(1, "    Printing horizontal division");
			util::printText(
				renderer, texture,
				hLabel,
				graphInfo->framepos.x - 20 - 5 * hLabel.length() - util::ABR_GRAPH_THICKNESS,
				graphInfo->framepos.y + x * rowHeight - 5,
				14, 0,
				util::ABR_GRAPH_COLOR1,
				font,
				nullptr
				);
		}

		//Store the points for the outer edge of the graph
		util::debug(1, "  Printing initial left-bottom sides of the graph");
		SDL_Point points[3] = {
			{graphInfo->framepos.x,
			 graphInfo->framepos.y},

			{graphInfo->framepos.x,
			 graphInfo->framepos.y + graphInfo->framepos.h + util::ABR_GRAPH_THICKNESS},

			{graphInfo->framepos.x + graphInfo->framepos.w,
			 graphInfo->framepos.y + graphInfo->framepos.h + util::ABR_GRAPH_THICKNESS}
		};
		util::polygon_t boundary = { points, 3 };

		util::debug(1, "  Increasing main frame thickness");
		for (int x = 0; x < util::ABR_GRAPH_THICKNESS; x++) {
			//Draw the current boundary
			util::drawPolygon(renderer, texture, boundary, util::ABR_GRAPH_COLOR1);

			//Shift the top-left boundary point
			boundary.pointArr[0].x += 1;

			//Shift the corner boundary point
			boundary.pointArr[1].x += 1;
			boundary.pointArr[1].y -= 1;

			//Shift the bottom-right boundary point
			boundary.pointArr[2].y -= 1;
		}

		util::debug(1, "  Graph frame printed, returning...");
		return;
	}


	/*Gathers the appropriate data range from the graph based on the data given
	*
	* Precondition: graphdata != nullptr AND graphData.vertDivisions > 2
	* Postcondition: graphdata.rangeMax and graphdata.rangeMin will be populated with appropriate values
	* 
	* Param table is the data table generated from the input file
	* Param graphdata is the a data structure whose range values will be populated
	*/
	void getDataRange(vector<vector<string>> table, graphData_t* graphdata) {
		util::debug(1, "getDataRange():");

		//Initialize the minimum and maximum values to null values
		double min = 0, max = 0;

		//Set a value to track the initialization of the min/max values
		bool first = true;
		util::debug(1, "  Parsing data to gather data range");
		for (int x = 2; x < table.size(); x++) {
			for (int y = 0; y < table[x].size(); y++) {
				
				//If no value was found in the Abricate processing, skip past the entry
				if (table[x][y] == ".") {
					util::debug(1, "    Null value found, moving on to next entry in the table");
					continue;
				}

				//This section converts a string entry to a double value
				double currVal = 0;
				//If multiple values have been found, take the first and convert it to a double
				// This will probably change later when I decide on how to handle multiple hits
				if (util::contains(table[x][y].c_str(), ';', table[x][y].size())) {
					util::debug(1, "    Multivalue entry found, taking first numeric value");
					int endpt = 0;
					for (char c : table[x][y]) {
						if (c == ';') break;
						endpt++;
					}

					currVal = std::stod(table[x][y].substr(0, endpt));
				}
				//If a single value has been found, convert it to a double directly
				else {
					util::debug(1, "    Single value entry found");
					currVal = std::stod(table[x][y]);
				}

				//If this is the first value found, initialize the min and max values
				if (first) {
					util::debug(1, "    First numeiric entry found, initializing range values");
					min = currVal; max = currVal;
					first = false;
				}

				util::debug(1, "    Adjusting range values");
				//Check whether the maximum value is greater than the current maximum
				if (currVal > max) {
					util::debug(1, "      Increasing maximum range value");
					max = currVal;
				}
				//Check whether the minimum value is less than the current minimum
				if (currVal < min) {
					util::debug(1, "      Decreasing minimum range value");
					min = currVal;
				}

				util::debug(1, "    Entry properly parsed, moving on");
			}
		}
		util::debug(1, "  Finished parsing table");

		util::debug(1, "  Calculating range margin for display padding");
		double margin = 0.0;
		//If the min and max are the same, add 5% padding on either side
		if (max == min) {
			util::debug(1, "    Range found to be zero, storing absolute 5% margin");
			margin = 5.0;
		}
		//If there is a range of values, add 25% of the range as padding
		else {
			util::debug(1, "    Range found to be greater than zero, storing 25% of range as margin");
			margin = (max - min) * 0.25;
		}

		//Add the padding to the value, ensuring that the range does not
		// exceed 100% or 0% certainty values
		util::debug(1, "  Ensuring range expansion does not exit the bounds");
		graphdata->rangeMax = (max + margin < 100.0 ? max + margin : 100.0);
		graphdata->rangeMin = (min - margin > 000.0 ? min - margin : 100.0);

		return;
	}


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
		) {
		util::debug(1, "generateBars()");

		//Process the table into raw data
		vector<vector<double>> rawdata;
		util::debug(1, "  Processing passed-in table into raw data");
		for (int x = 2; x < labels.size(); x++) {
			
			vector<double> currCol;
			for (string s : table[x]) {

				//If there were no hits found, add a zero to the value list
				if (s == ".") {
					util::debug(1, "    Null value entryfound, interpreting as 0.0");
					currCol.push_back(0.0);
				}
				//If there is only a single hit in the table, convert it directly for the table
				else if (!util::contains(s.c_str(), ';', s.length())) {
					util::debug(1, "    Single value entry found");
					currCol.push_back(std::stod(s));
				}
				//if there are several hits in the table, take the first (fix this later)
				else {
					util::debug(1, "    Multivalue entry found, taking first numeric value");
					int endpt = 0;
					for (char c : s) {
						if (c == ';') break;
						endpt++;
					}
					currCol.push_back( std::stod(s.substr(0, endpt)) );
				}
			}
			rawdata.push_back(currCol);
		}
		util::debug(1, "  Table successfully parsed into raw data");

		if (rawdata.size() == 0) {
			util::debug(1, "  Parsed table found to be empty, returning...");
			return vector<graphBar_t>();
		}

		//Calculate the width of a bar on the screen so it only has to be done once
		util::debug(1, "  Calculating bar width");
		const int entryWidth = (graphdata.framepos.w / graphdata.fileList.size()) + 1;
		int barwidth = entryWidth - util::ABR_GRAPH_THICKNESS * 4;
		int padding = (entryWidth - barwidth) / 2;

		//barwidth /= rawdata.size();
		barwidth /= rawdata.size() + (rawdata.size() > 3 ? 1 : 0);
		int barpad = barwidth;
		if (rawdata.size() > 3) barwidth *= 2;


		//Create a vector to store the bars, they will be graphed label-by-label
		vector<graphBar_t> graphList;
		//Parse through the data to create bars for rendering
		util::debug(1, "  Generating displayable bars");
		int xoffset = 0;
		for (int x = 0; x < rawdata.size(); x++) {
			for (int y = 0; y < rawdata[x].size(); y++) {
				//If the current entry is zero, move on
				if (rawdata[x][y] == 0) {
					util::debug(1, "    Bar value found to be zero, moving on to next entry");
					continue;
				}

				//Update the horizontal position based on the file being displayed
				util::debug(1, "    Calculating horizontal bar position");
				int xpos = graphdata.fileList[y].second - 15 + padding;
				xpos += barpad * x;

				//Calculate the height of the bar
				util::debug(1, "    Calculating bar height");
				double range = graphdata.rangeMax - graphdata.rangeMin;
				int height = 0;
				if (rawdata[x][y] != 0)
					height = graphdata.framepos.h * (
						(double)(rawdata[x][y] - graphdata.rangeMin) / range
						);
				
				//Calculate the top position of the bar
				util::debug(1, "    Calculating vertical bar position");
				int ypos = graphdata.framepos.y + (graphdata.framepos.h - height) + 1;

				//Gather the bar's rect
				util::debug(1, "    Storing bar data");
				SDL_Rect barRect = { xpos, ypos, barwidth, height };

				util::debug(1, "    Storing bar metadata");
				graphBar_t newBar;
				newBar.label = labels[x + 2];
				newBar.value = rawdata[x][y];
				newBar.barRect = barRect;
				newBar.color = util::ABR_BAR_COLORS[x];

				graphList.push_back(newBar);
			}
			//Increase the offset so the bars are all equally visible
			xoffset += barwidth;
		}

		//Return the compiled list of bars
		util::debug(1, "  Bar list successfully generated, returning...");
		return graphList;
	}


	/*Determines if the height of bar B is less than that of bar A*/
	bool isShorter(graphBar_t barA, graphBar_t barB) {
		return barA.barRect.h > barB.barRect.h;
	}


	/*Sorts a graphBar_t list by bar height in descending order
	* 
	* Precondition: Each element in barsList has fully populated fields
	* Postcondition: The elements in barsList are sorted by height in
	*		descending order
	* 
	* Param barsList is a list of graphBar_t objects with populated data
	* Returns nothing
	*/
	void focusShortBars(vector<graphBar_t>* barsList) {
		util::debug(1, "focusShortBars()");
		util::debug(1, "  Sorting bars by height, tallest to shortest");
		std::sort(barsList->begin(), barsList->end(), isShorter);
		util::debug(1, "  Bars sorted, returning...");
	}


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
		) {
		util::debug(1, "printKeys():");

		//Store the starting positions of the graph and the sizing for both color tiles and text
		util::debug(1, "  Storing key starting position, as well as sizing information");
		int xpos = graphinfo.framepos.x, ypos = graphinfo.framepos.y - 70;
		int colw = 10, colh = 10;
		int fontsize = 14;

		//Iterate through each database field in the labels list
		util::debug(1, "  Iterating through labels");
		for (int x = 2; x < labels.size(); x++) {
			util::debug(1, "  Handling label " + labels[x]);

			//Create a color tile rect with a proper position, centering it with the text
			SDL_Rect colTileRect = { xpos, ypos + (fontsize - colh) / 2, colw, colh };
			//Draw the color tile and move the xposition to where the new text will be printed
			try {
				util::debug(1, "    Drawing bar color");
				util::fillRect(renderer, texture, colTileRect, util::ABR_BAR_COLORS[x - 2]);
			}
			//Handle potential errors and throw them up the chain
			catch (std::string err) {
				throw "proc::printKeys(): " + (std::string)err;
			}
			catch (const char* err) {
				throw "proc::printKeys(): " + (std::string)err;
			}
			catch (...) {
				throw "proc::printKeuys(): Unknown error occurred while drawing color tile";
			}
			xpos += colw + 5;

			//Print the label of the database and store the destination rect of the text
			SDL_Rect textRect;
			try {
				util::debug(1, "    Printing label text");
				util::printText(renderer, texture, labels[x], xpos, ypos, fontsize,
								0, util::ABR_GRAPH_COLOR1, font, &textRect);
			}
			//Handle potential errors and throw them up the chain
			catch (std::string err) {
				throw "proc::printKeys(): " + (std::string)err;
			}
			catch (const char* err) {
				throw "proc::printKeys(): " + (std::string)err;
			}
			catch (...) {
				throw "proc::printKeuys(): Unknown error occurred while rendering label text";
			}

			util::debug(1, "    Shifting horizontal index forward");
			xpos += colw + 5;
			//Advance the x-position to the other side of the text, adding padding
			xpos += textRect.w + 25;

			if (xpos >= graphinfo.framepos.w * 0.9) {
				util::debug(1, "      Horizontal position exited range, moving to a new line");
				xpos = graphinfo.framepos.x;
				ypos += fontsize * 1.5;
			}
		}

		util::debug(1, "  Key successfully printed, returning...");
		return;
	}


	/*Prints a list of bars to the provided surface, including their values if the flag is set
	* 
	* Precondition: renderer != nullptr AND texture != nullptr AND
	*		(font == nullptr IFF printVals == false)
	* Postcondition: The bars are rendered onto the screen, with the value if specified
	* 
	* Param renderer is the SDL_Renderer required to render onto a surface
	* Param texture is the SDL_Texture that will recieve the bars
	* Param barsList is a vector containing populated graphBar_t objects
	* Param font is a TTF_Font that the values will be printed in (nullptr if printVals is false)
	* Param printVals is a bool representing whether the bar values will be printed
	*/
	void printBars(
		SDL_Renderer* renderer, SDL_Texture* texture, vector<graphBar_t> barsList,
		TTF_Font* font, bool printVals
		) {
		util::debug(1, "printBars()");

		//Iterate through each bar in the list
		util::debug(1, "  Iterating through the passed-in list of bars");
		for (graphBar_t bar : barsList) {
			util::debug(1, "    Rendering bar to screen");
			//Print the bar itself
			util::fillRect(renderer, texture, bar.barRect, bar.color);

			//Print the value of the bar if instructed to do so
			if (printVals) {
				util::debug(1, "    Printing bar hit value");
				util::printText(renderer, texture,
					std::to_string(bar.value).substr(0, std::to_string(bar.value).length() - 4), //I don't want to talk about it
					bar.barRect.x + 5,
					bar.barRect.y + 5, 14, 0, util::ABR_GRAPH_COLOR1, font, nullptr);
			}
		}

		util::debug(1, "  Bars successfully printed, returning...");
		return;
	}

}