#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "structs.h"
#include <GLFW/glfw3.h>

struct Plane
{
	glm::vec3 normal;
	float d;

	float distance(const glm::vec3& point) const
	{
		return glm::dot(normal, point) + d;
	}
};


class Camera
{
public:
	Camera();
	Camera& operator=(Camera&) = delete;
	Camera(Camera&) = delete;

	MVP& getMatrices();
	void processInput(GLFWwindow* window, float deltaTime);
	void modifyAspectRatio(float newAR);

	glm::vec3 getPosition() const;
private:

	glm::vec3 mOrientation;
	glm::vec3 mFacing;
	glm::vec3 mPosition;
	glm::vec3 mUp;
	glm::vec3 mVelocity;

	float mSpeed = 0.1f;
	float mMouseSens = 100.0f;
	MVP mMatrices;

	float mRange = 5.0f;

	float mFOV = 90.0f;

	std::array<Plane, 2> frustumPlanes;
};