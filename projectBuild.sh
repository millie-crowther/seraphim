cd build
cmake .
make
~/VulkanSDK/1.1.73.0/x86_64/bin/glslangValidator -V ../src/shaders/shader.vert
~/VulkanSDK/1.1.73.0/x86_64/bin/glslangValidator -V ../src/shaders/shader.frag
./blaspheme "$@"
