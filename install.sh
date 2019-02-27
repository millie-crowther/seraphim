sudo apt install libxcb1-dev xorg-dev
sudo apt install libglm-dev

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

if cd stb; then 
    git pull
    cd ..
else
    git clone https://github.com/nothings/stb.git
fi

if cd tinyobjloader; then
    git pull
    cd ..
else
    git clone https://github.com/syoyo/tinyobjloader.git
fi

cd ..
