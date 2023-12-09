#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


/*####################
####    Globals   ####
####################*/

#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)


/*####################
####     Class    ####
####################*/


class Camera
{
public:
    bool isRotating = false;                                                // Is camera currently rotating?

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 2.0f), 
        int width = 1, int height = 1, float fovy = 45.0f);                 // Constructor w/ custom values

    glm::mat4 getViewMatrix();                                              // Get view matrix
    glm::mat4 getProjectionMatrix();                                        // Get view * projection matrix
    glm::mat3 getTBNMatrix();                                               // Get Tangent, Up, Center matrix

    void rotate(glm::vec2 mousePos);                                        // Rotate camera about (0, 0, 0)
    void zoom(float amount);                                                // Zoom camera in or out
    void startRotation(glm::vec2 mousePos);                                 // On rotation start
    void endRotation();                                                     // On rotation complete
    inline glm::ivec2 getSize() { return glm::ivec2(_width, _height); }
    inline glm::vec3 getCoords() { return _pos; }
    
private:
    void _recalcUpAndTangent();                                             // Recalculate the up and tangent vectors

    // camera values
    int _width, _height;		                                            // Width and height of the window
    float _fovy;		                                                    // Vertical field of view in degrees
    
    glm::vec3 _pivot = glm::vec3(0.0f);                                     // Point the camera orbits
    glm::vec3 _pos;                                                         // Camera coordinates
    glm::vec3 _center;                                                      // Camera center
    glm::vec3 _up;                                                          // Camera up
    glm::vec3 _tangent;                                                     // Camera right vector
    glm::vec3 _initPos;                                                     // Initial rotation
    glm::vec2 _initMousePos;                                                // Initial mouse position
    float _zoomAmount = 0.5f;                                               // Current zoom amount
};

