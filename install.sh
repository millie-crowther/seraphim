sudo apt install libxcb1-dev xorg-dev
sudo apt install cmake
sudo apt install make

cd build
if cd glfw; then 
    git pull
    cd ..
else
    git clone https://github.com/glfw/glfw.git
fi
cd ..
