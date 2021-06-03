#include "stdafx.h"
#include "SandboxMode.h"
#include "FlatTerrain.h"
#include <Spore\Simulator\cInteractiveOrnament.h>

using namespace Simulator;

SandboxMode::SandboxMode()
	: mInput()
	, mpCreature()
{
	Initialize(nullptr);
}

SandboxMode::~SandboxMode()
{

}

// For internal use, do not touch this.
int SandboxMode::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not touch this.
int SandboxMode::Release()
{
	return DefaultRefCounted::Release();
}

// The use of this function is unknown.
bool SandboxMode::func0Ch()
{
	return false;
}

bool SandboxMode::Initialize(App::IGameModeManager* pManager)
{
	class Cheat : public ArgScript::ICommand
	{
	public:
		void ParseLine(const ArgScript::Line& line) override {
			GameModeManager.SetActiveMode(id("SandboxProject::Mode"));
		}
	};
	CheatManager.AddCheat("sb", new Cheat());
	return true;
}

bool SandboxMode::Dispose()
{
	// Called when the game exits. Here you should dispose all your model/effect worlds,
	// ensure all objects are released, etc
	
	return true;
}

bool SandboxMode::OnEnter()
{
	CameraManager.SetActiveCameraByID(id("EffectEditorCamera"));

	Simulator::InitializeWithoutPlanet();

	Simulator::GetLightingWorld()->SetConfiguration(id("CreatureEditor"));
	
	// creature_editorModel~!0x19A3A9AE.crt
	// ResourceKey(0x1C6A4146, TypeIDs::crt, 0x40626200)
	auto species = SpeciesManager.GetSpeciesProfile({ 0x19A3A9AE, TypeIDs::crt, 0x40626200 });
	mpCreature = Simulator::cCreatureAnimal::Create(Vector3(0, 0, 1.5f), species, 1, nullptr, true);
	assert(mpCreature);
	mpCreature->mPlanetCorrection = 0;
		
	/*auto model = simulator_new<Simulator::cInteractiveOrnament>();
	model->SetModelKey({ id("EP1_sg_rare_fossils_04"), TypeIDs::prop, GroupIDs::CivicObjects });
	model->SetScale(0.3f);*/

	auto terrain = new Terrain::FlatTerrain();
	PlanetModel.mpTerrain = terrain;
	PlanetModel.mpTerrain2 = terrain;
	MessageBoxA(NULL, "", "", MB_OK);

	terrain->Load();

	//TODO fix crash loc_AF3778, we need height and normal map
	
	return true;
}

void SandboxMode::OnExit()
{
	// Called when the game mode is exited. Here you should kill all effects and models, 
	// stop any renderers, unload the UI, etc.
}

// The use of this function is unknown.
void* SandboxMode::func20h(int) 
{
	return nullptr;
}


//// INPUT LISTENERS ////

// Called when a keyboard key button is pressed.
bool SandboxMode::OnKeyDown(int virtualKey, KeyModifiers modifiers)
{
	mInput.OnKeyDown(virtualKey, modifiers);
	
	// Return true if the keyboard event has been handled in this method.
	return false;
}

// Called when a keyboard key button is released.
bool SandboxMode::OnKeyUp(int virtualKey, KeyModifiers modifiers)
{
	mInput.OnKeyUp(virtualKey, modifiers);

	if (virtualKey == 'F') {
		uint32_t animID = 0x04FFA018;
		mpCreature->PlayAnimation(animID);
	}
	else if (virtualKey == 'G') {
		mpCreature->WalkTo(cCreatureAnimal::kStandardSpeed, Vector3(5, 0, mpCreature->GetPosition().z), Vector3(0, 0, 0));
	}

	// Return true if the keyboard event has been handled in this method.
	return false;
}

// Called when a mouse button is pressed (this includes the mouse wheel button).
bool SandboxMode::OnMouseDown(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseDown(mouseButton, mouseX, mouseY, mouseState);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}

// Called when a mouse button is released (this includes the mouse wheel button).
bool SandboxMode::OnMouseUp(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseUp(mouseButton, mouseX, mouseY, mouseState);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}

// Called when the mouse is moved.
bool SandboxMode::OnMouseMove(float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseMove(mouseX, mouseY, mouseState);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}

// Called when the mouse wheel is scrolled. 
// This method is not called when the mouse wheel is pressed.
bool SandboxMode::OnMouseWheel(int wheelDelta, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseWheel(wheelDelta, mouseX, mouseY, mouseState);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}


//// UPDATE FUNCTION ////

void SandboxMode::Update(float dt, float delta2)
{
	SimulatorSystem.Update(int(roundf(dt * 200)));

	SimulatorSystem.PostUpdate(int(roundf(dt * 200)));
}
