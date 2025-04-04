#include <iostream>
#include <fstream>
#include <cstring>

#include "../filectrl.h"


namespace filectrl {
	using std::ifstream; using std::string; using std::ios;

	/*Prompts the user for input to open a file for analyzation
	*
	* Precondition: Input name must match the name of a file in the abr_files directory
	*
	* Returns an ifstream referencing the file input by the user during the function
	*/
	ifstream filectrl::loadFile(string* filename) {
		//Take in the file name (within the abr_files directory) to process
		std::cout << "Please enter the file to process:" << std::endl << "?> ";
		std::cin >> *filename;

		//Open the file and ensure that it's ready for reading
		std::ifstream src;
		src.open(ABR_INPUT_DIR + *filename, ios::in);
		if (!src.is_open()) throw "filectrl::loadFile(): Error opening file " + *filename + "\n";

		//Return the filestream
		return src;
	}

}