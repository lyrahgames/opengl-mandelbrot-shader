# OpenGL GPGPU: GLSL Mandelbrot Shader

Simple application showing how to use the GLSL fragment shader to compute and draw the Mandelbrot set directly on the GPU.
This is not a sophisticated method and only well suited for learning purposes.

![](example_output.png)

## Requirements
- C++14
- Linux | Windows
- glbinding
- GLFW
- GLM

## Build and Run with build2

Open a terminal in the project root and run the following command to build and run the code.

    b test

On Windows, I recommend to use the Nuwen MinGW compiler distribution with custom configurations to find the libraries.
For example:

    b \
        config.cxx=g++ \
        "config.cxx.poptions=-IC:/MinGW/include" \
        "config.cxx.loptions=-LC:/MinGW/libs" \
        "config.cxx.coptions=..."
    b test

## Usage

- Escape: Quit the program.
- Left Mouse Button: Move origin.
- Scroll Mouse Wheel: Zoom in or out.