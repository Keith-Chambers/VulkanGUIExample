# VulkanGUIExample

A repository containing a crude and experimental GUI application written in C++ and Vulkan. This is the code I'm currently working on to learn Vulkan for a GUI project I have planned. The actual project will be rewritten in Zig but I'm uploading this here as it might help others trying to learn Vulkan. 



What's demonstrated in the repository. 

- Multiple Pipelines + shaders (One for textures and another for simple primative shapes)

- Somewhat efficient Frame Buffer recreation on screen resize (Moreso than examples I've seen which just recreate everything from scratch)

- Text rendering (Font loading done with FreeType)

- Fixed frames per second

- Basic pipeline draw order

- Fixed pixel sizes (Don't stretch drawn UI components on screen resize)



I've only tested this on Linux so I can't gaurantee it will work on any other OS. If it doesn't though it shouldn't be too difficult to fix as no strictly Linux API's were used. 

### Requirements

- C++ compiler

- CMake

- Vulkan lib

- Glfw lib

- FreeType lib (Included but may need to be installed)



### How to build

```
git clone https://github.com/Keith-Chambers/VulkanGUIExample 
cd VulkanGUIExample 
cmake .
make
```

The executable will be located inside the bin folder in the project.

**Note:** Before you build, you will want to edit the config.cpp file as it contains variable definitions that you will likely want to change, including a system path for the font to load that may not exist on your OS. 



If you can't build the project, or have any questions about the code, feel free to reach out. 

The code was originally based off of https://vulkan-tutorial.com but has been pretty heavily modified. 
