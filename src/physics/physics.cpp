#include "physics/physics.h"

#include "core/scheduler.h"

#include <chrono>
#include <functional>
#include <iostream>

using namespace srph;

std::default_random_engine engine;

physics_t::physics_t(){
    quit = false;
    thread = std::thread(&physics_t::run, this);
}

physics_t::~physics_t(){
    quit = true;
    thread.join();
}

void physics_t::run(){
    auto t = scheduler::clock_t::now();
    auto previous = std::chrono::steady_clock::now();
       
    while (!quit){
        frames++;
        
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count() / 1000000.0;

        previous = now;

        std::vector<collision_t> collisions;
        
        {
            std::lock_guard<std::mutex> lock(matters_mutex);
            
            // reset acceleration and apply gravity force
            for (auto & m : matters){
                if (m->get_position()[1] > -90.0){
                    m->reset_acceleration();
                }
            }

            // collide awake substances with each other
            for (uint32_t i = 0; i < matters.size(); i++){
                for (uint32_t j = i + 1; j < matters.size(); j++){
                    collisions.emplace_back(delta, matters[i].get(), matters[j].get());
                }
            }
            
            // collide awake substances with asleep substances
            for (auto awake_matter : matters){
                for (auto asleep_matter : asleep_matters){
                    collisions.emplace_back(delta, asleep_matter.get(), awake_matter.get());
                }
            }
        }
        
        // correct all present collisions and anticipate the next one
        for (auto & c : collisions){
            if (c.is_intersecting()){
                correct(c);
            } else if (c.is_anticipated()){
                delta = std::min(delta, c.get_estimated_time());
            }
        }
       
        {
            std::lock_guard<std::mutex> lock(matters_mutex);
 
            // apply acceleration and velocity changes to matters
            for (auto m : matters){
                m->physics_tick(delta);
            } 

            // try to put matters to sleep
            for (uint32_t i = 0; i < matters.size();){
                auto m = matters[i]; 
            
                if (m->is_inert()){
                    std::cout << "Matter going to sleep!" << std::endl;
                    asleep_matters.push_back(m);
                    matters[i] = matters[matters.size() - 1];
                    matters.pop_back();
                } else { 
                    i++;
                }
            }
        }

        t += std::chrono::microseconds(static_cast<int64_t>(delta * 1000000.0));
        std::this_thread::sleep_until(t);
    }
}

void physics_t::correct(collision_t & c){
    auto pair = c.get_matters();
    auto points = &contact_points[pair];

    // remove all points which are no longer contact points
    //*
    for (unsigned int i = 0; i < points->size();){
        auto x1 = pair.first->to_local_space(points->at(i));
        auto x2 = pair.second->to_local_space(points->at(i)); 

        if (
            pair.first->get_sdf()->phi(x1)  > 0 || 
            pair.second->get_sdf()->phi(x2) > 0 
        ){
            points->at(i) = points->at(points->size() - 1);
            points->pop_back();
        } else {
            i++;
        }
    }
    //*/

    // check if max point limit reached 
    if (points->size() < max_contact_points){
        points->push_back(c.get_position());
    } else {
        /*/
        int min_i = 0;
        double min_distance = vec::length(points->at(0) - c.get_position());
        for (unsigned int i = 1; i < points->size(); i++){
            double distance = vec::length(points->at(i) - c.get_position());
            if (distance < min_distance){
                min_i = i;
                distance = min_distance;
            }
        }
        // */

        std::uniform_int_distribution<int> distr(0, points->size() - 1);
        int min_i = distr(engine);
    
        points->at(min_i) = c.get_position();
    }

 //   std::cout << "points size: " << points->size() << std::endl;

    // find mean of points to find centre of collision surface
    vec3_t total = std::accumulate(points->begin(), points->end(), vec3_t());
    c.correct(total / points->size());
}

void physics_t::register_matter(std::shared_ptr<matter_t> matter){
    std::lock_guard<std::mutex> lock(matters_mutex);
    matters.push_back(matter);
}
    
void physics_t::unregister_matter(std::shared_ptr<matter_t> matter){
    std::lock_guard<std::mutex> lock(matters_mutex);
    
    auto it = std::find(matters.begin(), matters.end(), matter);
    if (it != matters.end()){
        matters.erase(it);
    } else {
        it = std::find(asleep_matters.begin(), asleep_matters.end(), matter);
        if (it != asleep_matters.end()){
            asleep_matters.erase(it);
        }
    }
}

int physics_t::get_frame_count(){
    int f = frames;
    frames = 0;
    return f;
}
