#include <stdexcept>
#include <iostream>
#include <cstring>

#include "core/blaspheme.h"

int 
main(int argc, char ** argv) {
    bool is_debug = false;
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "--debug") == 0){
            is_debug = true;
            break;
        }
    }

    blaspheme_t blaspheme(is_debug);

    try {
        blaspheme.run();
    } catch (const std::runtime_error& e){
	    std::cerr << e.what() << std::endl;
	    return -1;
    }
    
    return 0;
}
