#ifndef SERAPHIM_SUBSTANCE_H
#define SERAPHIM_SUBSTANCE_H

#include <memory>

#include "form.h"
#include "matter.h"

#include "maths/matrix.h"

typedef struct substance_t {
	struct data_t {
		float near;
		float far;
		uint32_t sdf_id;
		float _2;

		 srph::f32vec3_t r;
		uint32_t id;

		float transform[16];

		 data_t();
		 data_t(float near, float far, const srph::f32vec3_t & r,
			uint32_t id);

		struct comparator_t {
			bool operator() (const data_t & a,
					 const data_t & b)const;
		};
	};

	 substance_t();
	 substance_t(form_t * form, matter_t * matter, uint32_t i);

	data_t get_data(const vec3 * eye_position);

	uint32_t id;
	form_t form;
	matter_t matter;
} substance_t;

#endif
