#include "./basicApp.h"

void BasicApp::cleanup(GLFWwindow * window)
{

};

void BasicApp::processInput(GLFWwindow * window)
{
	camera.cameraSpeed = 10.f * lastFrameTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.move(FORWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.move(BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.move(LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.move(RIGHT);
//	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
//		steps += 0.1;
//	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
//		steps -= 0.1;
//	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
//		verticalSteps += 0.1;
//	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
//		verticalSteps -= 0.1;
};
