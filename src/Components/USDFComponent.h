#pragma once
#include <iostream>
#include <vector>

#include "UActorComponent.h"
#include "Helpers/SDFs/ISignedDistanceField.h"


class USDFComponent : public UActorComponent
{

public:

	USDFComponent(std::weak_ptr<const AActor> owningActor) : UActorComponent(owningActor) {}
	~USDFComponent() override = default;

	//Add an SDF
	template<typename T, typename... Args>
	void AddSDF(Args&&... args)
	{
		//Set flag to regenerate the mesh
		bShouldRegenerateMesh = true;

		sdfList.push_back(std::make_shared<T>(std::forward<Args>(args)...));

		std::cout << "\nAdded an SDF to the list";
	}

	float EvaluateSDF(const glm::vec3& queryPoint) const
	{
		float result = std::numeric_limits<float>::max();

		if (sdfList.empty())
		{
			std::cout << "\nCould not evaluate SDF for this query point. The sdf list was empty";
			return result;
		}

		for (const auto& sdf : sdfList)
		{
			result = std::min(result, sdf->EvaluateSDF(queryPoint));
		}

		return result;
	}

	std::vector<std::shared_ptr<ISignedDistanceField>> GetSDFList() { return sdfList; }

	bool GetShouldRegenerateMesh() const { return bShouldRegenerateMesh; }
	void SetShouldRegenerateMesh(bool bRegenerate) { bShouldRegenerateMesh = bRegenerate; }
	


private:

	bool bShouldRegenerateMesh = true;

	//Stores all the sdf objects
	std::vector<std::shared_ptr<ISignedDistanceField>> sdfList;

};
