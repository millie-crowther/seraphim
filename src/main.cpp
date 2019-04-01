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

    blaspheme_t(is_debug).run();
    
    return 0;
}
