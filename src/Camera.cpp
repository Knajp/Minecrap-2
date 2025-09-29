#include "Camera.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>
MVP& Camera::getMatrices()
{
	return mMatrices;
}

void Camera::processInput(GLFWwindow* window, float deltaTime)
{
	glm::vec3 acceleration(0.0f);
	//KEYBOARD INPUT
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		acceleration += mFacing * mSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		acceleration += mFacing * -mSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		acceleration += glm::normalize(glm::cross(mUp, mFacing)) * mSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		acceleration += glm::normalize(glm::cross(mUp, mFacing)) * -mSpeed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		acceleration += mUp * -mSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		acceleration += mUp * mSpeed;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


	mVelocity += acceleration;

	mVelocity -= mVelocity * 0.6f * deltaTime;

	mPosition += mVelocity * deltaTime;

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	int height, width;
	glfwGetFramebufferSize(window, &width, &height);
	float pitch = mMouseSens * (float)(mouseY - (height / 2)) / height;
	float yaw = mMouseSens * (float)(mouseX - (width / 2)) / width;

	glm::vec3 newOrient = glm::rotate(mOrientation, glm::radians(pitch), glm::normalize(glm::cross(mOrientation, mUp)));

	if (!(glm::angle(newOrient, mUp) <= glm::radians(5.0f) || glm::angle(newOrient, -mUp) <= glm::radians(5.0f)))
		mOrientation = newOrient;

	mOrientation = glm::rotate(mOrientation, glm::radians(-yaw), mUp);
	mFacing = glm::rotate(mFacing, glm::radians(-yaw), mUp);

	glfwSetCursorPos(window, width / 2, height / 2);
	mMatrices.view = glm::lookAt(mPosition, mPosition + mOrientation, mUp);


	// MOUSE INPUT

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		for (float rayLength = 0.0f; rayLength <= 5.5f; rayLength += 0.1f)
		{
			glm::vec3 rayCoord = mPosition + mOrientation * rayLength;

			glm::ivec3 intCoord = { (int)rayCoord.x, (int)rayCoord.y, (int)rayCoord.z };

			//glm::ivec2 chunkCoord = {intCoord.x / 16}
		}
	}

	// frustum calc

	glm::mat4 VP = mMatrices.proj * mMatrices.view;

	// Left
	frustumPlanes[0].normal.x = VP[0][3] + VP[0][0];
	frustumPlanes[0].normal.y = VP[1][3] + VP[1][0];
	frustumPlanes[0].normal.z = VP[2][3] + VP[2][0];
	frustumPlanes[0].d = VP[3][3] + VP[3][0];

	// Right
	frustumPlanes[1].normal.x = VP[0][3] - VP[0][0];
	frustumPlanes[1].normal.y = VP[1][3] - VP[1][0];
	frustumPlanes[1].normal.z = VP[2][3] - VP[2][0];
	frustumPlanes[1].d = VP[3][3] - VP[3][0];

	// Bottom
	frustumPlanes[2].normal.x = VP[0][3] + VP[0][1];
	frustumPlanes[2].normal.y = VP[1][3] + VP[1][1];
	frustumPlanes[2].normal.z = VP[2][3] + VP[2][1];
	frustumPlanes[2].d = VP[3][3] + VP[3][1];

	// Top
	frustumPlanes[3].normal.x = VP[0][3] - VP[0][1];
	frustumPlanes[3].normal.y = VP[1][3] - VP[1][1];
	frustumPlanes[3].normal.z = VP[2][3] - VP[2][1];
	frustumPlanes[3].d = VP[3][3] - VP[3][1];



	// Normalize all planes
	for (auto& plane : frustumPlanes)
	{
		float length = glm::length(plane.normal);
		plane.normal /= length;
		plane.d /= length;
	}
}

void Camera::modifyAspectRatio(float newAR)
{
	mMatrices.proj = glm::perspective(glm::radians(45.0f), newAR, 0.1f, 1000.0f);
}

glm::vec3 Camera::getPosition() const
{
	return mPosition;
}

bool Camera::AABBIntersectsFrustum(const AABB* boundingBox)
{

	for (const auto& plane : frustumPlanes)
	{
		glm::vec3 pVertex = boundingBox->min;

		if (plane.normal.x >= 0)
			pVertex.x = boundingBox->max.x;
		if (plane.normal.y >= 0)
			pVertex.y = boundingBox->max.y;
		if (plane.normal.z >= 0)
			pVertex.z = boundingBox->max.z;

		auto distance = plane.distance(pVertex);
		if (distance < 0)
		{
			return false;
		}
			
	}
	return true;
}

Camera::Camera()
	:mOrientation(glm::vec3(1.0f, 0.0f, 0.0f)), mPosition(glm::vec3(3.0f, -2.0f, -2.0f)), mUp(glm::vec3(0.0f, 1.0f, 0.0f)), mFacing(mOrientation)
{

	mMatrices.proj = glm::perspective(glm::radians(mFOV), 1.0f, 0.1f, 1000.0f);
	mMatrices.view = glm::lookAt(mPosition, mPosition + mOrientation, mUp);
}
