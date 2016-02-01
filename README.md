#Line Integral Convolution

Line Integral Convolution using openGL shaders in GLSL.

![alt text](images/sample.gif "screenshot")

##Compile It

Depends on openGL headers.  On debian based systems:

sudo apt-get install mesa-common-dev  
sudo apt-get install libglu1-mesa-dev  
sudo apt-get install freeglut3-dev  

make

##Run It

./LIC

Usage:  
./LIC [vertex shader] [fragment shader] [lic number of steps]  
Example usage:  
./LIC ./src/vertex-shader ./src/fragment-shader 80


##Notes

You need the openGL headers installed.  Tested for linux.  Should compile for both mac and linux.  Check stdout for the UI controls.
