#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "procedural.hpp"

class PlanetSphere
{
public:
	bool isRotating = false;

	PlanetSphere(int width = 1, int height = 1);
	TerrainEditor terrain;

	void startRotation(glm::vec2 mousePos);
	void endRotation();
	void rotate(glm::vec2 mousePos);
	void updateRotation(); // Sets radians of rotation based on velocity

	glm::vec2 rotationRad = glm::vec2(0.0f, 0.0f);
	glm::vec2 rotationVelocity = glm::vec2(0.0f, 0.0f);
private:
	int _h, _w;
	glm::vec2 _initMousePos = glm::vec2(0.0f, 0.0f);
	glm::vec2 _rotationAcceleration = glm::vec2(0.0f, 0.0f);
};

