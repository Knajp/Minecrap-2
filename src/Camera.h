#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "structs.h"
#include <GLFW/glfw3.h>

class Camera
{
public:
	Camera();
	Camera& operator=(Camera&) = delete;
	Camera(Camera&) = delete;

	MVP& getMatrices();
	void processInput(GLFWwindow* window);
	void modifyAspectRatio(float newAR);

private:

	glm::vec3 mOrientation;
	glm::vec3 mPosition;
	glm::vec3 mUp;
	float mSpeed = 0.01f;
	float mMouseSens = 100.0f;
	MVP mMatrices;
};