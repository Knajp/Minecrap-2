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
	//KEYBOARD INPUT
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		mVelocity += mFacing * mSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		mVelocity += mFacing * -mSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		mVelocity += glm::normalize(glm::cross(mUp, mFacing)) * mSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		mVelocity += glm::normalize(glm::cross(mUp, mFacing)) * -mSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		mVelocity += mUp * -mSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		mVelocity += mUp * mSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Damp and Apply velocity to player
	mVelocity *= 55.0f * deltaTime;
	mPosition += mVelocity;

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

}

void Camera::modifyAspectRatio(float newAR)
{
	mMatrices.proj = glm::perspective(glm::radians(45.0f), newAR, 0.1f, 1000.0f);
}

glm::vec3 Camera::getPosition() const
{
	return mPosition;
}

Camera::Camera()
	:mOrientation(glm::vec3(1.0f, 0.0f, 0.0f)), mPosition(glm::vec3(3.0f, -2.0f, -2.0f)), mUp(glm::vec3(0.0f, 1.0f, 0.0f)), mFacing(mOrientation)
{

	mMatrices.proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);
	mMatrices.view = glm::lookAt(mPosition, mPosition + mOrientation, mUp);
}
