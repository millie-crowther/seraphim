cd build
cmake .
make
echo "Verifying vertex shader..."
~/VulkanSDK/1.1.73.0/x86_64/bin/glslangValidator -V ../src/shaders/shader.vert
echo "Verifying fragment shader..."
~/VulkanSDK/1.1.73.0/x86_64/bin/glslangValidator -V ../src/shaders/shader.frag
./blaspheme "$@"
