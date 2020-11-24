#ifndef CALL_AND_RESPONSE_H
#define CALL_AND_RESONSE_H

#include "metaphysics/substance.h"

namespace srph {
    class call_t {
    private:
        f32vec3_t position;
        float radius;

        uint32_t index;
        uint32_t hash;
        uint32_t substanceID;
        uint32_t status;

    public:
        struct comparator_t {
            bool operator()(const call_t & a, const call_t & b) const;
        };

        call_t();

        uint32_t get_substance_ID() const;
        f32vec3_t get_position() const;
        float get_radius() const;
        uint32_t get_index() const;
        uint32_t get_hash() const;

        bool is_valid() const;
    };

    class response_t {
    public:
        struct patch_t {
            uint32_t contents;
            uint32_t hash;
            float    phi;
            uint32_t normal;
        };  

        response_t();
        response_t(const call_t & call, std::weak_ptr<substance_t> substance);

        const std::array<uint32_t, 8> & get_normals() const;
        const std::array<uint32_t, 8> & get_colours() const;
        patch_t get_patch() const;

    private:
        patch_t patch;
        std::array<uint32_t, 8> normals;
        std::array<uint32_t, 8> colours;

        uint32_t squash(const vec4_t & x) const;    

    };
}
#endif
