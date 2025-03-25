#pragma once
#include "ISignedDistanceField.h"

class BoxSDF : public ISignedDistanceField
{
public:
    glm::vec3 center;
    glm::vec3 halfExtents;  // Half-size of the box in each dimension

    BoxSDF(const glm::vec3& boxCenter, const glm::vec3& boxHalfExtents) : center(boxCenter), halfExtents(boxHalfExtents) {}

    float EvaluateSDF(const glm::vec3 queryPoint) const override
    {
        glm::vec3 q = glm::abs(queryPoint - center) - halfExtents;
        return glm::length(glm::max(q, glm::vec3(0.0f))) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
    }

    SDFType GetType() const override { return SDFType::Box; }

};
