#include <Actors/ACamera.h>
#include <iostream>

ACamera::ACamera(glm::vec3 spawnPosition = glm::vec3(0.f, 0.f, 0.f), glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f), float yaw = -90.f, float pitch = 0.f, float aspectRatio = 0.5f)
{
	this->currentCameraPosition = spawnPosition;
	this->startCameraPosition = spawnPosition;
	this->worldUp = worldUp;
	this->cameraYaw = yaw;
	this->cameraPitch = pitch;
	this->cameraAspectRatio = aspectRatio;

	//Set camera position
	SetCameraPosition(startCameraPosition);

	//Set the camera's rotation in degrees
	SetCameraRotation(glm::vec3(pitch, yaw, 0.f));

	//Update view matrix according to new camera position
	UpdateViewMatrix();
	//Update projection matrix according to camera fov
	UpdateProjectionMatrix();
}

void ACamera::UpdateViewMatrix()
{
	//The new view matrix is based on the 3 coordinate system with the camera at the center, using the cameraUp direction, camera front direction and currentCamera position
	viewMatrix = CalculateLookAtMatrix(currentCameraPosition, currentCameraPosition + cameraFront, worldUp);

	//std::cout << "\nDisplay: (ACamera) Camera view matrix updated!\n";
}

void ACamera::UpdateCameraDirectionVectors()
{
	//Calculate direction vector that camera now looks at 
	glm::vec3 direction(0.f, 0.f, 0.f);
	direction.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
	direction.y = sin(glm::radians(cameraPitch));
	direction.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
	direction = glm::normalize(direction);

	//Set camera's new front directiont
	cameraFront = direction;

	//Also calculate and store cameraUp and cameraRight
	cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

	//Set camera rotation 
	//SetCameraRotation(glm::vec3(cameraPitch, cameraYaw, 0.f));

	//Update view matrix according to new direction
	UpdateViewMatrix();
}


void ACamera::UpdateProjectionMatrix()
{
	std::cout << "\nUpdating Projection matrix.. FOV: " << GetCameraFov() << "\t Aspect Ratio: " << cameraAspectRatio;
	//Updates the projection matrix according to new fov of camera
	projectionMatrix = glm::perspective(glm::radians(GetCameraFov()), cameraAspectRatio, 0.1f, 100.f);
}

glm::mat4 ACamera::CalculateLookAtMatrix(glm::vec3 cameraPosition, glm::vec3 targetPosition, glm::vec3 worldUp)
{
	//Calculate cameraDirection
	glm::vec3 zAxis = glm::normalize(cameraPosition - targetPosition);
	//Get positive right axis from worldUp
	glm::vec3 xAxis = glm::normalize(glm::cross(glm::normalize(worldUp), zAxis));
	//Calculate camera up vector
	glm::vec3 yAxis = glm::cross(zAxis, xAxis);

	//Create translation and rotation matrix
	//In glm, we access matrix by column-major layout
	glm::mat4 translation = glm::mat4(1.f);
	translation[3][0] = -cameraPosition.x; //Fourth column
	translation[3][1] = -cameraPosition.y;
	translation[3][2] = -cameraPosition.z;

	//Create rotation matrix
	glm::mat4 rotation = glm::mat4(1.f);
	rotation[0][0] = xAxis.x; //First row is the right vector
	rotation[1][0] = xAxis.y;
	rotation[2][0] = xAxis.z;

	rotation[0][1] = yAxis.x; //Second row is up vector
	rotation[1][1] = yAxis.y;
	rotation[2][1] = yAxis.z;

	rotation[0][2] = zAxis.x; //Third row is the direction vector
	rotation[1][2] = zAxis.y;
	rotation[2][2] = zAxis.z;

	//Return lookAt matrix as a combination of the translation and rotation matrix

	return rotation * translation;
}

void ACamera::SetCameraPosition(glm::vec3 newCameraPosition, bool bAutoUpdateViewMatrix)
{
	currentCameraPosition = newCameraPosition;

	//Simply update view matrix according to new position
	if (bAutoUpdateViewMatrix) UpdateViewMatrix();
}

void ACamera::AddCameraPosition(glm::vec3 positionToAdd, bool bAutoUpdateViewMatrix)
{
	currentCameraPosition += positionToAdd;

	//Simply update view matrix according to new position
	if (bAutoUpdateViewMatrix) UpdateViewMatrix();
}

void ACamera::SetCameraRotation(glm::vec3 newCameraRotation, bool bAutoUpdateViewMatrix)
{
	cameraPitch = newCameraRotation.x;
	cameraYaw = newCameraRotation.y;

	cameraRotation = newCameraRotation;

	//Update camera vectors based on new rotation
	UpdateCameraDirectionVectors();

	//Update the view matrix according to new direction vectors
	if (bAutoUpdateViewMatrix) UpdateViewMatrix();
}

