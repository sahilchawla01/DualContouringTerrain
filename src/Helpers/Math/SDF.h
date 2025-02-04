#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SDF
{
	public:
	SDF();

	static float GetSphereSDFValue(const glm::vec3& queryPoint, const glm::vec3& sphereCenter, const float& sphereRadius)
	{
		return glm::length(queryPoint - sphereCenter) - sphereRadius;
	}

	~SDF();
};
