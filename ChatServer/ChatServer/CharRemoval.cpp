#include "CharRemoval.h"

namespace CharRemove {

	//return true if an input char needs to be removed (takes in a string containing all characters that are allowed)
	bool predicate(char c, std::string allowed) {
		if (allowed.find_first_of(c) != std::string::npos) {
			return false;
		}
		else {
			return true;
		}
	}


	//check if a string contains any unwanted characters and removes them
	void clean(std::string* str, std::string filename) {

		//read the file containing all allowed characters
		std::ifstream ifs(filename);
		std::string allowed((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

		//remove characters from array
		for (std::string::size_type i = 0; i < str->size(); ++i) {
			if (predicate((*str)[i], allowed)) {
				str->replace(i, 1, "-");
			}
		}
	}

}