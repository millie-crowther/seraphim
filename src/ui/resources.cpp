#include "ui/resources.h"

#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

using namespace srph;

std::string resources::load_file(std::string filename){
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();

    return  buffer.str();

    // std::ifstream file(filename, std::ios::ate | std::ios::binary);

    // if (!file.is_open()){
	//     return "";
    // }

    // size_t filesize = file.tellg();    
    // std::vector<char> buffer(filesize);

    // file.seekg(0);
    // file.read(buffer.data(), filesize);
    // file.close();
    // return std::string(buffer.data());
}
