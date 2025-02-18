//Prevent glfw from importing opengl header
#define GLFW_INCLUDE_NONE


#include "app.h"

#include <array>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Helpers/Shader.h"
#include "../Helpers/Math/SDF.h"
#include "Actors/ACamera.h"
#include "Helpers/DualContouring.h"
#include <algorithm>


//IMGUI INCLUDES

#include "Helpers/imgui/imgui.h"
#include "Helpers/imgui/imgui_impl_glfw.h"
#include "Helpers/imgui/imgui_impl_opengl3.h"

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
	glfwSetKeyCallback(window, App::KeyCallback);
	glfwSetInputMode(window, GLFW_CURSOR, settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

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
	Shader litShader("../../../src/Shaders/simple-lit.vert", "../../../src/Shaders/simple-lit.frag");
	Shader unlitShader("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag");

	
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

	//Generate vertex array and vertex buffer for triangle render
	unsigned int VBO, VAO;
	//Generate Vertex Array Object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	//Bind VAO
	glBindVertexArray(VAO);

	// 0. copy our vertices array in a buffer for OpenGL to use
	//Binds a buffer object to the current buffer type, only 1 can be set at one time
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Copy data to the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 2. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Now we can unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Next unbind the VAO
	glBindVertexArray(0);

	CreateInitActors();

	const int voxelSize = 1;


	//Setup IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	bool bDebugEdges = true;
	
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
		PollSettings(window);

		// Start the Dear ImGui frame
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Dual Contouring Settings!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Checkbox("Enable Cursor", &settings.bIsCursorEnabled);      // Edit bools storing our window open/close state
			if (ImGui::CollapsingHeader("Debug"))
			{
				ImGui::Checkbox("Enable Debug", &settings.bIsDebugEnabled);
				ImGui::Checkbox("Enable Voxel Debug", &settings.bIsVoxelDebugEnabled);
				ImGui::Checkbox("Enable SDF Mesh Rendering", &settings.bViewMesh);
			}
			//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
			
		}

		//~~ Handle Rendering ~~
		

		//Create a 3D grid
		{
			const int gridWidth = 15;
			const int gridHeight = 15;
			const int gridDepth = 15;
; 

			std::vector<glm::vec3> grid;
			std::vector<float> modelVertices;
			std::vector<float> modelNormals;
			std::vector<unsigned int> modelIndices; 
			grid.reserve(gridWidth * gridHeight * gridDepth);

			std::unordered_map<int, std::array<std::pair<bool, bool>, 3>> voxelEdgeIsoSurfaceMap;
			std::unordered_map<int, std::array<std::pair<float, float>, 3>> voxelEdgeSDFMap;

			std::unordered_map<int, int> voxelVertexIndexMap; 

			glm::vec3 gridPosition(0.f, 0.f, 0.f);

			glm::mat4 gridModelMatrix(1.f);
			glm::translate(gridModelMatrix, gridPosition);
			 
			glm::vec3 gridCenter((gridWidth * voxelSize) / 2, (gridHeight * voxelSize )/ 2, (gridDepth * voxelSize)/ 2);

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
							std::array<std::pair<float, float>, 3> debugAdjacentEdgeSDFS
							{
								{
									std::make_pair(-10000.f, -10000.f),
									std::make_pair(-10000.f, -10000.f),
									std::make_pair(-10000.f, -10000),
								} };

							if (bDebugEdges) std::cout << "\nVoxel Being Considered: " << x << "," << y << "," << z<<"\n";

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

								if (bDebugEdges) std::cout << "Corner Indices:" << cornerIndex1 << "," << cornerIndex2<<std::endl;

								//glm::vec3 currentCornerPos1 = (DualContouring::voxelCornerOffsets[cornerIndex1] * static_cast<float>(voxelSize)) + relativePos;
								//const int cornerIndex1SDF = ;
								//glm::vec3 currentCornerPos2 = (DualContouring::voxelCornerOffsets[cornerIndex2] * static_cast<float>(voxelSize)) + relativePos;
								//const int cornerIndex2SDF = ;
								//Crossing over has occured, check if edge is one of the 3 adjacent left most corner ones, and set the value.
								if (i == 0)
								{
									//Mark intersection occurred 
									adjacentEdgesCrossingOver[0].first = true;
									//If intersection is from + to -ve, mark as true, else false
									adjacentEdgesCrossingOver[0].second = (m1 < m2);

									//Corner 1 index SDF Value
									debugAdjacentEdgeSDFS[0].first = (cornerSDFValues[cornerIndex1]);
									debugAdjacentEdgeSDFS[0].second = (cornerSDFValues[cornerIndex2]);
								}
								else if (i == 3)
								{
									//Mark intersection occurred 
									adjacentEdgesCrossingOver[1].first = true;
									//If intersection is from + to -ve, mark as true, else false
									adjacentEdgesCrossingOver[1].second = (m1 < m2);

									debugAdjacentEdgeSDFS[1].first = (cornerSDFValues[cornerIndex1]);
									debugAdjacentEdgeSDFS[1].second = (cornerSDFValues[cornerIndex2]);
								}
								else if (i == 8)
								{
									//Mark intersection occurred 
									adjacentEdgesCrossingOver[2].first = true;
									//If intersection is from + to -ve, mark as true, else false
									adjacentEdgesCrossingOver[2].second = (m1 < m2);

									debugAdjacentEdgeSDFS[2].first = (cornerSDFValues[cornerIndex1]);
									debugAdjacentEdgeSDFS[2].second = (cornerSDFValues[cornerIndex2]);
								}

								//Find position along the edge where surface crosses signs

								//TODO: convert position from grid relative to SDF center relative
								const glm::vec3 cornerPos1 = (DualContouring::voxelCornerOffsets[cornerIndex1] * static_cast<float>(voxelSize)) + relativePos;
								const glm::vec3 cornerPos2 = (DualContouring::voxelCornerOffsets[cornerIndex2] * static_cast<float>(voxelSize)) + relativePos;

								//Get current intersection point by using linear interpolation
								float interpolateFactor = abs(cornerSDFValues[cornerIndex1]) / (abs(cornerSDFValues[cornerIndex1]) + abs(cornerSDFValues[cornerIndex2]));
								glm::vec3 currIntersectionPoint = cornerPos1 + ((cornerPos2 - cornerPos1) * interpolateFactor);
								intersectionPoints.push_back(currIntersectionPoint);

								//TODO: SAnity check whether point is within voxel

								//Calculate normal using Finite Sum Difference
								intersectionNormals.push_back(DualContouring::CalculateSurfaceNormal(currIntersectionPoint, gridPosition, 4.f));
							}

							if (bDebugEdges) std::cout << std::endl;

							//Map voxel to adjacent edges vector
							voxelEdgeIsoSurfaceMap[GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)] = adjacentEdgesCrossingOver;
							voxelEdgeSDFMap[GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)] = debugAdjacentEdgeSDFS;

							glm::vec3 vertexPos(0.f);
							for(const glm::vec3& pos : intersectionPoints)
							{
								vertexPos += pos;
							}

							//Get centroid of intersection positions 
							vertexPos = vertexPos / static_cast<float>(intersectionPoints.size());

							//Calculate centroid of intersection normals
							glm::vec3 vertexNormal(0.f);
							for (const glm::vec3& normal : intersectionNormals)
							{
								vertexNormal += normal;
							}

							vertexNormal = glm::normalize(vertexNormal);

							//TODO: Perform QEF to find the vertex position, and then use normals, currently normals are unused

							//SANITY CHECK: CHECK IF CURRENT UNIQUE ID HAS ALREADY BEEN SET FOR VOXEL-VERTEX MAP
							if (voxelVertexIndexMap.find(GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)) != voxelVertexIndexMap.end())
							{
								std::cout << "ERROR: THIS UNIQUE ID HAS ALREADY BEEN SET\n";
							}
							//Map voxel to vertex array index position
							voxelVertexIndexMap[GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)] = static_cast<int>(modelVertices.size());

							//std::cout << "New Vertex: " << modelVertices.size() / 3<<", ";

							//Store vertex position relative to grid space
							modelVertices.push_back(vertexPos.x);
							modelVertices.push_back(vertexPos.y);
							modelVertices.push_back(vertexPos.z);

							//std::cout << "Vertex added, model vertices size" << modelVertices.size();

							//Store model normals 
							modelNormals.push_back(vertexNormal.x);
							modelNormals.push_back(vertexNormal.y);
							modelNormals.push_back(vertexNormal.z);
							
						}
					}
				}
			}


			//DEBUG: Spawn cube at vertex positions
			if (settings.bIsDebugEnabled)
			{

				unlitShader.use();

				//Bind the VAO for debugging cube
				glBindVertexArray(VAO);

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal shading
				//Draw vertex position
				for (int x = 0; x < gridWidth * voxelSize; x += voxelSize)
				{
					for (int y = 0; y < gridHeight * voxelSize; y += voxelSize)
					{
						for (int z = 0; z < gridDepth * voxelSize; z += voxelSize)
						{
							//If a vertex isn't there in a voxel, skip. 
							if (voxelVertexIndexMap.find(GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)) == voxelVertexIndexMap.end())
							{
								continue;
							}

							//Draw vertex position
							int modelVerticesIndex = voxelVertexIndexMap[GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)];
							glm::vec3 vertexPos(modelVertices[modelVerticesIndex], modelVertices[modelVerticesIndex + 1], modelVertices[modelVerticesIndex + 2]);

							//Create MVP
							glm::mat4 model = glm::mat4(1.0f);
							model = glm::translate(model, vertexPos);
							model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

							glm::mat4 view = m_currentCamera->GetViewMatrix();

							glm::mat4 projection = m_currentCamera->GetProjectionMatrix();

							glm::mat4 mvp = projection * view * model;
							glm::vec3 debugCubeColor(0.027f, 0.843f, 1.0f);
							unlitShader.setMat4("mvp", mvp);
							unlitShader.setVec3("color", debugCubeColor);

							glDrawArrays(GL_TRIANGLES, 0, 36);

						}
					}
				}

				//Draw voxels
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				if (settings.bIsVoxelDebugEnabled)
				{
					for (int x = 0; x < gridWidth * voxelSize; x += voxelSize)
					{
						for (int y = 0; y < gridHeight * voxelSize; y += voxelSize)
						{
							for (int z = 0; z < gridDepth * voxelSize; z += voxelSize)
							{

								
									glm::vec3 relativePos(static_cast<float>(x) - gridCenter.x, static_cast<float>(y) - gridCenter.y, static_cast<float>(z) - gridCenter.z);

									relativePos += gridPosition;

									glm::mat4 model = glm::mat4(1.0f);
									model = glm::translate(model, relativePos);
									model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
									glm::mat4 view = m_currentCamera->GetViewMatrix();
									glm::mat4 projection = m_currentCamera->GetProjectionMatrix();
									glm::mat4 mvp = projection * view * model;
									glm::vec3 voxelCubeColor(1.f, 0.984f, 0.f);
									unlitShader.setMat4("mvp", mvp);
									unlitShader.setVec3("color", voxelCubeColor);

									glDrawArrays(GL_TRIANGLES, 0, 36);

								}

							}
						}
				}

				//Unbind the VAO
				glBindVertexArray(0);

			}


			//Iterate through the cubes again, and make the edge connections
			for (int x = 0; x < gridWidth * voxelSize; x += voxelSize)
			{
				for (int y = 0; y < gridHeight * voxelSize; y += voxelSize)
				{
					for (int z = 0; z < gridDepth * voxelSize; z += voxelSize)
					{
						const std::array<std::pair<bool, bool>, 3> adjacentEdgesIntersection = voxelEdgeIsoSurfaceMap[
							GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)];

						const std::array<std::pair<float, float>, 3> edgesSDFPairs = voxelEdgeSDFMap[
							GetUniqueIndexForGrid(x, y, z, gridWidth, gridHeight)];

						bool bVoxelDebug = true;
						//Iterate over adjacent edges and make connections where possible
						for(int axis = 0; axis < 3; ++axis)
						{
							//For an edge, check if an intersection exists and all 4 neighboring voxels are valid
							if(adjacentEdgesIntersection[axis].first == true && ((x - 1) > 0 && (y - 1) > 0 && (z - 1) > 0))
							{
								if (bDebugEdges && bVoxelDebug) std::cout << "\nVoxel Being Considered: "<<x<<"," << y << ","<<z;
								if (bDebugEdges)
								{
									if (axis == 0)
										std::cout << "\nEdge 0-1 Being Considered";

									if (axis == 1)
										std::cout << "\nEdge 0-3 Being Considered";

									if (axis == 2)
										std::cout << "\nEdge 0-4 (Vertical) Being Considered";
								}
								bVoxelDebug = false;

								std::vector<unsigned int> vertexIndices;
								std::vector<std::vector<int>> debugIndices;

								//Store vertex indices for neighboring voxels
								for(int i = 0; i < 4; ++i)
								{
									int curX = x + static_cast<int>(DualContouring::adjacentVoxelsOffsets[axis][i].x);
									int curY = y + static_cast<int>(DualContouring::adjacentVoxelsOffsets[axis][i].y);
									int curZ = z + static_cast<int>(DualContouring::adjacentVoxelsOffsets[axis][i].z);

									auto it = voxelVertexIndexMap.find(GetUniqueIndexForGrid(curX, curY, curZ, gridWidth, gridHeight));

									debugIndices.push_back({ curX, curY, curZ });

									if (it == voxelVertexIndexMap.end())
										continue;  // If voxel is missing, just skip it


									// Store the found vertex index
									vertexIndices.push_back(it->second / 3);
								}

								if (vertexIndices.size() == 3)
								{
									//std::cout << "THERE WERE 3 VALID NEIGHBORS!";
									if (bDebugEdges)
									{
										std::cout << "\nEdge Information: ";
										for (std::vector<int> voxelId : debugIndices)
										{
											std::cout << "Voxel Id:" << voxelId[0] << "," << voxelId[1] << "," << voxelId[2] << "\n";

											std::cout<<"Corners SDFs: "<<edgesSDFPairs[axis].first<<","<<edgesSDFPairs[axis].second<<"\n";
										}
									}


								}

								// If we have fewer than 4 valid neighbors, we cannot form a face
								if (vertexIndices.size() <= 3)
								{
									continue;
								}
								//Join all 4 vertices in those voxels
								if (vertexIndices.size() == 4)
								{
									//If the transition is from + to -ve 
									if(adjacentEdgesIntersection[axis].second == true)
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
			}

			litShader.use();

			//--Set directional light values--
			litShader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
			litShader.setVec3("dirLight.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
			litShader.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
			litShader.setVec3("dirLight.specular", glm::vec3(0.6f, 0.6f, 0.6f));

			////Set material values
			litShader.setFloat("mat.shine", 64.0f);
			litShader.setVec3("mat.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
			litShader.setVec3("mat.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
			litShader.setVec3("mat.specular", glm::vec3(0.5f, 0.5f, 0.5f));

			//unlitShader.use();

			//Create VAO, VBO, EBO for the model
			unsigned int sphere_Vertices_VBO, sphere_Normals_VBO, sphere_VAO, sphere_EBO;
			//Generate Vertex Array Object
			glGenVertexArrays(1, &sphere_VAO);
			glGenBuffers(1, &sphere_Vertices_VBO);
			glGenBuffers(1, &sphere_Normals_VBO);
			//Generate Element Buffer Object
			glGenBuffers(1, &sphere_EBO);

			//Bind VAO
			glBindVertexArray(sphere_VAO);

			// 0. copy our vertices array in a buffer for OpenGL to use
			//Binds a buffer object to the current buffer type, only 1 can be set at one time
			glBindBuffer(GL_ARRAY_BUFFER, sphere_Vertices_VBO);
			//Copy data to the buffer
			glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(float), modelVertices.data(), GL_STATIC_DRAW);
			// 1. Copy index array in an element buffer for OpenGL to use.
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelIndices.size() * sizeof(unsigned int), modelIndices.data(), GL_STATIC_DRAW);
			// 2. then set the vertex attributes pointers
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			//Copy normal array in a buffer
			glBindBuffer(GL_ARRAY_BUFFER, sphere_Normals_VBO);
			//Copy data to the buffer
			glBufferData(GL_ARRAY_BUFFER, modelNormals.size() * sizeof(float), modelNormals.data(), GL_STATIC_DRAW);
			// 2. then set the vertex attributes pointers
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);

			

			//Draw the model
			if (settings.bViewMesh)
			{
				

				glm::mat4 view = m_currentCamera->GetViewMatrix();

				glm::mat4 projection = m_currentCamera->GetProjectionMatrix();

				glm::mat4 modelViewMatrix = view * gridModelMatrix;
				glm::mat4 mvp = projection * modelViewMatrix;

				litShader.setMat4("modelViewMatrix", modelViewMatrix);
				litShader.setVec3("objectColor", glm::vec3(1.0));
				litShader.setMat4("mvp", mvp);
				//unlitShader.setMat4("mvp", mvp);
				//unlitShader.setVec3("color", glm::vec3(0.8f));

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal shading

				glDrawElements(GL_TRIANGLES, static_cast<int>(modelIndices.size()), GL_UNSIGNED_INT, 0);

			}

			//Now we can unbind the buffer
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			//Next unbind the VAO
			glBindVertexArray(0);
		}


		//Render the UI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//~~ Handle Events Polling ~~ 
		//Swap the back buffer with the front buffer (to show new image)
		glfwSwapBuffers(window);
		//Check if any events have been triggered (keyboard inputs, mouse movements, etc)
		glfwPollEvents();

		bDebugEdges = false;

	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);


	//Clean up code
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	//Terminate the window
	glfwTerminate();
}

void App::PollSettings(GLFWwindow* window) const
{
	glfwSetInputMode(window, GLFW_CURSOR, settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

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

	////Handle cursor display
	//if (glfwGetKey(window, GLFW_KEY_C) == GLFW_REPEAT)
	//{
	//	//Flip flag
	//	settings.bIsCursorEnabled = !settings.bIsCursorEnabled;
	//	glfwSetInputMode(window, GLFW_CURSOR, settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	//}
}

std::vector<unsigned int> App::getOrderedVertexIndices(int x, int y, int z, int axis, int gridWidth, int gridHeight,
	const std::unordered_map<int, int>& voxelVertexIndexMap, const std::vector<float>& modelVertices)
{
//	std::vector<unsigned int> rawIndices;
//	std::vector<glm::vec3> positions;
//
//	// Gather the four raw vertex indices using the adjacent offsets for this axis.
//	// DualContouring::adjacentVoxelsOffsets[axis] is a vector of 4 glm::vec3 offsets.
//	for (int i = 0; i < 4; ++i) {
//		glm::vec3 offset = DualContouring::adjacentVoxelsOffsets[axis][i];
//		int curX = x + static_cast<int>(offset.x);
//		int curY = y + static_cast<int>(offset.y);
//		int curZ = z + static_cast<int>(offset.z);
//		int uniqueIdx = GetUniqueIndexForGrid(curX, curY, curZ, gridWidth, gridHeight);
//		unsigned int vIdx = voxelVertexIndexMap[uniqueIdx];
//		rawIndices.push_back(vIdx);
//		// Retrieve the vertex position (assuming 3 floats per vertex)
//		glm::vec3 pos(
//			modelVert ices[vIdx],
//			modelVertices[vIdx + 1],
//			modelVertices[vIdx + 2]
//		);
//		positions.push_back(pos);
//	}
//
//	// Project the 3D positions onto the appropriate 2D plane.
//	// For axis 0: use the YZ plane (ignore x)
//	// For axis 1: use the XZ plane (ignore y)
//	// For axis 2: use the XY plane (ignore z)
//	std::vector<glm::vec2> projected;
//	for (const auto& pos : positions) {
//		glm::vec2 proj;
//		if (axis == 0)
//			proj = glm::vec2(pos.y, pos.z);
//		else if (axis == 1)
//			proj = glm::vec2(pos.x, pos.z);
//		else // axis == 2
//			proj = glm::vec2(pos.x, pos.y);
//		projected.push_back(proj);
//	}
//
//	// Compute the centroid of the projected points.
//	glm::vec2 centroid(0.0f, 0.0f);
//	for (const auto& proj : projected) {
//		centroid += proj;
//	}
//	centroid /= static_cast<float>(projected.size());
//
//	// For each projected point, compute the angle relative to the centroid.
//	// We'll store the angle together with the corresponding raw index.
//	std::vector<std::pair<float, unsigned int>> angleIndexPairs;
//	for (size_t i = 0; i < projected.size(); ++i) {
//		glm::vec2 dir = projected[i] - centroid;
//		float angle = std::atan2(dir.y, dir.x);
//		angleIndexPairs.push_back({ angle, rawIndices[i] });
//	}
//
//	// Sort the pairs by angle in ascending order.
//	std::sort(angleIndexPairs.begin(), angleIndexPairs.end(),
//		[](const std::pair<float, unsigned int>& a, const std::pair<float, unsigned int>& b) {
//			return a.first < b.first;
//		});
//
//	// Extract the sorted vertex indices.
	std::vector<unsigned int> orderedIndices;
//	for (const auto& p : angleIndexPairs) {
//		orderedIndices.push_back(p.second);
//	}
//
	return orderedIndices;
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

	appPtr->m_currentCamera->ProcessMouseInput(static_cast<float>(xposIn), static_cast<float>(yposIn), appPtr->settings.bIsCursorEnabled);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void App::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	appPtr->m_currentCamera->ProcessScrollInput(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void App::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		//Flip flag
		appPtr->settings.bIsCursorEnabled = !appPtr->settings.bIsCursorEnabled;
		glfwSetInputMode(window, GLFW_CURSOR, appPtr->settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}
}

App::~App()
{
}
