#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

namespace CharRemove {

	//return true if an input char needs to be removed (takes in a string containing all characters that are allowed)
	//takes a char and a string of allowed characters
	bool predicate(char, std::string);

	//check if a string contains any unwanted characters and removes them
	//takes a pointer to the string to clean and the name of the file containing all allowed characters(as a string)
	void clean(std::string*, std::string);

}