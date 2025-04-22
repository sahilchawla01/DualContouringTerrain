#pragma once

class Settings
{
public:

	float mouse_yaw = -90.f;
	float mouse_pitch = 0.f;

	bool bIsCursorEnabled = false;
	bool bIsDebugEnabled = false;
	bool bShouldFlatShade = false;
	static bool bIsDuplicateVerticesDebugEnabled;
	bool bIsVoxelDebugEnabled = false;
	//TODO: Eventually move to actor class
	bool bViewMesh = true;

};