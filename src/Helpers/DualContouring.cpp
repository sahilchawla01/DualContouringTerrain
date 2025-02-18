#include "DualContouring.h"

#include <array>
#include <iostream>
#include <unordered_map>
#include <glad/glad.h>
#include <Helpers/Settings.h>
#include <Actors/ACamera.h>
#include "Shader.h"
#include "Math/SDF.h"


DualContouring::DualContouring(const unsigned int& gridWidth, const unsigned int& gridHeight,
	const unsigned int& gridDepth, const unsigned int& voxelSize)
{
	this->m_gridWidth = gridWidth;
	this->m_gridHeight = gridHeight;
	this->m_gridDepth = gridDepth;
	this->m_voxelSize = voxelSize;
}

DualContouring::~DualContouring()
= default;


const std::vector<glm::vec3> DualContouring::voxelCornerOffsets =
{
	//Lower cube vertices
	glm::vec3(0, 0, 0),
	glm::vec3(1, 0, 0),
	glm::vec3(1, 0, 1),
	glm::vec3(0, 0, 1),

	//Upper cube vertices
	glm::vec3(0, 1, 0),
	glm::vec3(1, 1, 0),
	glm::vec3(1, 1, 1),
	glm::vec3(0, 1, 1),

};

const std::vector<std::pair<int, int>> DualContouring::edgePairs =
{
		std::make_pair(0, 1), // 0 
		std::make_pair(1, 2),
		std::make_pair(2, 3),
		std::make_pair(0, 3), // 3
		std::make_pair(4, 5),
		std::make_pair(5, 6),
		std::make_pair(6, 7),
		std::make_pair(7, 4),
		std::make_pair(0, 4), // 8
		std::make_pair(1, 5),
		std::make_pair(2, 6),
		std::make_pair(3, 7)
};

const std::vector<std::vector<glm::vec3>> DualContouring::adjacentVoxelsOffsets =
{
	//Front-Bottom Horizontal edge adjacent voxels
	{

			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, -1.f, 0.f),
			glm::vec3(0.f, 0.f, -1.f),
			glm::vec3(0.f, -1.f, -1.f),

	},
	  //Left-Bottom most edge adjacent voxels
  {

		  glm::vec3(-1.f, 0.f, 0.f),
		  glm::vec3(-1.f, -1.f, 0.f),
		  glm::vec3(0.f, 0.f, 0.f),
		  glm::vec3(0.f, -1.f, 0.f),
  },
	//Front most left vertical edge adjacent voxels
 {
		glm::vec3(-1.f, 0.f, -1.f),
		glm::vec3(-1.f, 0.f, 0.f),
		glm::vec3(0.f, 0.f, -1.f),
		glm::vec3(0.f, 0.f, 0.f),
	}
};


const glm::vec3 DualContouring::GetIntersectionPoint(const glm::vec3& firstPosition, const glm::vec3& secondPosition, const glm::vec3& spherePosition, const float& sphereRadius, int totalSteps)
{
	glm::vec3 direction = glm::normalize(secondPosition - firstPosition);
	const float step = 1 / static_cast<float>(totalSteps);

	float minValue = 100000.f;

	float finalT = 0.f;
	float currentT = 0.f;

	while(currentT <= 1.f)
	{
		//TODO: Change it to a generic SDF function
		const glm::vec3 currentPos = firstPosition + (direction * currentT);
		const float density = SDF::GetSphereSDFValue(currentPos, spherePosition, sphereRadius);

		if(density < minValue)
		{
			minValue = density;
			finalT = currentT;
		}

		currentT += step;
	}

	return firstPosition + (direction * finalT);
	
}

const glm::vec3 DualContouring::CalculateSurfaceNormal(const glm::vec3& intersectionPos, const glm::vec3& spherePosition,
	const float& sphereRadius)
{
	//Perform Finite Sum Difference
	const float h = 0.001f;

	const float dx = SDF::GetSphereSDFValue(intersectionPos + glm::vec3(h, 0.f, 0.f), spherePosition, sphereRadius) - SDF::GetSphereSDFValue(intersectionPos - glm::vec3(h, 0.f, 0.f), spherePosition, sphereRadius);
	const float dy = SDF::GetSphereSDFValue(intersectionPos + glm::vec3(0.f, h, 0.f), spherePosition, sphereRadius) - SDF::GetSphereSDFValue(intersectionPos - glm::vec3(0.f, h, 0.f), spherePosition, sphereRadius);
	const float dz = SDF::GetSphereSDFValue(intersectionPos + glm::vec3(0.f, 0.f, h), spherePosition, sphereRadius) - SDF::GetSphereSDFValue(intersectionPos - glm::vec3(0.f, 0.f, h), spherePosition, sphereRadius);

	return glm::normalize(glm::vec3(dx, dy, dz));
}

