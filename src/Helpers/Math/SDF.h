#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//TODO: Remove class

struct BoxSDF
{
	glm::vec3 queryPoint;
	glm::vec3 bounds;
};

struct SphereSDF
{
	glm::vec3 queryPoint;
	glm::vec3 sphereCenter;
	glm::vec3 sphereRadius;
};


class SDF
{
	public:

	SDF();


	/*static float GetSphereSDFValue(const glm::vec3& queryPoint, const glm::vec3& sphereCenter, const float& sphereRadius)
	{
		return glm::length(queryPoint - sphereCenter) - sphereRadius;
	}

	static float GetBoxSDFValue(const glm::vec3& queryPoint, const glm::vec3& bounds)
	{
		glm::vec3 q = abs(queryPoint) - bounds;
		return glm::length(glm::max(q, glm::vec3(0.0f))) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);;
	}*/

	~SDF();
};
