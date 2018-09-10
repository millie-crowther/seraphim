#include "input.h"

#include <fstream>

std::vector<char>
input_t::load_file(std::string filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()){
	    return std::vector<char>();
    }

    size_t filesize = file.tellg();    
    std::vector<char> buffer(filesize);

    file.seekg(0);
    file.read(buffer.data(), filesize);
    file.close();

    return buffer;
}