void DualContouring::DebugDrawVertices(std::weak_ptr<ACamera> curCamera, std::weak_ptr<Settings> settings)
{
	//DEBUG: Spawn cube at vertex positions
	if (settings.lock()->bIsDebugEnabled)
	{

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
		Shader unlitShader("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag");

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

		unlitShader.use();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal shading
		//Draw vertex position
		for (int x = 0; x < this->m_gridWidth * this->m_voxelSize; x += this->m_voxelSize)
		{
			for (int y = 0; y < this->m_gridHeight * this->m_voxelSize; y += this->m_voxelSize)
			{
				for (int z = 0; z < this->m_gridDepth * this->m_voxelSize; z += this->m_voxelSize)
				{
					//If a vertex isn't there in a voxel, skip. 
					if (voxelVertexIndexMap.find(GetUniqueIndexForGrid(x, y, z, this->m_gridWidth, this->m_gridHeight)) == voxelVertexIndexMap.end())
					{
						continue;
					}

					//Draw vertex position
					int modelVerticesIndex = voxelVertexIndexMap[GetUniqueIndexForGrid(x, y, z, this->m_gridWidth, this->m_gridHeight)];
					glm::vec3 vertexPos(vertices[modelVerticesIndex], vertices[modelVerticesIndex + 1], vertices[modelVerticesIndex + 2]);

					//Create MVP
					glm::mat4 model = glm::mat4(1.0f);
					model = glm::translate(model, vertexPos);
					model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

					glm::mat4 view = curCamera.lock()->GetViewMatrix();

					glm::mat4 projection = curCamera.lock()->GetProjectionMatrix();

					glm::mat4 mvp = projection * view * model;
					glm::vec3 debugCubeColor(0.027f, 0.843f, 1.0f);
					unlitShader.setMat4("mvp", mvp);
					unlitShader.setVec3("color", debugCubeColor);

					glDrawArrays(GL_TRIANGLES, 0, 36);

				}
			}
		}
	}
}

