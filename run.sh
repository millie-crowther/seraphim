cd build
cmake .
make

echo "Verifying vertex shader..."
~/VulkanSDK/1.1.73.0/x86_64/bin/glslangValidator -V ../src/shaders/shader.vert
echo "Verifying fragment shader..."
~/VulkanSDK/1.1.73.0/x86_64/bin/glslangValidator -V ../src/shaders/shader.frag

# export VK_LAYER_PATH=/Users/amelia/vulkansdk-macos-1.1.92.1/macOS/etc/vulkan/explicit_layer.d
./blaspheme "$@"
# unset VK_LAYER_PATH