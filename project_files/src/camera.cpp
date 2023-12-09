#include "camera.hpp"

/*####################
####  Constructor ####
####################*/

Camera::Camera(glm::vec3 position, int width, int height, float fovy) {
	_pos = position;
	_width = width;
	_height = height;
	_fovy = fovy;
	_recalcUpAndTangent();
}


/*####################
####   Movement   ####
####################*/

// From https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}


void Camera::rotate(glm::vec2 mousePos) {
	if (isRotating) {
		_recalcUpAndTangent();
		glm::vec4 position(_pos, 1.0);
		glm::vec4 pivot(_pivot, 1.0);

		// Help from https://asliceofrendering.com/camera/2019/11/30/ArcballCamera/
		double deltaAngleX = (2.0 * glm::pi<float>() / _width); // a movement from left to right = 2*PI = 360 deg
		double deltaAngleY = (glm::pi<float>() / _height);  // a movement from top to bottom = PI = 180 deg
		double xAngle = (_initMousePos.x - mousePos.x) * deltaAngleX / 2525.2525;
		double yAngle = (_initMousePos.y - mousePos.y) * deltaAngleY / 2525.2525;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		float cosAngle = glm::dot(_pivot - _pos, _up);
		if (cosAngle * sgn(yAngle) > 0.99f)
			yAngle = 0.0;

		glm::mat4x4 rotationMatrixX(1.0f);
		rotationMatrixX = glm::rotate(rotationMatrixX, (float)xAngle, _up);
		position = (rotationMatrixX * (position - pivot)) + pivot;

		glm::mat4x4 rotationMatrixY(1.0f);
		rotationMatrixY = glm::rotate(rotationMatrixY, (float)yAngle, _tangent);
		glm::vec3 newPos = (rotationMatrixY * (position - pivot)) + pivot;

		_recalcUpAndTangent();

		_initMousePos = mousePos;

		if (glm::length(newPos) > FLT_EPSILON) {
			_pos.x = newPos.x;
			_pos.y = newPos.y;
			_pos.z = newPos.z;
		}
	}
}

void Camera::startRotation(glm::vec2 mousePos) {
	isRotating = true;
	_initPos = _pos;
	_initMousePos = mousePos;
}

void Camera::endRotation() {
	isRotating = false;
}

void Camera::zoom(float amount) {
	glm::vec3 newPos = _pos * (1.0f + amount);
	if ((glm::length(newPos) < 10.0f) && (glm::length(newPos) > 0.556f)) {
		_pos = newPos;
	}
}


/*####################
####   Matricies  ####
####################*/

glm::mat3 Camera::getTBNMatrix() {
	_recalcUpAndTangent();
	// eye, center, up
	return glm::mat3(glm::normalize(_tangent), glm::normalize(_up), glm::normalize(_pivot - _pos));
}

glm::mat4 Camera::getViewMatrix() {
	
	// eye, center, up
    return glm::lookAt(_pos, _pivot, _up);
}

glm::mat4 Camera::getProjectionMatrix() {
	float aspect = (float)(_width) / (float)(_height);
	return glm::perspective(glm::radians(_fovy), aspect, 0.1f, 100.0f);
}


/*####################
####    Update    ####
####################*/

void Camera::_recalcUpAndTangent() {
	glm::vec3 camDir = glm::normalize(_pivot - _pos);
	_tangent = glm::normalize(glm::cross(WORLD_UP, camDir));
	_up = glm::cross(camDir, _tangent);
}