void DualContouring::GenerateMesh(std::vector<float>& vertices, std::vector<float>& normals,
                                  std::vector<unsigned int>& indices)
{

	std::vector<glm::vec3> grid;
	std::vector<float> modelVertices;
	std::vector<float> modelNormals;
	std::vector<unsigned int> modelIndices;
	grid.reserve(this->m_gridWidth * this->m_gridHeight * this->m_gridDepth);

	glm::vec3 gridPosition(0.f, 0.f, 0.f);

	glm::mat4 gridModelMatrix(1.f);
	glm::translate(gridModelMatrix, gridPosition);

	glm::vec3 gridCenter((this->m_gridWidth * this->m_voxelSize) / 2, (this->m_gridHeight * this->m_voxelSize) / 2, (this->m_gridDepth * this->m_voxelSize) / 2);

	//Generate vertex positions
	for (int x = 0; x < this->m_gridWidth * this->m_voxelSize; x += this->m_voxelSize)
	{
		for (int y = 0; y < this->m_gridHeight * this->m_voxelSize; y += this->m_voxelSize)
		{
			for (int z = 0; z < this->m_gridDepth * this->m_voxelSize; z += this->m_voxelSize)
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

					for (int i = 0; i < 8; ++i)
					{
						//Get current position 
						glm::vec3 currentCornerPos = (voxelCornerOffsets[i] * static_cast<float>(this->m_voxelSize)) + relativePos;

						//Calculate distance
						float distanceValue = SDF::GetSphereSDFValue(currentCornerPos, gridPosition, 4.f);
						cornerSDFValues[i] = distanceValue;

						//TODO: convert position from grid relative to SDF center relative
						//If within the surface, consider for triangulation
						if (distanceValue <= 0.f)
						{
							cornersToConsider |= 1 << i;
						}
					}

					//If the voxel is completely within the surface, or outside the volume, ignore it.
					if (cornersToConsider == 0 || cornersToConsider == 255)
						continue;

					std::vector<glm::vec3> intersectionPoints;
					std::vector<glm::vec3> intersectionNormals;

					//Array describes if an intersection occurs (first element), and if so, if intersection is + to -ve (second element) is true, else false.

					std::array<std::pair<bool, bool>, 3> adjacentEdgesCrossingOver{ {
							std::make_pair(false, false),
							std::make_pair(false, false),
							std::make_pair(false, false),
					} };
	
					for (int i = 0; i < 12; ++i)
					{
						const int cornerIndex1 = edgePairs[i].first;
						const int cornerIndex2 = edgePairs[i].second;


						const int m1 = (cornersToConsider >> cornerIndex1) & 1;
						const int m2 = (cornersToConsider >> cornerIndex2) & 1;

						//This means that the edge has no crossing over from one sign to the other, skip.
						if (m1 == m2)
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
						else if (i == 8)
						{
							//Mark intersection occurred 
							adjacentEdgesCrossingOver[2].first = true;
							//If intersection is from + to -ve, mark as true, else false
							adjacentEdgesCrossingOver[2].second = (m1 < m2);
						}

						//Find position along the edge where surface crosses signs

						//TODO: convert position from grid relative to SDF center relative
						const glm::vec3 cornerPos1 = (voxelCornerOffsets[cornerIndex1] * static_cast<float>(this->m_voxelSize)) + relativePos;
						const glm::vec3 cornerPos2 = (voxelCornerOffsets[cornerIndex2] * static_cast<float>(this->m_voxelSize)) + relativePos;

						//Get current intersection point by using linear interpolation
						float interpolateFactor = abs(cornerSDFValues[cornerIndex1]) / (abs(cornerSDFValues[cornerIndex1]) + abs(cornerSDFValues[cornerIndex2]));
						glm::vec3 currIntersectionPoint = cornerPos1 + ((cornerPos2 - cornerPos1) * interpolateFactor);
						intersectionPoints.push_back(currIntersectionPoint);

						//TODO: SAnity check whether point is within voxel

						//Calculate normal using Finite Sum Difference
						intersectionNormals.push_back(CalculateSurfaceNormal(currIntersectionPoint, gridPosition, 4.f));
					}

					
					//Map voxel to adjacent edges vector
					voxelEdgeIsoSurfaceMap[GetUniqueIndexForGrid(x, y, z, this->m_gridWidth, this->m_gridHeight)] = adjacentEdgesCrossingOver;

					glm::vec3 vertexPos(0.f);
					for (const glm::vec3& pos : intersectionPoints)
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
					if (voxelVertexIndexMap.find(GetUniqueIndexForGrid(x, y, z, this->m_gridWidth, this->m_gridHeight)) != voxelVertexIndexMap.end())
					{
						std::cout << "ERROR: THIS UNIQUE ID HAS ALREADY BEEN SET\n";
					}

					//Map voxel to vertex array index position
					voxelVertexIndexMap[GetUniqueIndexForGrid(x, y, z, this->m_gridWidth, this->m_gridHeight)] = static_cast<int>(modelVertices.size());

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


	

		//Draw voxels
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	if (settings.bIsVoxelDebugEnabled)
	//	{
	//		for (int x = 0; x < gridWidth * voxelSize; x += voxelSize)
	//		{
	//			for (int y = 0; y < gridHeight * voxelSize; y += voxelSize)
	//			{
	//				for (int z = 0; z < gridDepth * voxelSize; z += voxelSize)
	//				{


	//					glm::vec3 relativePos(static_cast<float>(x) - gridCenter.x, static_cast<float>(y) - gridCenter.y, static_cast<float>(z) - gridCenter.z);

	//					relativePos += gridPosition;

	//					glm::mat4 model = glm::mat4(1.0f);
	//					model = glm::translate(model, relativePos);
	//					model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	//					glm::mat4 view = m_currentCamera->GetViewMatrix();
	//					glm::mat4 projection = m_currentCamera->GetProjectionMatrix();
	//					glm::mat4 mvp = projection * view * model;
	//					glm::vec3 voxelCubeColor(1.f, 0.984f, 0.f);
	//					unlitShader.setMat4("mvp", mvp);
	//					unlitShader.setVec3("color", voxelCubeColor);

	//					glDrawArrays(GL_TRIANGLES, 0, 36);

	//				}

	//			}
	//		}
	//	}

	//	//Unbind the VAO
	//	glBindVertexArray(0);

	//}


	//Iterate through the cubes again, and make the edge connections
	for (int x = 0; x < this->m_gridWidth * m_voxelSize; x += m_voxelSize)
	{
		for (int y = 0; y < this->m_gridHeight * m_voxelSize; y += m_voxelSize)
		{
			for (int z = 0; z < this->m_gridDepth * m_voxelSize; z += m_voxelSize)
			{
				const std::array<std::pair<bool, bool>, 3> adjacentEdgesIntersection = voxelEdgeIsoSurfaceMap[
					GetUniqueIndexForGrid(x, y, z, this->m_gridWidth, this->m_gridHeight)];

				//Iterate over adjacent edges and make connections where possible
				for (int axis = 0; axis < 3; ++axis)
				{
					//For an edge, check if an intersection exists and all 4 neighboring voxels are valid
					if (adjacentEdgesIntersection[axis].first == true && ((x - 1) > 0 && (y - 1) > 0 && (z - 1) > 0))
					{

						std::vector<unsigned int> vertexIndices;
						std::vector<std::vector<int>> debugIndices;

						//Store vertex indices for neighboring voxels
						for (int i = 0; i < 4; ++i)
						{
							int curX = x + static_cast<int>(DualContouring::adjacentVoxelsOffsets[axis][i].x);
							int curY = y + static_cast<int>(DualContouring::adjacentVoxelsOffsets[axis][i].y);
							int curZ = z + static_cast<int>(DualContouring::adjacentVoxelsOffsets[axis][i].z);

							auto it = voxelVertexIndexMap.find(GetUniqueIndexForGrid(curX, curY, curZ, this->m_gridWidth, this->m_gridHeight));

							debugIndices.push_back({ curX, curY, curZ });

							if (it == voxelVertexIndexMap.end())
								continue;  // If voxel is missing, just skip it

							// Store the found vertex index
							vertexIndices.push_back(it->second / 3);
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
							if (adjacentEdgesIntersection[axis].second == true)
							{
								//Triangle 1
								modelIndices.push_back(vertexIndices[1]);
								modelIndices.push_back(vertexIndices[3]);
								modelIndices.push_back(vertexIndices[2]);

								//Triangle 2
								modelIndices.push_back(vertexIndices[0]);
								modelIndices.push_back(vertexIndices[1]);
								modelIndices.push_back(vertexIndices[2]);

							}
							else //Reverse indices order otherwise (-ve to +ve transition)
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

	//Finally assign the mesh details
	vertices = modelVertices;
	normals = modelNormals;
	indices = modelIndices;

	//Draw the model
	//if (settings.bViewMesh)
	//{


	//	glm::mat4 view = m_currentCamera->GetViewMatrix();

	//	glm::mat4 projection = m_currentCamera->GetProjectionMatrix();

	//	glm::mat4 modelViewMatrix = view * gridModelMatrix;
	//	glm::mat4 mvp = projection * modelViewMatrix;

	//	litShader.setMat4("modelViewMatrix", modelViewMatrix);
	//	litShader.setVec3("objectColor", glm::vec3(1.0));
	//	litShader.setMat4("mvp", mvp);
	//	//unlitShader.setMat4("mvp", mvp);
	//	//unlitShader.setVec3("color", glm::vec3(0.8f));

	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal shading

	//	glDrawElements(GL_TRIANGLES, static_cast<int>(modelIndices.size()), GL_UNSIGNED_INT, 0);

	//}

	////Now we can unbind the buffer
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	////Next unbind the VAO
	//glBindVertexArray(0);
}

int DualContouring::GetUniqueIndexForGrid(const int x, const int y, const int z, const int gridWidth,
	const int gridHeight)
{
	return x + gridWidth * (y + gridHeight * z);
}
