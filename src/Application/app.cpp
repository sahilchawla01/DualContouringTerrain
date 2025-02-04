//Prevent glfw from importing opengl header
#define GLFW_INCLUDE_NONE


#include "app.h"

#include <array>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Helpers/Shader.h"
#include "../Helpers/Math/SDF.h"
#include "Actors/ACamera.h"
#include "Helpers/DualContouring.h"

App::App(int windowWidth, int windowHeight)
{
	this->window_width = windowWidth;
	this->window_height = windowHeight;
}

void App::init()
{

	//Init settings
	settings = Settings();

	std::cout << "App started!";

	//Setup the GLFW window context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	////Instantiate the GLFW window
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Terrain Editor", NULL, NULL);

	if(!window)
	{
		std::cout << "Failed to create the GLFW window!";
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, App::MouseCallback);
	glfwSetScrollCallback(window, App::ScrollCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Load GLAD before any OpenGL calls, (to find function pointers for OpenGL)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return;
	}

	//Tell window dimensions to OpenGL
	glViewport(0, 0, window_width, window_height);

	//Store pointer to app on window
	glfwSetWindowUserPointer(window, this);


	//Enable depth buffer and depth testing
	glEnable(GL_DEPTH_TEST);

	//Tell glfw to call the function when window size changes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Compile Shader
	Shader testShader("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag");

	
	// set up vertex data (and buffer(s)) and configure vertex attributes
   // ------------------------------------------------------------------
	float vertices[] = {
	  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	   0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	  -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	  -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	  -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	  -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	//float vertices[] = {
	// 0.5f,  0.5f, 0.0f,  // top right
	// 0.5f, -0.5f, 0.0f,  // bottom right
	//-0.5f, -0.5f, 0.0f,  // bottom left
	//-0.5f,  0.5f, 0.0f   // top left 
	//};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//Generate vertex array and vertex buffer for triangle render
	unsigned int VBO, VAO, EBO;
	//Generate Vertex Array Object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//Generate Element Buffer Object
	//glGenBuffers(1, &EBO);

	//Bind VAO
	glBindVertexArray(VAO);

	// 0. copy our vertices array in a buffer for OpenGL to use
	//Binds a buffer object to the current buffer type, only 1 can be set at one time
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Copy data to the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 1. Copy index array in an element buffer for OpenGL to use.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// 2. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Now we can unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Next unbind the VAO
	glBindVertexArray(0);

	CreateInitActors();

	const int voxelSize = 1;

	
	//Render frames
	while(!glfwWindowShouldClose(window))
	{
		// Set window background color and clear previous image
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Store time variables
		float currentTime = static_cast<float>(glfwGetTime());
		elapsedTimeSinceLaunch = currentTime;
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		//~~ Handle Input~~ 
		ProcessInput(window);

		//~~ Handle Rendering ~~
		testShader.use();

		//Create a 3D grid
		{
			const int gridWidth = 10;
			const int gridHeight = 10;
			const int gridDepth = 10;
; 

			std::vector<glm::vec3> grid;
			std::vector<float> modelVertices;
			std::vector<unsigned int> modelIndices; 
			grid.reserve(gridWidth * gridHeight * gridDepth);

			std::unordered_map<int, std::array<std::pair<bool, bool>, 3>> voxelEdgeIsoSurfaceMap;
			std::unordered_map<int, int> voxelVertexIndexMap; 

			glm::vec3 gridPosition(0.f, 0.f, 0.f);

			glm::mat4 gridModelMatrix(1.f);
			glm::translate(gridModelMatrix, gridPosition);

			glm::vec3 gridCenter((gridWidth * voxelSize) / 2, (gridHeight * voxelSize )/ 2, (gridDepth * voxelSize)/ 2);

			bool bIsDebugEnabled = false;

			glBindVertexArray(VAO);

			//Generate vertex positions
			for(int x = 0; x < gridWidth * voxelSize; x += voxelSize)
			{
				for(int y = 0; y < gridHeight * voxelSize; y += voxelSize)
				{
					for(int z = 0; z < gridDepth * voxelSize; z += voxelSize)
					{
						//Get relative position to grid position
						glm::vec3 relativePos(static_cast<float>(x) - gridCenter.x, static_cast<float>(y) - gridCenter.y, static_cast<float>(z) - gridCenter.z);

						relativePos += gridPosition;

						//Push position to grid
						grid.emplace_back(relativePos);

						//DEBUG: Spawn cube at grid position
						if(bIsDebugEnabled)
						{
							//std::cout << "Spawning cube at position: " << relativePos.x << ", " << relativePos.y << ", " << relativePos.z << "\n";
							//Create MVP
							glm::mat4 model = glm::mat4(1.0f);
							model = glm::translate(model, relativePos);
							//model = glm::scale(model, glm::vec3(0.75f, 0.75f, 0.75f));

							glm::mat4 view = m_currentCamera->GetViewMatrix();

							glm::mat4 projection = m_currentCamera->GetProjectionMatrix();

							glm::mat4 mvp = projection * view * model;

							testShader.setMat4("mvp", mvp);
							glDrawArrays(GL_TRIANGLES, 0, 36);
						}

						//Go over each corner

						{
							int cornersToConsider = 0;
							std::array<float, 8> cornerSDFValues;

							for(int i = 0; i < 8; ++i)
							{
								//Get current position 
								glm::vec3 currentCornerPos = (DualContouring::voxelCornerOffsets[i] * static_cast<float>(voxelSize)) + relativePos;

								//Calculate distance
								float distanceValue = SDF::GetSphereSDFValue(currentCornerPos, gridPosition, 4.f);
								cornerSDFValues[i] = distanceValue;

								//TODO: convert position from grid relative to SDF center relative
								//If within the surface, consider for triangulation
								if(distanceValue <= 0.f)
								{
									cornersToConsider |= 1 << i;
								}
							}

							//If the voxel is completely within the surface, or outside the volume, ignore it.
							if(cornersToConsider == 0 || cornersToConsider == 255)
								continue;

							std::vector<glm::vec3> intersectionPoints;
							std::vector<glm::vec3> intersectionNormals;

							//Array describes if an intersection occurs (first element), and if so, if intersection is + to -ve (second element) is true, else false. 
							std::array<std::pair<bool, bool>, 3> adjacentEdgesCrossingOver{{
									std::make_pair(false, false),
									std::make_pair(false, false),
									std::make_pair(false, false),
							}};

							for(int i = 0; i < 12; ++i)
							{
								const int cornerIndex1 = DualContouring::edgePairs[i].first;
								const int cornerIndex2 = DualContouring::edgePairs[i].second;

								const int m1 = (cornersToConsider >> cornerIndex1) & 1;
								const int m2 = (cornersToConsider >> cornerIndex2) & 1;

								//This means that the edge has no crossing over from one sign to the other, skip.
								if(m1 == m2)
								{
									continue;
								}

								//Crossing over has occured, check if edge is one of the 3 adjacent left most corner ones, and set the value.
								if (i == 0)
								{
									//Mark intersection occurred 
									adjacentEdgesCrossingOver[0].first = true;
									//If intersection is from + to -ve, mark as true, else false
									adjacentEdgesCrossingOver[0].second = (m1 < m2);
								}
								else if (i == 3)
								{
									//Mark intersection occurred 
									adjacentEdgesCrossingOver[1].first = true;
									//If intersection is from + to -ve, mark as true, else false
									adjacentEdgesCrossingOver[1].second = (m1 < m2);
								}
								else if(i == 8)
								{
									//Mark intersection occurred 
									adjacentEdgesCrossingOver[2].first = true;
									//If intersection is from + to -ve, mark as true, else false
									adjacentEdgesCrossingOver[2].second = (m1 < m2);
								}

								//Find position along the edge where surface crosses signs

								//TODO: convert position from grid relative to SDF center relative
								const glm::vec3 cornerPos1 = (DualContouring::voxelCornerOffsets[cornerIndex1] * static_cast<float>(voxelSize)) + relativePos;
								const glm::vec3 cornerPos2 = (DualContouring::voxelCornerOffsets[cornerIndex2] * static_cast<float>(voxelSize)) + relativePos;

								//Get current intersection point by using linear interpolation
								float interpolateFactor = abs(cornerSDFValues[cornerIndex1]) / (abs(cornerSDFValues[cornerIndex1]) + abs(cornerSDFValues[cornerIndex2]));
								glm::vec3 currIntersectionPoint = cornerPos1 + ((glm::normalize(cornerPos2 - cornerPos1)) * interpolateFactor);
								intersectionPoints.push_back(currIntersectionPoint);

								//Calculate normal using Finite Sum Difference
								intersectionNormals.push_back(DualContouring::CalculateSurfaceNormal(currIntersectionPoint, gridPosition, 4.f));
							}

							//Map voxel to adjacent edges vector
							voxelEdgeIsoSurfaceMap[GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)] = adjacentEdgesCrossingOver;

							glm::vec3 vertexPos(0.f);
							for(const glm::vec3& pos : intersectionPoints)
							{
								vertexPos += pos;
							}

							//Get centroid of intersection positions and normals
							vertexPos = vertexPos / static_cast<float>(intersectionPoints.size());


							//TODO: Perform QEF to find the vertex position, and then use normals, currently normals are unused

							//Map voxel to vertex array index position
							voxelVertexIndexMap[GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)] = modelVertices.size() / 3;

							//Store vertex position relative to grid space
							modelVertices.push_back(vertexPos.x);
							modelVertices.push_back(vertexPos.y);
							modelVertices.push_back(vertexPos.z);

							
						}
					}
				}
			}


			//Iterate through the cubes again, and make the edge connections
			for (int x = 0; x < gridWidth * voxelSize; x += voxelSize)
			{
				for (int y = 0; y < gridHeight * voxelSize; y += voxelSize)
				{
					for (int z = 0; z < gridDepth * voxelSize; z += voxelSize)
					{

						const std::array<std::pair<bool, bool>, 3> adjacentEdgesIntersection = voxelEdgeIsoSurfaceMap[GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)];

					/*	if(adjacentEdgesIntersection.empty())
						{
							std::cout << "App || Error!, Couldn't find unique index for the current voxel when getting adjacentEdgeIntersection from map\n";
							continue;
						}*/

						//Iterate over adjacent edges and make connections where possible
						for(int k = 0; k < 3; ++k)
						{
							//For an edge, check if all 4 neighboring voxels are valid
							if(adjacentEdgesIntersection[k].first == true && ((x - 1) > 0 && (y - 1) > 0 && (z - 1) > 0))
							{
								std::vector<unsigned int> vertexIndices;
								//Store vertex indices for neighboring voxels
								for(int i = 0; i < 4; ++i)
								{
									int curX = x + static_cast<int>(DualContouring::adjacentVoxelsOffsets[k][i].x);
									int curY = y + static_cast<int>(DualContouring::adjacentVoxelsOffsets[k][i].y);
									int curZ = z + static_cast<int>(DualContouring::adjacentVoxelsOffsets[k][i].z);

									//Use voxelVertexIndexMap to get the index of the vertex for a voxel when connecting edges
									vertexIndices.push_back(voxelVertexIndexMap[GetUniqueIndexForGrid(curX, curY, curZ, gridWidth, gridHeight)]);
								}

								//Join all 4 vertices in those voxels

								//If the transition is from + to -ve 
								if(adjacentEdgesIntersection[k].second == true)
								{
									//Triangle 1
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[3]);
									modelIndices.push_back(vertexIndices[2]);

									//Triangle 2
									modelIndices.push_back(vertexIndices[0]);
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[2]);

								} else //Reverse indices order otherwise (-ve to +ve transition)
								{
									//Triangle 1
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[2]);
									modelIndices.push_back(vertexIndices[3]);

									//Triangle 2
									modelIndices.push_back(vertexIndices[0]);
									modelIndices.push_back(vertexIndices[2]);
									modelIndices.push_back(vertexIndices[1]);
								}

							}
						}




					}
				}
			}

			//Create VAO, VBO, EBO for the model
			unsigned int sphere_VBO, sphere_VAO, sphere_EBO;
			//Generate Vertex Array Object
			glGenVertexArrays(1, &sphere_VAO);
			glGenBuffers(1, &sphere_VBO);
			//Generate Element Buffer Object
			glGenBuffers(1, &sphere_EBO);

			//Bind VAO
			glBindVertexArray(sphere_VAO);

			// 0. copy our vertices array in a buffer for OpenGL to use
			//Binds a buffer object to the current buffer type, only 1 can be set at one time
			glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO);
			//Copy data to the buffer
			glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(float), modelVertices.data(), GL_STATIC_DRAW);
			// 1. Copy index array in an element buffer for OpenGL to use.
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelIndices.size() * sizeof(unsigned int), modelIndices.data(), GL_STATIC_DRAW);
			// 2. then set the vertex attributes pointers
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			//Now we can unbind the buffer
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			

			//Draw the model	
			glm::mat4 view = m_currentCamera->GetViewMatrix();

			glm::mat4 projection = m_currentCamera->GetProjectionMatrix();

			glm::mat4 mvp = projection * view * gridModelMatrix;

			testShader.setMat4("mvp", mvp);
			glDrawElements(GL_TRIANGLES, static_cast<int>(modelVertices.size()), GL_UNSIGNED_INT, 0);

			//Next unbind the VAO
			glBindVertexArray(0);
		}




		//~~ Handle Events Polling ~~ 
		//Swap the back buffer with the front buffer (to show new image)
		glfwSwapBuffers(window);
		//Check if any events have been triggered (keyboard inputs, mouse movements, etc)
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	//Terminate the window
	glfwTerminate();
}

