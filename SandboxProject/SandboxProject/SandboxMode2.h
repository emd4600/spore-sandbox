#pragma once

#include <Spore\BasicIncludes.h>
#include "CreatureController.h"
#include "SphereCamera.h"
#include "PhysicallyBasedSky.h"

#define SandboxMode2Ptr intrusive_ptr<SandboxMode2>

class SandboxMode2 
	: public App::IGameMode
	, public DefaultRefCounted
{
public:
	static const uint32_t WORLD_ID = id("SandboxMode2::WORLD");

	SandboxMode2();

	int AddRef() override;
	int Release() override;
	~SandboxMode2();

	bool func0Ch() override;
	bool Initialize(App::IGameModeManager* pManager) override;
	bool Dispose() override;
	bool OnEnter() override;
	void OnExit() override;
	void* func20h(int) override;

	bool OnKeyDown(int virtualKey, KeyModifiers modifiers) override;
	bool OnKeyUp(int virtualKey, KeyModifiers modifiers) override;
	bool OnMouseDown(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState) override;
	bool OnMouseUp(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState) override;
	bool OnMouseMove(float mouseX, float mouseY, MouseState mouseState) override;
	bool OnMouseWheel(int wheelDelta, float mouseX, float mouseY, MouseState mouseState) override;
	void Update(float delta1, float delta2) override;
	
protected:
	void WalkTo(const Vector3& position);
	void PlayAnimation(uint32_t animID, bool loop, int mode);

	GameInput mInput;
	SphereCamera* mpCamera;

	PhysicallyBasedSkyPtr mpPBSky;
	float sunZenith;
	float sunAzimuth;

	IModelWorldPtr mpModelWorld;
	IEffectWorldPtr mpEffectWorld;
	ILightingWorldPtr mpLightingWorld;
	IAnimWorldPtr mpAnimWorld;
	AnimatedCreaturePtr mpCreature;
	intrusive_ptr<CreatureController> mpCreatureController;

	ModelPtr mpPlaneModel;
	ModelPtr mpTestBallModel;

	IEffectPtr mpMovementCursorEffect;
};
