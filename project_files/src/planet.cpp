#include "planet.hpp"
#include <iostream>

PlanetSphere::PlanetSphere(int width, int height) : _h(height), _w(width) {
}

void PlanetSphere::startRotation(glm::vec2 mousePos){
	if (!glm::any(glm::isnan(mousePos))) {
		isRotating = true;
		_initMousePos = mousePos;
	}
	
}

void PlanetSphere::endRotation() {
	isRotating = false;
}


void PlanetSphere::rotate(glm::vec2 mousePos) {
	if (!glm::any(glm::isnan(mousePos))) {
		if (isRotating) {
			// Find delta angle from mouse movement
			double dx = (2.0f * glm::pi<float>() / _w); // 360 deg
			double dy = (glm::pi<float>() / _h);  // 180 deg
			double angleX = (_initMousePos.x - mousePos.x) * dx / 2525.2525;
			double angleY = (_initMousePos.y - mousePos.y) * dy / 2525.2525;

			// Add velocity based on delta
			glm::vec2 mouseDeltaDir = glm::normalize(glm::vec2((float)angleX, -(float)angleY));
			glm::vec2 newRotationVelocity;
			newRotationVelocity.x = glm::clamp(rotationVelocity.x + 0.001f * mouseDeltaDir.x, -0.01f, 0.01f);
			newRotationVelocity.y = glm::clamp(rotationVelocity.y + 0.001f * mouseDeltaDir.y, -0.01f, 0.01f);

			// Too small value check
			if (glm::length(newRotationVelocity) > FLT_EPSILON) {
				rotationVelocity.x = newRotationVelocity.x;
				rotationVelocity.y = newRotationVelocity.y;
			}

			// Update mouse position
			_initMousePos = mousePos;
		}
	}
}

void PlanetSphere::updateRotation() {
	
	// If not in range close to zero, increment towards 0 for both x and y values independently
	if ((rotationVelocity.x > -0.0005f) && (rotationVelocity.x < 0.0005f)) {
		rotationVelocity.x = 0.0f;
	}
	else {
		if (rotationVelocity.x > 0.0) {
			rotationVelocity.x -= 0.0001f;
		}
		else if (rotationVelocity.x < 0.0) {
			rotationVelocity.x += 0.0001f;
		}
	}

	if ((rotationVelocity.y > -0.0005f) && (rotationVelocity.y < 0.0005f)) {
		rotationVelocity.y = 0.0f;
	}
	else {
		if (rotationVelocity.y > 0.0) {
			rotationVelocity.y -= 0.0001f;
		}
		else if (rotationVelocity.y < 0.0) {
			rotationVelocity.y += 0.0001f;
		}
	}

	// Update rotation
	rotationRad = rotationRad + rotationVelocity;
	
}


