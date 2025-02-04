#include "DualContouring.h"
#include "Math/SDF.h"

DualContouring::DualContouring()
= default;

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
	//Front most left vertical edge adjacent voxels
{
		glm::vec3(-1.f, 0.f, -1.f),
		glm::vec3(-1.f, 0.f, 0.f),
		glm::vec3(0.f, 0.f, -1.f),
		glm::vec3(0.f, 0.f, 0.f),
	  },
	  //Left-Bottom most edge adjacent voxels
  {

		  glm::vec3(-1.f, 0.f, 0.f),
		  glm::vec3(-1.f, -1.f, 0.f),
		  glm::vec3(0.f, 0.f, 0.f),
		  glm::vec3(0.f, -1.f, 0.f),
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
