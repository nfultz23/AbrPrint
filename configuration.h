#ifndef CONFIG_H
#define CONFIG_H


#include <iostream>
#include <fstream>
#include <cstring>
#include <stdint.h>
#include <cctype>


namespace util {

	//This struct contains information about a color for rendering
	struct color_t {
		uint16_t r, g, b, a;
	};

	extern std::string ABR_INPUT_DIR;
	extern std::string ABR_TYPEFACE_DIR;
	extern std::string ABR_TYPEFACE_NAME;
	extern std::string ABR_OUTPUT_DIR;
	extern std::string ABR_OUTPUT_EXT;

	extern bool batch;
	extern bool raw;
	extern bool flagsUsed;

	static int IMG_W = 1200;
	static int IMG_H = 800;

	static color_t ABR_BKGD_COLOR = { 220, 235, 240, 255 };
	static color_t ABR_GRAPH_COLOR1 = { 40, 50, 80, 255 };
	static color_t ABR_GRAPH_COLOR2 = { 185, 200, 225, 255 };

	static color_t ABR_BAR_COLORS[] = { //Need 28-ish
		{210,  70,  70, 255},
		{150,  20,  20, 255},
		{ 75, 210,  70, 255},
		{ 34, 150,  20, 255},
		{ 70,  90, 210, 255},
		{ 20,  60, 150, 255},
		{210,  70, 170, 255},
		{150,  20,  90, 255},

		{210, 145,  70, 255},
		{150,  90,  20, 255},
		{ 70, 210, 140, 255},
		{ 20, 150, 100, 255},
		{130,  70, 210, 255},
		{ 25,  20, 150, 255},
		{210,  70, 150, 255},
		{150,  20, 150, 255},

		{210, 190,  70, 255},
		{150, 140,  20, 255},
		{ 70, 210, 195, 255},
		{ 20, 140, 150, 255},
		{ 70, 150, 210, 255},
		{100,  20, 150, 255},
		{210,  70, 210, 255},
		{143,  20,  80, 255}
	};

	static int ABR_GRAPH_PADDING = 150;
	static int ABR_GRAPH_THICKNESS = 5;

	static const std::string supportedTypes[] = {
		"PNG", "JPEG",
	};


	/*Generates a configuration file if there is not one present
	*
	* Returns -1 if there is a file writing error, 0 OW
	*/
	static int generateConfigFile() {
		//Create a new file for writing
		std::ofstream configFile;
		configFile.open("./AbrPrint.cfg", std::ios::out);
		if (!configFile.is_open()) return -1;
		//Output the default values for each environment variable to the file
		configFile << "ABR_INPUT_DIR\t./" << std::endl;
		configFile << "ABR_TYPEFACE_DIR\t./" << std::endl;
		configFile << "ABR_TYPEFACE_NAME\tConsolas" << std::endl;
		configFile << "ABR_OUTPUT_DIR\t./" << std::endl;
		configFile << "ABR_OUTPUT_EXT\tPNG" << std::endl;
		//Close and save the file
		configFile.close();

		return 0;
	}


