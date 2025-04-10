#ifndef FILECTRL_H
#define FILECTRL_H


#include <fstream>
#include <vector>
#include <iostream>


namespace filectrl {
	using std::ifstream; using std::string;

	/*Takes in a directory and filename, and opens an input stream to read data
	*  from the file.
	*
	* Param directory is the directory that the file is found in. Can be absolute
	*  or relative to the executable
	* Param filename is the name of the file within the directory to open
	*
	* Return a filestream object that allows the system to read data from the file
	*/
	ifstream loadFile(string directory, string filename);


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
	std::vector<std::string> gatherFilenames(std::string loc, std::string* directory);


	/*
	*/
	void saveGraphToFile(
		std::string sourceName, std::string fileType, std::string directory,
		std::string graphType, SDL_Texture* graph
	);

}


#endif