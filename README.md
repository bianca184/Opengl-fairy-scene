# OpenGL Fairy World Scene 🧚‍♀️
Overview

This project implements a photorealistic 3D fairy world using modern OpenGL.
The goal of the project is to demonstrate real-time rendering techniques such as lighting, textures, animations, and camera control.

The scene represents a magical fairy environment containing multiple 3D objects, animated elements, and environmental effects.

The project was developed using:
-OpenGL
-GLFW
-GLM
-stb_image
-tinyobjloader

Features
-Camera and Scene Interaction
-Camera movement using keyboard controls
-Mouse control for scene rotation
-Scene transformations:
                        -translation
                        -rotation
                        -scaling
-Animated presentation of the environment

Lighting System
The scene includes multiple light sources:
-global light (ambient)
-local lights placed in the environment
-spot lighting effects
Lighting calculations simulate realistic illumination for the objects in the scene.

Rendering Modes
The scene can be rendered in multiple visualization modes:
-solid rendering
-wireframe mode
-polygon mode
-smooth shading

Texture Mapping and Materials
Objects in the scene use texture mapping and material definitions to achieve a realistic appearance.
The project includes:
-detailed textures
-texture mapping applied to different objects
-material properties that interact with the lighting system

Shadows
The scene demonstrates shadow generation, allowing objects to cast realistic shadows on the environment.

Animations
Several elements in the scene are animated, including:
-fairy wings animation
-environmental movement
-animated presentation of the scene

Environment Effects
The fairy world includes environmental elements such as:
-wind simulation
-rain effects
-dynamic scene components
These effects help create a more immersive and dynamic environment.

Scene Description
The environment represents a fantasy fairy world containing multiple objects such as:
-trees
-mushrooms
-flowers
-magical structures
-animated fairy character
The scene emphasizes visual detail and atmosphere to create a magical environment.

Controls
Key	Action
W / A / S / D	  -- Move camera
Mouse	-- Rotate camera
Other keys	Scene interaction / rendering modes
Project Structure
Street/
├── main.cpp
├── Shader.cpp
├── Window.cpp
├── Camera.cpp
├── shaders/
├── models/
├── textures/

Requirements
To run the project you need:
-Visual Studio
-OpenGL
-GLFW
-GLM

Author
Iacob Bianca Lavinia
