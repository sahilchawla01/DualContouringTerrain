#pragma once
#include "AActor.h"

enum class ECameraMoveDirection
{
	FORWARD,
	LEFT,
	RIGHT,
	BACKWARD,
};

class ACamera : public AActor
{
public:

	//Constructor sets start position, rotation and updates view matrix
	ACamera(glm::vec3 spawnPosition, glm::vec3 worldUp, float yaw, float pitch, float aspectRatio);

	//-- Camera Variables -- 
	//The camera position in world space
	glm::vec3 currentCameraPosition = glm::vec3(0.f);
	//The world space position camera should spawn at
	glm::vec3 startCameraPosition = glm::vec3(0.f, 0.f, 10.f);
	//The unit vector defining the direction the camera is looking at, by default, in the -ve z axis
	glm::vec3 cameraFront = glm::vec3(0.f, 0.f, -1.f);
	//The unit vector defining the right direction of camera
	glm::vec3 cameraRight = glm::vec3(1.f, 0.f, 0.f);
	//The unit vector defining the up direction for the camera
	glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);
	//The unit vector defining the up direction for the world
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
	//The camera rotation is represented in the order XYZ (Yaw, Pitch, Roll)
	glm::vec3 cameraRotation = glm::vec3(0.f, 0.f, 0.f);

	//-- Euler Angles --
	float cameraPitch = 0.f;
	float cameraYaw = 0.f;

	//-- FOV Variables --
	float cameraFov = 45.f;
	float originalCameraFov = 45.f;
	float cameraFovStep = 10.f;
	float minCameraFov = 20.f;
	float maxCameraFov = 120.f;
	float cameraAspectRatio = 0.5f;

	//-- Camera user-settable settings -- 
	float cameraSpeed = 5.f;
	float fastCameraSpeedMultiplier = 2.5f;
	float cameraSensitivity = 0.1f;

	// -- Misc Variables --
	bool bHasCameraMoved = false;
	//The last position of the mouse on the X axis
	float lastX = 400.f;
	//The last position of the mouse on the y axis
	float lastY = 400.f;

private:
	//The view matrix associated with the camera
	glm::mat4 viewMatrix = glm::mat4(1.f);
	//The projection matrix associated with the camera
	glm::mat4 projectionMatrix = glm::mat4(1.f);

public:

	//Updates the view matrix according to the 3 vectors including camera position, camera front and world up
	void UpdateViewMatrix();
	//Updates all the direction vectors for the camera (Front, Right, Up) according to the current camera rotation
	void UpdateCameraDirectionVectors();
	//Updates the projection matrix according to the camera FOV
	void UpdateProjectionMatrix();

	//--	Translate and Rotate operations		--
	//(Updates view matrix automatically) Sets camera position according to a provided world position
	void SetCameraPosition(glm::vec3 newCameraPosition, bool bAutoUpdateViewMatrix = true);
	//(Updates view matrix automatically) Adds camera position
	void AddCameraPosition(glm::vec3 positionToAdd, bool bAutoUpdateViewMatrix = true);
	//(Updates view matrix and camera direction vectors automatically) The camera rotation is represented in the order XYZ(Pitch, Yaw, Roll)
	void SetCameraRotation(glm::vec3 newCameraRotation, bool bAutoUpdateViewMatrix = true);
	//(Updates view matrix and camera direction vectors automatically) The camera rotation is represented in the order XYZ(Pitch, Yaw, Roll)
	void AddToCameraRotation(glm::vec3 rotationToAdd, bool bAutoUpdateViewMatrix = true);
	//Returns the camera's current rotation
	glm::vec3 GetCameraRotation() { return cameraRotation; };
	//Increases speed of camera movement
	void EnableFastCamera();
	//Returns speed of camera movement to default
	void DisableFastCamera();

public:
	/* -- Input handler functions --*/
	void ProcessMouseInput(float xPos, float yPos);
	void ProcessKeyboardInput(ECameraMoveDirection direction, float deltaTime);
	void ProcessScrollInput(float xOffset, float yOffset);

public:
	//Sets the fov of the camera, which updates the projection matrix
	void SetCameraFov(float newFov);
	//Get the camera's fov in float 
	float GetCameraFov() { return cameraFov; };
	//Simply return camera's world position
	glm::vec3 GetCameraWorldPosition();
	glm::vec3 GetCameraForwardDirVector();


	//Get matrices
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();

private:
	//Calculates look at matrix according to given arguments
	glm::mat4 CalculateLookAtMatrix(glm::vec3 cameraPosition, glm::vec3 targetPosition, glm::vec3 worldUp);
};

