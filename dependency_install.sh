sudo apt install libxcb1-dev xorg-dev
sudo apt install libglm-dev

if cd build/glfw; then 
    git pull
    cd ../..
else
    cd build
    git clone https://github.com/glfw/glfw.git
    cd ..
fi

