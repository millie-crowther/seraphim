cd build
cmake .
make

echo "Verifying fragment shader..."
~/VulkanSDK/1.1.101.0/x86_64/bin/glslangValidator -V ../src/render/shader.frag
