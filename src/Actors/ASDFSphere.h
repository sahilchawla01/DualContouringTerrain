#pragma once
#include "AActor.h"

class ACamera;

class ASDFSphere : AActor
{
public:
	ASDFSphere() = default;
	virtual ~ASDFSphere() = default;

private:
	void SetupShader() override;
	void UseShader() override;

};
