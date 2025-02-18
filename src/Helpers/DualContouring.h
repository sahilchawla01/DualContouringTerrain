#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class DualContouring
{
public:
	DualContouring(const unsigned int& gridWidth, const unsigned int& gridHeight, const unsigned int& gridDepth);
	~DualContouring();

	static const std::vector<glm::vec3> voxelCornerOffsets;
	static const std::vector<std::pair<int, int>> edgePairs;
	static const std::vector<std::vector<glm::vec3>> adjacentVoxelsOffsets;

	static const glm::vec3 GetIntersectionPoint(const glm::vec3& firstPosition, const glm::vec3& secondPosition, const glm::vec3& spherePosition, const float& sphereRadius, int totalSteps = 100);
	static const glm::vec3 CalculateSurfaceNormal(const glm::vec3& intersectionPos, const glm::vec3& spherePosition, const float& sphereRadius);

	void GenerateMesh(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices);

private:
	unsigned int m_gridWidth = 15;
	unsigned int m_gridHeight = 15;
	unsigned int m_gridDepth = 15;

};
