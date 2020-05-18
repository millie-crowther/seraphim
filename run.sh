./build.sh
[ $? -eq 0 ]  || exit 1

cd build
# export VK_LAYER_PATH=/Users/amelia/vulkansdk-macos-1.1.92.1/macOS/etc/vulkan/explicit_layer.d
./seraphim "$@"
# unset VK_LAYER_PATH
