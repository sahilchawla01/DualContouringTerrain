#pragma once

#include <glm/glm.hpp>

class ISignedDistanceField
{
public:
	virtual ~ISignedDistanceField() = default;

	virtual float EvaluateSDF(const glm::vec3 queryPoint) const = 0;
};
