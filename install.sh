sudo apt install libxcb1-dev xorg-dev

cd build
if cd glfw; then 
    git pull
    cd ..
else
    git clone https://github.com/glfw/glfw.git
fi

if cd VulkanMemoryAllocator; then 
    git pull
    cd ..
else
    git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
fi

cd ..