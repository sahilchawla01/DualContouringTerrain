#pragma once
#include <vector>
#include <glm/vec3.hpp>

#include "Helpers/DualContouring.h"

class QEFSolver
{
public:

	static glm::vec3 ComputeBestVertexPosition(const std::vector<VertexHermiteData>& hermiteDataPoints)
	{

        // Define A^T A and A^T b matrices
        glm::mat3 ATA(0.0f);
        glm::vec3 ATb(0.0f);

        // Compute A^T A and A^T b
        for (const auto& data : hermiteDataPoints) {
            glm::vec3 n = data.normal;
            float d = glm::dot(n, data.position); // b_i = n_i . p_i

            glm::mat3 outerProduct = glm::outerProduct(n, n); // n_i * n_i^T
            ATA += outerProduct; // Accumulate A^T A
            ATb += n * d; // Accumulate A^T b
        }

        // Solve the system A^T A * x = A^T b
        glm::vec3 x = glm::inverse(ATA) * ATb; // Solve for x using matrix inverse

        return x;

	}
};
