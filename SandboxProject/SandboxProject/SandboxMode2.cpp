#include "stdafx.h"
#include "SandboxMode2.h"
#include "SphereCamera.h"
#include <Spore\Anim\IAnimManager.h>

SandboxMode2::SandboxMode2()
	: mpCamera()
	, mInput()
	, mpModelWorld()
	, mpEffectWorld()
	, mpLightingWorld()
	, mpAnimWorld()
	, mpCreature()
	, mpCreatureController()
	, mpPlaneModel()
	, mpPBSky()
	, sunZenith(1.3f)
	, sunAzimuth(2.9f)
	, mpMovementCursorEffect()
	, mpTerrainSystem()
	, mpTestPlane()
{
	Initialize(nullptr);
}

SandboxMode2::~SandboxMode2()
{

}

// For internal use, do not touch this.
int SandboxMode2::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not touch this.
int SandboxMode2::Release()
{
	return DefaultRefCounted::Release();
}

// The use of this function is unknown.
bool SandboxMode2::func0Ch()
{
	return false;
}

bool SandboxMode2::Initialize(App::IGameModeManager* pManager)
{
	class Cheat : public ArgScript::ICommand
	{
	public:
		void ParseLine(const ArgScript::Line& line) override {
			GameModeManager.SetActiveMode(id("SandboxProject::Mode2"));
		}
	};
	CheatManager.AddCheat("sb2", new Cheat());

	class AnimCheat : public ArgScript::ICommand
	{
	public:
		void PlayAnimation(Anim::AnimatedCreature* creature, uint32_t animID, bool loop, int mode)
		{
			//TODO PlayMode does more things

			auto animIndex = creature->LoadAnimation(animID);
			creature->SetAnimationMode(animIndex, mode);
			creature->StartAnimation(animIndex);

			if (loop)
			{
				creature->SetLoop(animIndex, true);
				creature->SetLoopTimes(animIndex, -1.0f);
			}
		}

		void ParseLine(const ArgScript::Line& line) override {
			static const uint32_t csa_idle_walk_lookat_start = 0x0452B634;
			static const uint32_t csa_idle_walk_lookat_loop = 0x0452B64A;
			static const uint32_t csa_idle_walk_normal_armsmoving = 0x4485A08;

			// sub_62CD40
			auto* p = *Editor.mpPlayMode->mpMainActor;
			auto controller = p->mpWalkController.get();

			Vector3 position = { 0.0, 4.0, 0.0 };
			auto& anims = Editor.mpPlayMode->mAnimations;

			/*controller->SetTargetPosition(position, false, false);
			controller->SetTargetAngle(-Vector3::OrientedAngle(position - controller->mCurrentPosition, -Y_AXIS, Z_AXIS), false);

			auto creature = controller->GetAnimatedCreature();
			uint32_t animID;
			creature->GetCurrentAnimation(&animID);
			if (animID != csa_idle_walk_lookat_start && animID != csa_idle_walk_lookat_loop)
			{
				//PlayAnimation(creature, csa_idle_walk_lookat_start, false, 1);
				//PlayAnimation(creature, csa_idle_walk_lookat_loop, true, 0);
				//PlayAnimation(creature, csa_idle_walk_normal_armsmoving, true, 2);

				anims.PlayAnimation(3, csa_idle_walk_lookat_start, false, 1);
				anims.PlayAnimation(3, csa_idle_walk_lookat_loop, true, 0);
				anims.PlayAnimation(3, csa_idle_walk_normal_armsmoving, true, 2);
			}*/

			//char* object = CALL(Address(0x62CB30), char*, Args(Editors::PlayModeActor*, const Vector3&, float, bool, float),
			//	Args(p, position, 1.0f, false, 0.5f));
			//CALL(Address(0x62CE10), void, Args(Editors::PlayModeActor*, void*, bool), Args(p, object, false));

			controller->SetTargetPosition(position, false, false);
			controller->SetTargetAngle(-Vector3::OrientedAngle(position - controller->mCurrentPosition, -Y_AXIS, Z_AXIS), false);

			uint32_t animID;
			controller->mpAnimatedCreature->GetCurrentAnimation(&animID);

			//anims.PlayAnimation(p->mActorID, csa_idle_walk_lookat_start, false, animID == 0x4079859 ? 0 : 1);
			//controller->mpAnimatedCreature->field_154 = 2;
			//int animIndex;
			//anims.PlayAnimation(p->mActorID, csa_idle_walk_lookat_loop, true, 0, &animIndex);
			//if (controller->mpAnimatedCreature->field_184) {
			//	CALL(Address(0xA00670), void, Args(int, int), Args(controller->mpAnimatedCreature->field_184, animIndex));
			//}

			//anims.PlayAnimation(p->mActorID, csa_idle_walk_normal_armsmoving, true, 2);

			PlayAnimation(controller->mpAnimatedCreature.get(), csa_idle_walk_lookat_start, false, 1);
			controller->mpAnimatedCreature->field_154 = 2;
			PlayAnimation(controller->mpAnimatedCreature.get(), csa_idle_walk_lookat_loop, true, 0);
			PlayAnimation(controller->mpAnimatedCreature.get(), csa_idle_walk_normal_armsmoving, true, 2);

			//CALL(Address(0x62CC30), void, Args(Editors::PlayModeActor*, const Vector3&, bool), Args(p, position, false));
			//CALL(Address(0x628980), void, Args(int, Anim::AnimatedCreature*, int), Args(p->field_14, p->mpCreature.get(), p->mActorID));
		}
	};
	CheatManager.AddCheat("testWalk", new AnimCheat());
	
	return true;
}