	/*Updates AbrPrint's configuration file to accomodate the provided value
	*
	* Precondition: AbrPrint.cfg should be in the same directory as the executable
	* Postcondition: AbrPrint.cfg is unchanged save for the single passed-in value
	*
	* Param fieldName is the name of the environment variable being editied
	* Param value is the value of the variable being passed in
	* Returns -1 if there is a file open error, 0 OW
	*/
	static int UpdateConfig(std::string fieldName, std::string value) {
		bool VSLoc = false;

		//Create a stream from the configuration file
		std::ifstream configInput;
		//Check for the configuration file in the executable's location
		configInput.open("./AbrPrint.cfg", std::ios::in);
		//If the configuration file is not in the executable's location, try
		// the VSCode default location
		if (!configInput.is_open()) {
			configInput.open("./x64/Debug/AbrPrint.cfg", std::ios::in);
			VSLoc = true;
		}
		//If the file failed to open, throw an error
		if (!configInput.is_open())
			if (generateConfigFile() != 0) return -1;

		//Cache the entirety of the configuration file
		std::string configCache = "";
		std::string currEntry;
		while (configInput >> currEntry) {
			//Add the field name to the config cache
			configCache += currEntry + "\t";

			//If the value is being modified, add the altered value to the cache
			if (currEntry == fieldName) {
				configCache += value;
				configInput >> currEntry;
			}
			//Otherwise, add the original value to the list
			else {
				//If it's a single value input field, just add the value
				if (currEntry == "ABR_INPUT_DIR" ||
					currEntry == "ABR_TYPEFACE_DIR" ||
					currEntry == "ABR_TYPEFACE_NAME" ||
					currEntry == "ABR_OUTPUT_DIR" ||
					currEntry == "ABR_OUTPUT_EXT") {
					configInput >> currEntry;
					configCache += currEntry;
				}
				//If it's a multivariate input field add each value
			}
			configCache += "\n";
		}
		configInput.close();

		//Create a stream to the configuration file
		std::ofstream configOutput;
		//Check for the configuration file in the exectable's location
		std::string path = VSLoc ? "./x64/Debug/AbrPrint.cfg" : "./AbrPrint.cfg";
		configOutput.open(path, std::ios::out);
		//Write the cached configuration to the cfg file
		configOutput << configCache;

		//Close the file and make a clean exit
		configOutput.close();
		return 0;
	}


	/*Initializes the active environment variables based on their information
	*  in the configuration file
	*
	* Precondition: AbrPrint.cfg should be in the same directory as the executable
	* Postcondition: All environment variables (The ones defined in this file) are updated
	*		to the values they have in the configuration file
	*
	* Returns -1 if there is a file open error OR -2 if there is an unrecognized name, 0 OW
	*/
	static int AbrPrint_Init() {
		//Create a stream from the configuration file
		std::ifstream configFile;
		//Check for the configuration file in the executable's location
		configFile.open("./AbrPrint.cfg", std::ios::in);
		//If the configuration file is not in the executable's location, try
		// the VSCode default location
		if (!configFile.is_open()) {
			configFile.open("./x64/Debug/AbrPrint.cfg", std::ios::in);
			//If the file failed to open, throw an error
			if (!configFile.is_open())
				if (generateConfigFile() != 0)
					throw "Error finding AbrPrint configuration file";
		}

		//Iterate through the configuration file and set the provided values
		std::string currEntry;
		while (configFile >> currEntry) {
			if (currEntry == "ABR_INPUT_DIR")
				configFile >> ABR_INPUT_DIR;
			else if (currEntry == "ABR_TYPEFACE_DIR")
				configFile >> ABR_TYPEFACE_DIR;
			else if (currEntry == "ABR_TYPEFACE_NAME")
				configFile >> ABR_TYPEFACE_NAME;
			else if (currEntry == "ABR_OUTPUT_DIR")
				configFile >> ABR_OUTPUT_DIR;
			else if (currEntry == "ABR_OUTPUT_EXT")
				configFile >> ABR_OUTPUT_EXT;
			else throw "Unrecognized field detected in AbrPrint configuration file";
		}

		configFile.close();
		return 0;
	}


