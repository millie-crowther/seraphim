#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include <string>

class input_t {
private:

public:
    static std::vector<char> load_file(std::string filename); 
};

#endif