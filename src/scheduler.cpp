#include "scheduler.h"

bool
scheduler_t::is_initialised = false;

float
scheduler_t::max_framerate = 60;

scheduler_t::scheduler_t(){

}

std::shared_ptr<scheduler_t>
scheduler_t::create(){
    return std::make_shared<scheduler_t>();
}