	/*Handle command line options from a passed in set of flags
	*/
	static int AbrPrint_HandleCLO(
		int argc, char** argv, std::string* source, bool* batch, bool* raw, bool* flagsUsed
	) {
		std::string sourcePath = "";
		bool batchFlag = false;
		bool rawFlag = false;
		bool usedFlag = false;

		//Run through the argument list and handle the help tab first
		for (int x = 1; x < argc; x++) {
			std::string currItem(argv[x]);

			if (currItem != "-h" && currItem != "--help") continue;
			if (x != 1)
				throw "Help flag provided incorrectly, please use \"AbrPrint -h\" for correct flag usage";

			//Handle the help menu here
			if (argc == 2) {
				std::cout << std::endl;
				std::cout << "Welcome to AbrPrint! This program takes in the output of an Abricate" << std::endl;
				std::cout << " analysis and generates a bar graph from the data provided. You can " << std::endl;
				std::cout << " provide a single file for graphing, or generate a series of graphs " << std::endl;
				std::cout << " from a set of files." << std::endl;
				std::cout << std::endl;
				std::cout << "There are several ways to generate these files. For your convenience" << std::endl;
				std::cout << " AbrPrint stores the directory of your Abricate output files. This  " << std::endl;
				std::cout << " directory cache can be changed to where your files are already. Raw" << std::endl;
				std::cout << " input files can be provided as well, details can be found below." << std::endl;
				std::cout << std::endl;
				std::cout << "AbrPrint.exe [source] [options]   -> This will take the file placed " << std::endl;
				std::cout << "                                      in [source] as the input file," << std::endl;
				std::cout << "                                      and will look for it in the   " << std::endl;
				std::cout << "                                      stored source directory. First" << std::endl;
				std::cout << "                                      it will handle any flags given" << std::endl;
				std::cout << "                                      in [options]." << std::endl;
				std::cout << "AbrPrint.exe                      -> This will generate a graph for " << std::endl;
				std::cout << "                                      each file in the stored source" << std::endl;
				std::cout << "                                      directory." << std::endl;
				std::cout << std::endl;
				std::cout << "Available options:" << std::endl;
				std::cout << " -i   --raw-input     Absolute path of an input file/directory" << std::endl;
				std::cout << " -b   --batch         Generate graphs for each file in the source" << std::endl;
				std::cout << " -h   --help [opt]    Makes this help menu. You can provide a flag" << std::endl;
				std::cout << "                      to get more information on how it works" << std::endl;
				std::cout << std::endl;
				std::cout << " -d   --set-source-dir [path]     Sets the directory for the source" << std::endl;
				std::cout << "                                  files to the path provided" << std::endl;
				std::cout << " -o   --set-output-dir [path]     Sets the directory that the graphs" << std::endl;
				std::cout << "                                  will be saved to when created" << std::endl;
				std::cout << " -e   --set-file-type [type]      Sets the type of image file that " << std::endl;
				std::cout << "                                  the graph will save to" << std::endl;
				std::cout << " -f   --set-font [font-name]      Set the font that the graph uses" << std::endl;
				std::cout << std::endl;
			}

			else if (argc == 3) {
				std::string arg(argv[2]);
				if (arg == "-i" || arg == "--raw-input") {
					std::cout << std::endl;
					std::cout << "AbrPrint -i or --raw-input flag" << std::endl;
					std::cout << std::endl;
					std::cout << "AbrPrint usually searches for input files using the path stored in " << std::endl;
					std::cout << " the configuration file, which is set using the -d/--set-source-dir" << std::endl;
					std::cout << " flags. If you want to work with a file outside this without making" << std::endl;
					std::cout << " lasting changes to AbrPrint's configuration, you can use this flag" << std::endl;
					std::cout << " to provide an absolute path to the file you're working with. For  " << std::endl;
					std::cout << " example, if you were to run: " << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint abricateOutput.tab" << std::endl;
					std::cout << std::endl;
					std::cout << " AbrPrint may check in ~/path/to/AbrPrint/abricateOutput.tab. If " << std::endl;
					std::cout << " instead you were to run:" << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint -i C:/path/to/abricate_results/abricateOutput.tab " << std::endl;
					std::cout << std::endl;
					std::cout << " Then AbrPrint will search to the absolute path that you provided" << std::endl;
					std::cout << "  instead of in the directory set in the configuration file." << std::endl;
					std::cout << std::endl;
				}

				else if (arg == "-b" || arg == "--batch") {
					std::cout << std::endl;
					std::cout << "AbrPrint -b or --batch flag" << std::endl;
					std::cout << std::endl;
					std::cout << "By default, AbrPrint will only print a graph for one set of Abricate" << std::endl;
					std::cout << " results at a time. This might become tedious if you have to process" << std::endl;
					std::cout << " lots and lots of result files, so you can instead process files in " << std::endl;
					std::cout << " a batch." << std::endl;
					std::cout << std::endl;
					std::cout << "If you were to simply type the command \"AbrPrint\", then AbrPrint " << std::endl;
					std::cout << " will graph a batch of files automatically, but these will be from " << std::endl;
					std::cout << " the source directory from AbrPrint's configuration. To graph files" << std::endl;
					std::cout << " from another directory, you can run the command: " << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint ~/path/to/abricate_results -b" << std::endl;
					std::cout << std::endl;
					std::cout << " This command will create graphs for each file in the directory you" << std::endl;
					std::cout << " give to it." << std::endl;
					std::cout << std::endl;
				}

				else if (arg == "-h" || arg == "--help") {
					std::cout << std::endl;
					std::cout << "AbrPrint -h or --help flag" << std::endl;
					std::cout << std::endl;
					std::cout << "AbrPrint has a lot going on under the hood, and it's designed to be" << std::endl;
					std::cout << " as customizable as possible. Since there's a lot that you can do to" << std::endl;
					std::cout << " configure AbrPrint's settings, these help menus are here to provide" << std::endl;
					std::cout << " more information about all the options you've got available to you." << std::endl;
					std::cout << std::endl;
					std::cout << "If you're curious to get more information about the list of options " << std::endl;
					std::cout << " AbrPrint takes, you can simply run:" << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint -h" << std::endl;
					std::cout << std::endl;
					std::cout << " AbrPrint will then give you a general help menu with an overview of" << std::endl;
					std::cout << " each of the options and settings you have control over. If you are " << std::endl;
					std::cout << " curious about a particular flag, say the --batch option, just run: " << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint -h --batch" << std::endl;
					std::cout << "         OR" << std::endl;
					std::cout << "      AbrPrint -h -b" << std::endl;
					std::cout << std::endl;
					std::cout << " AbrPrint will now give you a specific help menu all about the batch" << std::endl;
					std::cout << " option." << std::endl;
					std::cout << std::endl;
					std::cout << "If you have a question that doesn't have an answer in any of these" << std::endl;
					std::cout << " help menus, you can look through AbrPrint's source at its GitHub " << std::endl;
					std::cout << " repository (https://github.com/nfultz23/AbrPrint) or you can send" << std::endl;
					std::cout << " an email to noah.fultz314@gmail.com." << std::endl;
					std::cout << std::endl;
				}

				else if (arg == "-d" || arg == "--set-source-dir") {
					std::cout << std::endl;
					std::cout << "AbrPrint -d or --set-source-dir flag" << std::endl;
					std::cout << std::endl;
					std::cout << "To save you from having to type the full path to a folder every time" << std::endl;
					std::cout << " that you want to generate a graph for a file, AbrPrint will store a" << std::endl;
					std::cout << " single path that will be used as the default folder to search for a" << std::endl;
					std::cout << " specified file. By default, this points to the same folder as the" << std::endl;
					std::cout << " executable. If you already have a folder with all of your Abricate " << std::endl;
					std::cout << " result files, you can set that folder as the default for AbrPrint." << std::endl;
					std::cout << " All you have to do is run:" << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint -d ~/absolute/path/to/abr_results/" << std::endl;
					std::cout << "         OR" << std::endl;
					std::cout << "      AbrPrint -d ./relative/path/to/abr_results/" << std::endl;
					std::cout << std::endl;
					std::cout << " Now, AbrPrint will know to search in your abr_results folder for a " << std::endl;
					std::cout << " file you want to generate a graph for. When you run the command:" << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint MyResultFile.tab" << std::endl;
					std::cout << std::endl;
					std::cout << " It will look for ~/absolute/path/to/abr_results/MyResultFile.tab" << std::endl;
					std::cout << std::endl;
				}

				else if (arg == "-f" || arg == "--set-font") {
					std::cout << std::endl;
					std::cout << "AbrPrint -f or --set-font flag" << std::endl;
					std::cout << std::endl;
					std::cout << "AbrPrint allows you to make choices about the font used in the graph" << std::endl;
					std::cout << " output. This is Consolas by default, but changing it is a matter of" << std::endl;
					std::cout << " placing a TrueType Font file in the same folder as the executable, " << std::endl;
					std::cout << " and using the --set-font or -f flag to set the name. If you want to" << std::endl;
					std::cout << " change the font to Comic Sans, for example, all you have to do is  " << std::endl;
					std::cout << " download the Comic_Sans.ttf file, place it next to the AbrPrint exe" << std::endl;
					std::cout << " file, then run:" << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint -f Comic_Sans" << std::endl;
					std::cout << std::endl;
					std::cout << " Now, all text in the graph will be printed in Comic Sans." << std::endl;
					std::cout << std::endl;
				}

				else if (arg == "-o" || arg == "--set-output-dir") {
					std::cout << std::endl;
					std::cout << "AbrPrint -o or --set-output-dir flag" << std::endl;
					std::cout << std::endl;
					std::cout << "AbrPrint can save your graphs anywhere you need them. When you give " << std::endl;
					std::cout << " an output directory, it'll remember it and put all the graphs there" << std::endl;
					std::cout << " until you decide to point it somewhere else. AbrPrint will remember" << std::endl;
					std::cout << " where you told it to place its output to save you having to provide" << std::endl;
					std::cout << " a full file path every single time. If your provided directory does" << std::endl;
					std::cout << " not already exist, AbrPrint will create a folder for you, and place" << std::endl;
					std::cout << " your graphs inside." << std::endl;
					std::cout << std::endl;
					std::cout << "The names of these graph output files are made using the input file " << std::endl;
					std::cout << " names. If you give AbrPrint a file called test123.tab, it will make" << std::endl;
					std::cout << " you a graph called test123_bargraph.png or similar. Be careful when" << std::endl;
					std::cout << " generating graphs this way. If there already exists a file with the" << std::endl;
					std::cout << " name test123_bargraph.png in the output directory, AbrPrint may end" << std::endl;
					std::cout << " up overwriting the file that's there." << std::endl;
					std::cout << std::endl;
				}

				else if (arg == "-e" || arg == "--set-file-type") {
					std::cout << std::endl;
					std::cout << "AbrPrint -e or --set-file-type flag" << std::endl;
					std::cout << std::endl;
					std::cout << "AbrPrint can save your graphs to either a PNG or a JPEG image. The " << std::endl;
					std::cout << " default file type is PNG, but can be changed. You can just run:" << std::endl;
					std::cout << std::endl;
					std::cout << "      AbrPrint -e JPEG" << std::endl;
					std::cout << "        OR" << std::endl;
					std::cout << "      AbrPrint -e jpeg" << std::endl;
					std::cout << std::endl;
					std::cout << " and now the output of a file called test123.tab will be generated " << std::endl;
					std::cout << " as test123_bargraph.jpeg rather than test123_bargraph.png." << std::endl;
					std::cout << std::endl;

				}
			}

			else
				throw "Help flag provided too many arguments, please provide a single flag";

			//Return 1 to signal an immediate exit
			return 1;
		}

		//Iterate through the argument list to unpack the data inside
		for (int x = 1; x < argc; x++) {
			//Store a deep-copy of the current entry in argv to prevent aliasing issues
			std::string currItem(argv[x]);

			//If a source path has been provided, store it
			if (x == 1 && currItem[0] != '-') {
				sourcePath = currItem;
			}

			//If an option is detected, handle the option
			else if (currItem[0] == '-') {
				//Handle a user providing raw input
				if (currItem == "-i" || currItem == "--raw-input") {
					//Check that there was not also a filename provided
					if (sourcePath != "")
						throw "Two input files provided, one raw and one relative";

					//Ensure that an argument was given to the raw input flag
					if (x + 1 >= argc)
						throw "Path argument required for flag -i/--raw-input";
					if (argv[x + 1][0] == '-')
						throw "Path argument required for flag -i/--raw-input";

					//Store and mark the argument as raw input
					sourcePath = std::string(argv[x + 1]);
					rawFlag = true;
				}

				//Handle a user requesting a batch generation
				else if (currItem == "-b" || currItem == "--batch") {
					//Check that no argument was provided to the batch flag
					if (x + 1 < argc) {
						if (argv[x + 1][0] != '-')
							throw "Flag -b/--batch takes no arguments";
					}

					//Store this as a batch job
					batchFlag = true;
					usedFlag = true;
				}

				//Handle a user requesting a reassignment of the source directory
				else if (currItem == "-d" || currItem == "--set-source-dir") {
					//Check that an argument was properly provided
					if (x + 1 >= argc)
						throw "Path argument required for flag -d/--set-source-dir";
					if (argv[x + 1][0] == '-')
						throw "Path argument required for flag -d/--set-source-dir";

					//Store the directory argument as a string
					std::string dir(argv[x + 1]);
					//If there is no foreslash at the end, add one
					if (dir[dir.length() - 1] != '/') dir += "/";
					//Change all backslashes in the path to a foreslash for consistency
					for (int x = 0; x < dir.length(); x++)
						if (dir[x] == '\\') dir[x] = '/';
					//Update the configuration file to store the input directory
					if (UpdateConfig("ABR_INPUT_DIR", dir) < 0)
						throw "Error updating source directory in configuration file";

					usedFlag = true;
				}

				//Handle a user requesting a reassignment of the typeface directory
				else if (currItem == "-t" || currItem == "--set-typeface-dir") {
					//Ensure that a typeface directory was given as an argument
					if (x + 1 >= argc)
						throw "Path argument required for flag -t/--set-typeface-dir";
					if (argv[x + 1][0] == '-')
						throw "Path argument required for flag -t/--set-typeface-dir";

					//Store the path argument as a string
					std::string dir(argv[x + 1]);
					//Change all backslashes in the path to a foreslash for consistency
					for (int x = 0; x < dir.length(); x++)
						if (dir[x] == '\\') dir[x] = '/';
					//Update the configuration file to store the typeface directory
					if (UpdateConfig("ABR_TYPEFACE_DIR", dir) < 0)
						throw "Error updating typeface directory in configuration file";

					usedFlag = true;
				}

				//Handle the user requesting a reassignment of the output directory
				else if (currItem == "-o" || currItem == "--set-output-dir") {
					//Check that a path argument was provided as expected
					if (x + 1 >= argc)
						throw "Path argument required for flag -o/--set-output-dir";
					if (argv[x + 1][0] == '-')
						throw "Path argument required for flag -o/--set-output-dir";

					//Store the directory argument as a string
					std::string dir(argv[x + 1]);
					//If there is no foreslash at the end, add one
					if (dir[dir.length() - 1] != '/') dir += "/";
					//Change all backslashes in the path to a foreslash for consistency
					for (int x = 0; x < dir.length(); x++)
						if (dir[x] == '\\') dir[x] = '/';
					//Update the configuration file to store the output directory
					if (UpdateConfig("ABR_OUTPUT_DIR", dir) < 0)
						throw "Error updating output directory in configuration file";

					usedFlag = true;
				}

				//Handle a user requesting a reassignment of the source directory
				else if (currItem == "-f" || currItem == "--set-font") {
					//Check that a fontname was given properly
					if (x + 1 >= argc)
						throw "Path argument required for flag -f/--set-font";
					if (argv[x + 1][0] == '-')
						throw "Path argument required for flag -f/--set-font";

					//Store the name argument as a string
					std::string name(argv[x + 1]);
					//Update the configuration file to store the font name
					if (UpdateConfig("ABR_TYPEFACE_NAME", name) < 0)
						throw "Error updating typeface name in configuration file";

					usedFlag = true;
				}

				//Handle a user changing the file extension that graphs are saved to
				if (currItem == "-e" || currItem == "--set-file-type") {
					//Check that a name was properly given as a file extension
					if (x + 1 >= argc)
						throw "File extension argument required for flag -e/--set-file-type";
					if (argv[x + 1][0] == '-')
						throw "File extension argument required for flag -e/--set-file-type";

					//Store the extension as a string
					std::string ext(argv[x + 1]);
					//Convert the extension to full capitals
					for (int x = 0; x < ext.length(); x++) ext[x] = std::toupper(ext[x]);

					//Check that the provided extension is a supported file type
					bool supported = false;
					try {
						for (int x = 0; x < sizeof(supportedTypes); x++)
							if (ext == supportedTypes[x]) {
								supported = true;
								break;
							}
					}
					catch (...) { ; }
					//if the file type is unsupported, exit
					if (!supported) throw "Unrecognized file extension provided for flag -e/--set-file-type";

					//Update the configuration file to store the newly provided extension
					if (UpdateConfig("ABR_OUTPUT_EXT", ext) < 0)
						throw "Error updating file extension type in configuration file";

					usedFlag = true;
				}
			}
		}

		//If there is a batch job provided, add a slash to the end of the location given
		if (batchFlag && sourcePath != "") sourcePath += "/";

		//Store the booleans in their global positions
		*source = sourcePath;
		*batch = batchFlag;
		*raw = rawFlag;
		*flagsUsed = usedFlag;

		return 0;
	}

}

#endif