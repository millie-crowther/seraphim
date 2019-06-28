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

    blaspheme_t engine(is_debug);
    std::shared_ptr<camera_t> camera = std::make_shared<camera_t>(&engine);
    if (auto renderer = engine.get_renderer().lock()){
        renderer->set_main_camera(camera);
    }
    
    engine.run();
    
    return 0;
}