bool SandboxMode2::Dispose()
{
	// Called when the game exits. Here you should dispose all your model/effect worlds,
	// ensure all objects are released, etc
	
	return true;
}

bool SandboxMode2::OnEnter()
{
	mpModelWorld = ModelManager.CreateWorld(WORLD_ID);
	mpLightingWorld = LightingManager.CreateWorld(WORLD_ID);
	mpEffectWorld = SwarmManager.CreateWorld(WORLD_ID);
	SwarmManager.SetActiveWorld(mpEffectWorld.get());
	mpAnimWorld = AnimManager.CreateWorld(u"SandboxMode");

	mpAnimWorld->SetEffectWorld(mpEffectWorld.get());
	mpAnimWorld->SetModelWorld(mpModelWorld.get());

	//TODO sub_7826A0 related with shadows? check end of Editor_Update
	mpLightingWorld->SetConfiguration(id("SandboxMode"));
	mpModelWorld->AddLightingWorld(mpLightingWorld.get(), 0, false);
	mpModelWorld->SetVisible(true);

	RenderManager.AddRenderable(mpModelWorld->ToRenderable(), 8);

	//CameraManager.SetActiveCameraByID(id("EffectEditorCamera"));
	CameraManager.SetActiveCameraByID(SphereCamera::ID);
	mpCamera = (SphereCamera*)CameraManager.GetActiveCamera();

	/*mpPlaneModel = mpModelWorld->LoadModel(id("colorgrid_plane"), id("SandboxModels"));
	mpModelWorld->UpdateModel(mpPlaneModel.get());
	mpModelWorld->SetModelVisible(mpPlaneModel.get(), false);*/

	mpTestBallModel = mpModelWorld->LoadModel(id("PbrTest"), id("SandboxModels"));

	//mpTestPlane = mpModelWorld->LoadModel(id("test_pbr_plane"), id("SandboxModels"));

	mpCreature = mpAnimWorld->LoadCreature({ 0x67cd060, TypeIDs::crt, GroupIDs::CreatureModels });
	mpModelWorld->UpdateModel(mpCreature->GetModel());
	mpModelWorld->SetModelVisible(mpCreature->GetModel(), true);

	mpCreatureController = new CreatureController(mpCreature.get(), mpModelWorld.get());
	//mpCreatureController->field_84 = true;

	//mpCreatureController->field_58 = { 0, 0, 1 };


	mpPBSky = new PhysicallyBasedSky();
	mpPBSky->Load(id("Atmosphere"), id("SandboxMode"));
	mpPBSky->Precompute();
	mpPBSky->SetSun(sunZenith, sunAzimuth);
	RenderManager.AddRenderable(mpPBSky.get(), 4);

	UpdateSunPosition();

	mpTerrainSystem = new SbTerrainSystem();
	mpTerrainSystem->Generate();
	mpTerrainSystem->SetLightingWorld(mpLightingWorld.get());
	RenderManager.AddRenderable(mpTerrainSystem.get(), 5);

	return true;
}

