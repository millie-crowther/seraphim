shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_visible;

shared substance_t shadows[gl_WorkGroupSize.x];
shared uint shadows_visible;

shared light_t lights[gl_WorkGroupSize.x];
shared uint lights_visible;

aabb_t light_aabb;
aabb_t surface_aabb;

shared uint octree[octree_pool_size];

shared bool hitmap[octree_pool_size];
shared vec4 workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];