//Creates all required actors for the scene
void App::CreateInitActors()
{

	//Create camera
	m_currentCamera = std::make_unique<ACamera>(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 1.f, 0.f), settings.mouse_yaw, settings.mouse_pitch, static_cast<float>(this->window_width) / static_cast<float>(this->window_height));

}

int App::GetUniqueIndexForGrid(const int x, const int y, const int z, const int gridWidth, const int gridHeight)
{
	return x + gridWidth * (y + gridHeight * z);
}

void App::ProcessInput(GLFWwindow* window)
{
	//-- Handle WASD movement for camera  --
	if (m_currentCamera)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::FORWARD, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition + (cameraSpeed * deltaTime) * cameraFront;
			SetCameraPosition(newCamPos);*/
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::BACKWARD, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition - (cameraSpeed * deltaTime) * cameraFront;
			SetCameraPosition(newCamPos);*/
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::LEFT, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition - glm::normalize(glm::cross(cameraFront, cameraUp)) * (cameraSpeed * deltaTime);
			SetCameraPosition(newCamPos);*/
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::RIGHT, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition + glm::normalize(glm::cross(cameraFront, cameraUp)) * (cameraSpeed * deltaTime);
			SetCameraPosition(newCamPos);*/
		}
	} else
	{
		std::cout << "\nError: Camera not present.";
	}
}

/*
 * Resize the viewport of the OpenGL window to new width and height
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void App::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	appPtr->m_currentCamera->ProcessMouseInput(static_cast<float>(xposIn), static_cast<float>(yposIn));
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void App::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	appPtr->m_currentCamera->ProcessScrollInput(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

App::~App()
{
}