void SandboxMode2::UpdateSunPosition() {
	mpPBSky->SetSun(sunZenith, sunAzimuth);

	Vector3 sunDirection = -Vector3(
		cosf(sunAzimuth) * sinf(sunZenith),
		sinf(sunAzimuth) * sinf(sunZenith),
		cosf(sunZenith)
	);

	mpLightingWorld->SetWorldTransform(Transform().SetRotation(Quaternion::GetRotationTo(Y_AXIS, sunDirection)));
}

void SandboxMode2::OnExit()
{
	// Called when the game mode is exited. Here you should kill all effects and models, 
	// stop any renderers, unload the UI, etc.
}

// The use of this function is unknown.
void* SandboxMode2::func20h(int) 
{
	return nullptr;
}


//// INPUT LISTENERS ////

// Called when a keyboard key button is pressed.
bool SandboxMode2::OnKeyDown(int virtualKey, KeyModifiers modifiers)
{
	mInput.OnKeyDown(virtualKey, modifiers);
	
	// Return true if the keyboard event has been handled in this method.
	return false;
}

// Called when a keyboard key button is released.
bool SandboxMode2::OnKeyUp(int virtualKey, KeyModifiers modifiers)
{
	mInput.OnKeyUp(virtualKey, modifiers);

	if (virtualKey == int('O')) {
		mpPBSky->SetForceNoOzone(!mpPBSky->GetForceNoOzone());
		mpPBSky->Precompute();
	}
	else if (virtualKey == VK_SPACE) {
		PlayAnimation(0x04DD7391, false, 0);
	}
	
	// Return true if the keyboard event has been handled in this method.
	return false;
}

// Called when a mouse button is pressed (this includes the mouse wheel button).
bool SandboxMode2::OnMouseDown(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseDown(mouseButton, mouseX, mouseY, mouseState);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}

bool RayIntersectPlane(Vector3& dst, const Vector3& pos, const Vector3& dir, const Vector3& pointOnPlane, const Vector3& planeNormal)
{
	float d = (pointOnPlane - pos).Dot(planeNormal) / dir.Dot(planeNormal);
	if (d < 0) return false;

	dst = pos + d * dir;

	return true;
}

// Called when a mouse button is released (this includes the mouse wheel button).
bool SandboxMode2::OnMouseUp(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseUp(mouseButton, mouseX, mouseY, mouseState);

	if (mouseButton == MouseButton::kMouseButtonRight)
	{
		Vector3 viewPos, viewDir;
		if (App::GetViewer()->GetCameraToPoint(mouseX, mouseY, viewPos, viewDir))
		{
			// Calculate when it intersects with plane z=0
			Vector3 position;
			if (RayIntersectPlane(position, viewPos, viewDir, Vector3::ZERO, Z_AXIS)) {
				WalkTo(position);

				// effects 384E54A4 8660A7F4
				if (mpMovementCursorEffect) {
					mpMovementCursorEffect->Stop();
					mpMovementCursorEffect = nullptr;
				}
				if (SwarmManager.CreateEffect(0xE4173005, 0, mpMovementCursorEffect)) {
					mpMovementCursorEffect->SetTransform(Transform().SetOffset(position));
					mpMovementCursorEffect->Start();
				}
			}
		}
	}

	return false;
}


// Called when the mouse is moved.
bool SandboxMode2::OnMouseMove(float mouseX, float mouseY, MouseState mouseState)
{
	// Before updating the mouse position, calculate the sun move
	constexpr float kScale = 500.0f;
	if (mouseState.IsCtrlDown && mouseState.IsLeftButtonDown) {
		sunZenith -= (mInput.mousePosition.y - mouseY) / kScale;
		sunZenith = max(0.0f, min(Math::PI, sunZenith));
		sunAzimuth += (mInput.mousePosition.x - mouseX) / kScale;

		UpdateSunPosition();
	}

	mInput.OnMouseMove(mouseX, mouseY, mouseState);

	// Return true if the mouse event has been handled in this method.
	return false;
}

// Called when the mouse wheel is scrolled. 
// This method is not called when the mouse wheel is pressed.
bool SandboxMode2::OnMouseWheel(int wheelDelta, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseWheel(wheelDelta, mouseX, mouseY, mouseState);

	// Return true if the mouse event has been handled in this method.
	return false;
}


//// UPDATE FUNCTION ////

