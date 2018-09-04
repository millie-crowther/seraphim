#include <stdexcept>
#include <iostream>
#include <cstring>
#include "engine.h"

#include "sdf.h"
#include "bounds.h"

int 
main(int argc, char ** argv) {
    bool is_debug = false;
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "--debug") == 0){
            is_debug = true;
            break;
        }
    }

    engine_t engine(is_debug);

    try {
        engine.run();
    } catch (const std::runtime_error& e){
	    std::cerr << e.what() << std::endl;
	    return -1;
    }
    
    return 0;
}
