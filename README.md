# Procedural Planet

## Overview

This application is developed using OpenGL, FreeGLUT, and C++ to create a procedurally generated planet based on a seed, with various user-customizable options. It employs GLSL for both Fragment and Vertex shading to enhance the visual effects. The application leverages advanced techniques such as matrix operations, signed distance fields, ray marching, Phong shading, Gaussian distributions, fractal Brownian motion, and procedural Perlin noise to generate and render the planet. Additionally, it includes features for saving progress and undoing actions, along with a wide range of options for manipulating shaders and the planet's appearance.

## Goal/Motivation

- Realistic Terrain
- Mountains
- Varying Color
- Continents

![Motivation Image 1](https://raw.githubusercontent.com/svparekh/ProceduralPlanet/main/assets/image1.png)

*Planet Concept 1*

![Motivation Image 3](https://raw.githubusercontent.com/svparekh/ProceduralPlanet/main/assets/image3.png)

*Planet Concept 2*

![Motivation Image 2](https://raw.githubusercontent.com/svparekh/ProceduralPlanet/main/assets/image2.png)

*Procedural Terrain Generation Concept*

## Results

![Result image of planet](https://raw.githubusercontent.com/svparekh/ProceduralPlanet/main/assets/image4.png)

## Controls

```text
############################## -- Controls: -- ##############################
    Mouse:
        - Press and hold RIGHT MOUSE BUTTON to ORBIT around the PLANET
        - Press and hold LEFT MOUSE BUTTON to ROTATE the PLANET
        - Use the mouse SCROLL WHEEL to ZOOM in/out
    Keyboard:
        - Tap 's' to reload shaders
        - Tap 'p' to toggle performance mode
            Default ON. Turns off soft shadows + reflections + water normals
            - Tap 'h' to toggle hard shadows in performance mode
        - Tap 'f' to toggle placement mode, enables terrain editing
            Edits here do not save automatically
            - Tap the LEFT MOUSE BUTTON to generate terrain at the mouse cursor's location on the planet
            - Tap 'CTRL' + 'z' to remove last edit (including those loaded from the config file)
            - Tap 'CTRL' + 's' to save all changes to config file
            Control the radius:
                - Tap 'c' to increment the radius by 0.05
                - Tap 'z' to decrement the radius by 0.05
            Control the height:
                - Tap 'e' to increment the height by 0.05
                - Tap 'q' to decrement the height by 0.05
        - Tap 'r' to load/reload the terrain edits saved in the config file
        - Tap ESC to exit the application
        Control amount of detail:
            - Tap 'c' to increment the fractal brownian motion (FBM) iterations
            - Tap 'z' to decrement the fractal brownian motion (FBM) iterations
        Control seed:
            - Tap 'e' to increment the procedural world seed
            - Tap 'q' to decrement the procedural world seed
#############################################################################
```

## Techniques Used

### Rotations

- Arcball camera (Orbiting camera)
- Planet has velocity based rotation from mouse movement
- Rotate on Y axis, then X axis
    - `glm::rotate` for camera
    - Rotation matrices (turned to vectors to multiply out) for planet
- Send camera position and TBN matrix to frag shader (to calculate ro and rd)

### Shading

![Ray marching visualization image](https://raw.githubusercontent.com/svparekh/ProceduralPlanet/main/assets/image5.png)

*Visualization of how Ray Marching works*

- Ray Marching
    
    ```glsl
    const int MAX_STEPS = 64; // 256
    const float MAX_DIST = 50.0; // 500
    const float EPSILON = 0.001; // 0.00001
    ```
    
- Signed Distance Field (SDF) + Displacement based on noise
    
    ```glsl
    float sphereSDF(vec3 p, vec3 center, float radius) { return length(p - center) - radius; }
    float sphereDist = sphereSDF(p, vec3(0.0), planetBaseSize + displace(p));
    ```
    
- Union of objects for flat water sphere
    
    ```glsl
    vec2 res = (sphere.x < waterSphere.x) ? sphere : waterSphere;
    ```
    
- Normals (calculated using gradients, `SDF(r + 0.001) - SDF(r - 0.001)`
- Hard Shadows only mode vs Soft Shadows + Reflections + Water Normals
- Phong shading, Blinn-Phong available but doesn’t look as good in this environment

### Terrain Generation

![Perlin noise generation visualization image](https://raw.githubusercontent.com/svparekh/ProceduralPlanet/main/assets/image6.png)

*Visualization of the generational process concerning Perlin Noise*

Procedural:

- Simple Procedural Noise (Perlin)
    - Generated by calculating four corners, applying permutations to it, applying gradients to it, then calculating the normal to dot with calculated points, add back the corners, and some tweaks
- Fractal Brownian Motion
    
    ```glsl
        float fbm(vec3 samplePoint){
            float sum = 0;
            float amplitude = 1;
            float frequency = 1;
            // increase frequency, decrease amplitude per iteration
            for (int i = 0; i < fbmIterations; i++) {
                sum += getNoiseAt(samplePoint * frequency) * amplitude; 
                frequency *= 2;
                amplitude *= 0.5;
            }
            return sum;
        }
    ```
    

User Control:

- Generates a Gaussian based lump with given size, stays there no matter the seed but updates to terrain around it
- User created islands are stored in config file
    - Format: 5 lines per mound
    - `1: center.x, 2: center.y, 3: center.z, 4: radius, 5: height`
        - center is a xyz location on a unit sphere
- User can click to add terrain
    - Undo and Save also available

#### Colored Terrain

- Height based color with material lerp depending on height (for smooth fade)
    
    ```glsl
    float height = length(pos - center);
    float lerp_amount = saturate((height - (planetBaseSize + [THRESHOLD])) / 0.005);
    ```
    
- Steepeness based color with material lerp depending on steepness AND height
    
    ```glsl
    float steepness = 1.0 - dot(normal, normalize(pos - center));
    float lerp_amount = saturate((steepness - [THRESHOLD]) / 1.0) * (1.0 - height_multiplier);
    ```
    

## Lessons Learned & Moving Forward

- Lessons Learned:
    - Ray marching and SDFs - Must be continuous or weird artifacts emerge, graphics intensive
    - SDF Rotation - Must apply rotation to every single point, so rotation must be saved
    - Color lerping - Lerping for a correct fade must be done dynamically
    - Noise + FBM - Function creation is extremely intensive
- Looking Forward:
    - Orbital physics
    - Remove floating point precision errors
    - Quaternions for smoother rotations
    - Faster ray marching, remove inefficiencies
    - Water translucency based on depth

## Sources

**Perlin Noise**

- https://www.cs.umd.edu/class/spring2018/cmsc425/Lects/lect13-2d-perlin.pdf

**Ray Marching**

- https://celarek.at/tag/ray-marching/

**Phong Shading**

- https://learnopengl.com/Advanced-Lighting/Advanced-Lighting

**Signed Distance Fields**

- https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-34-signed-distance-fields-using-single-pass-gpu#:~:text=A%20signed%20distance%20field%20is,outside%20the%20object%20is%20applied

**Youtube Videos**

- https://www.youtube.com/watch?v=lctXaT9pxA0
- https://www.youtube.com/watch?app=desktop&v=z29BiRqnqiA