void SandboxMode2::Update(float dt, float delta2)
{
	if (mpCreature) {
		mpCamera->SetTarget(mpCreature->mPosition + 1.0f * Z_AXIS);

		if (mInput.IsKeyDown(int('W'))) {
			float speed = mpCreatureController->GetMovementSpeed() * dt;
			Vector3 dest = mpCreatureController->GetCurrentPosition() +
				Matrix3::FromAxisAngle(Z_AXIS, mpCreatureController->GetTargetAngle()) * -Y_AXIS * speed;
			mpCreatureController->SetTargetPosition(dest);
		}
		if (mInput.IsKeyDown(int('S'))) {
			float speed = mpCreatureController->GetMovementSpeed() * dt * 0.5f;
			Vector3 dest = mpCreatureController->GetCurrentPosition() +
				Matrix3::FromAxisAngle(Z_AXIS, mpCreatureController->GetTargetAngle()) * Y_AXIS * speed;
			mpCreatureController->SetTargetPosition(dest);
		}
		if (mInput.IsKeyDown(int('A'))) {
			float newAngle = mpCreatureController->GetTargetAngle() + dt * mpCreatureController->GetAngularSpeed() * 0.7f;
			mpCreatureController->SetTargetAngle(newAngle);
		}
		if (mInput.IsKeyDown(int('D'))) {
			float newAngle = mpCreatureController->GetTargetAngle() - dt * mpCreatureController->GetAngularSpeed() * 0.7f;
			mpCreatureController->SetTargetAngle(newAngle);
		}
	}

	if (mpAnimWorld) {
		if (mpCreatureController) {
			mpCreatureController->Update(dt);

			//mpCreatureController->field_58 = {3.0f, 3.0f, 1.0f};
			//mpCreatureController->field_64 = { 3.0f, 3.0f, 1.0f };

	//		if (mpCreatureController->mCurrentPosition == mpCreatureController->mTargetPosition)
//			{

			//}
		}

		mpAnimWorld->Update(dt);
	}
}


void SandboxMode2::PlayAnimation(uint32_t animID, bool loop, int mode)
{
	//TODO PlayMode does more things

	if (animID == 0x4485A08)
	{
		int* p = STATIC_CALL(Address(ModAPI::ChooseAddress(0x9FF670, 0x9FF630)), int*, Args(int, Anim::AnimIndex), Args(mpCreature->field_184, mpCreature->field_18C));
		if (!p || p[0xD8 / 4]) {
			auto animIndex = mpCreature->LoadAnimation(animID);
			mpCreature->SetAnimationMode(animIndex, mode);
			mpCreature->field_18C = animIndex;
			mpCreature->StartAnimation(animIndex);

		}
		if (p && p[0xD8 / 4]) {
			CALL(Address(ModAPI::ChooseAddress(0xA00F10, 0xA00ED0)), void, Args(int, int*, int), Args(mpCreature->field_184, p, 0));
		}

		if (loop)
		{
			mpCreature->SetLoop(mpCreature->field_18C, true);
			mpCreature->SetLoopTimes(mpCreature->field_18C, -1.0f);
		}
	}
	else {
		auto animIndex = mpCreature->LoadAnimation(animID);
		mpCreature->SetAnimationMode(animIndex, mode);
		mpCreature->StartAnimation(animIndex);

		if (loop)
		{
			mpCreature->SetLoop(animIndex, true);
			mpCreature->SetLoopTimes(animIndex, -1.0f);
		}
	}
}

void SandboxMode2::WalkTo(const Vector3& position)
{
	static const uint32_t csa_idle_walk_lookat_start = 0x0452B634;
	static const uint32_t csa_idle_walk_lookat_loop = 0x0452B64A;
	static const uint32_t csa_idle_walk_normal_armsmoving = 0x4485A08;


	Vector3 currentPos = mpCreatureController->GetCurrentPosition();
	if (position != currentPos)
	{
		mpCreatureController->SetTargetPosition(position);
		mpCreatureController->SetTargetAngle(-Vector3::OrientedAngle(position - currentPos, -Y_AXIS, Z_AXIS));
	}

	
	uint32_t animID;
	mpCreature->GetCurrentAnimation(&animID);
	if (animID != csa_idle_walk_lookat_start && animID != csa_idle_walk_lookat_loop)
	{
		PlayAnimation(csa_idle_walk_lookat_start, false, 1);
		mpCreature->field_154 = 2;
		PlayAnimation(csa_idle_walk_lookat_loop, true, 0);
		PlayAnimation(csa_idle_walk_normal_armsmoving, true, 2);
	}
}