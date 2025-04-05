#pragma once

enum class EAppState
{
	//The app state where SDF shapes are being created, added or deleted to the grid
	Modelling,
	//The app state where the SDFs have been finalized, and the user is now editing the created grid
	Editing
};