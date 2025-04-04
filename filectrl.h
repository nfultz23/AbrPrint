#ifndef FILECTRL_H
#define FILECTRL_H


#include <fstream>


namespace filectrl {
	using std::ifstream; using std::string;

	/*Prompts the user for input to open a file for analyzation
	*
	* Precondition: Input name must match the name of a file in the abr_files directory
	* Postcondition: The provided argument will now be populated with the user-input file name
	*
	* Param filename is a pointer that will be populated with the user-input file name
	* Returns an ifstream referencing the file input by the user during the function
	*/
	ifstream loadFile(string* filename);

}


#endif