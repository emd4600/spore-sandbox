// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SandboxMode.h"
#include "SandboxMode2.h"
#include "FlatTerrain.h"
#include "SphereCamera.h"

void Initialize()
{
	// This method is executed when the game starts, before the user interface is shown
	// Here you can do things such as:
	//  - Add new cheats
	//  - Add new simulator classes
	//  - Add new game modes
	//  - Add new space tools
	//  - Change materials
	
	CameraManager.PutCamera(SphereCamera::ID, new SphereCamera());
	GameModeManager.AddGameMode(new SandboxMode(), id("SandboxProject::Mode"), "SandboxProject::Mode");
	GameModeManager.AddGameMode(new SandboxMode2(), id("SandboxProject::Mode2"), "SandboxProject::Mode2");
}

void Dispose()
{
	// This method is called when the game is closing
}

member_detour(SetTargetPos__detour, Editors::CreatureWalkController, void(const Vector3&, bool, bool))
{
	void detoured(const Vector3 & pos, bool applyNow, bool ignoreZ)
	{
		mRealTargetPosition = pos;
		mTargetPosition = pos;
		if (ignoreZ) mTargetPosition.z = 0.0f;

		if (applyNow) {
			original_function(this, pos, applyNow, ignoreZ);
		}
	}
};

void AttachDetours()
{
	// Call the attach() method on any detours you want to add
	// For example: cViewer_SetRenderType_detour::attach(GetAddress(cViewer, SetRenderType));

	Terrain::FlatTerrain::AttachDetours();

	SetTargetPos__detour::attach(GetAddress(Editors::CreatureWalkController, SetTargetPosition));
}


// Generally, you don't need to touch any code here
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ModAPI::AddPostInitFunction(Initialize);
		ModAPI::AddDisposeFunction(Dispose);

		PrepareDetours(hModule);
		AttachDetours();
		CommitDetours();
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