void ACamera::AddToCameraRotation(glm::vec3 rotationToAdd, bool bAutoUpdateViewMatrix)
{
	cameraPitch += rotationToAdd.x;
	cameraYaw += rotationToAdd.y;

	cameraRotation += rotationToAdd;

	//Update camera vectors based on new rotation
	UpdateCameraDirectionVectors();

	//Update the view matrix according to new direction vectors
	if (bAutoUpdateViewMatrix) UpdateViewMatrix();
}

void ACamera::EnableFastCamera()
{
	//Increase camera speed 
	cameraSpeed *= fastCameraSpeedMultiplier;
}

void ACamera::DisableFastCamera()
{
	//Decrease camera speed back to default
	cameraSpeed /= fastCameraSpeedMultiplier;
}

void ACamera::ProcessMouseInput(float xPos, float yPos)
{
	//If mouse hasn't moved since start of the window
	if (!bHasCameraMoved)
	{
		lastX = xPos;
		lastY = yPos;
		bHasCameraMoved = true;
	}
	//Calculate offset since previous
	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos;
	//Store last x and y values
	lastX = xPos;
	lastY = yPos;

	//Factor in camera sensitivity
	xOffset *= cameraSensitivity;
	yOffset *= cameraSensitivity;

	//Add offset values to camera's rotation
	cameraPitch += yOffset;
	cameraYaw += xOffset;

	//Constraint looking up and down beyond 89 degrees
	if (cameraPitch > 89.f) cameraPitch = 89.f;
	else if (cameraPitch < -89.f) cameraPitch = -89.f;

	//Store new rotation, which also updates view matrix
	SetCameraRotation(glm::vec3(cameraPitch, cameraYaw, 0.f));

}

void ACamera::ProcessKeyboardInput(ECameraMoveDirection direction, float deltaTime)
{
	//-- Handle WASD movement for camera  --
	if (direction == ECameraMoveDirection::FORWARD)
	{
		glm::vec3 newCamPos = currentCameraPosition + (cameraSpeed * deltaTime) * cameraFront;
		SetCameraPosition(newCamPos, false);
		//std::cout << "Moving Camera Forward \n";
	}
	if (direction == ECameraMoveDirection::BACKWARD)
	{
		glm::vec3 newCamPos = currentCameraPosition - (cameraSpeed * deltaTime) * cameraFront;
		SetCameraPosition(newCamPos, false);
		//std::cout << "Moving Camera Backward \n";
	}
	if (direction == ECameraMoveDirection::LEFT)
	{
		glm::vec3 newCamPos = currentCameraPosition - (cameraSpeed * deltaTime) * cameraRight;
		SetCameraPosition(newCamPos, false);
		//std::cout << "Moving Camera Left \n";
	}
	if (direction == ECameraMoveDirection::RIGHT)
	{
		glm::vec3 newCamPos = currentCameraPosition + (cameraSpeed * deltaTime) * cameraRight;
		SetCameraPosition(newCamPos, false);
		//std::cout << "Moving Camera Right\n";
	}

	//Finally, update view matrix after all position operations
	UpdateViewMatrix();
}

void ACamera::ProcessScrollInput(float xOffset, float yOffset)
{
	//Positive implies zoom in, negative implies zoom out
	float zoomDirectionScale = 1.f;
	//If zooming out, invert zoom direction
	if (yOffset < 0.0) zoomDirectionScale = -1.f;

	float currCameraFov = GetCameraFov();

	//Finally, set camera fov according to step 
	SetCameraFov(currCameraFov - (cameraFovStep * zoomDirectionScale));

	//std::cout << "\nDisplay: (ACamera) Camera Fov set to : " << GetCameraFov();
}

void ACamera::SetCameraFov(float newFov)
{
	//Constrain fov to be between max and min camera fov
	if (newFov > maxCameraFov) newFov = maxCameraFov;
	else if (newFov < minCameraFov) newFov = minCameraFov;

	//Set new camera fov
	cameraFov = newFov;

	//std::cout << "\nNew camera fov: " << newFov;

	//With new fov, update projection matrix
	UpdateProjectionMatrix();
}

glm::vec3 ACamera::GetCameraWorldPosition()
{
	//std::cout << "Current camera position:\t" << currentCameraPosition.x << "||	" << currentCameraPosition.y << "||	" << currentCameraPosition.z<<"\n";
	return currentCameraPosition;
}

glm::vec3 ACamera::GetCameraForwardDirVector()
{
	//std::cout << "\nCurrent camera front direction:\t" << cameraFront.x << "||" << cameraFront.y << "||	" << cameraFront.z<<"\n";
	return cameraFront;
}

glm::mat4 ACamera::GetViewMatrix()
{
	//Update the view matrix if any changes are required, and return it
	UpdateViewMatrix();

	return viewMatrix;
}

glm::mat4 ACamera::GetProjectionMatrix()
{
	//Update the projection matrix if any changes required and return it
	UpdateProjectionMatrix();

	return projectionMatrix;
}
