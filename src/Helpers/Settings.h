#pragma once

class Settings
{
public:

	float mouse_yaw = -90.f;
	float mouse_pitch = 0.f;

	bool bIsCursorEnabled = false;
	bool bIsEditingEnabled = true;
	bool bIsDebugEnabled = false;
	bool bShouldFlatShade = false;
	bool bIsVoxelDebugEnabled = false;
	//TODO: Eventually move to actor class
	bool bViewMesh = true;

};