#include "engine.h"

#include <stdexcept>
#include <iostream>

int main() {
    engine_t engine;

    try {
        engine.run();
    } catch (const std::runtime_error& e){
	std::cerr << e.what() << std::endl;
	return -1;
    }

    return 0;
}
