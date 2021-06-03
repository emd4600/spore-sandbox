#include "stdafx.h"
#include "FlatTerrain.h"
#include "FlatTerrainDetours.h"

namespace Terrain
{
	bool FlatTerrain::IsFlatTerrain = false;

	void FlatTerrain::AttachDetours()
	{
		IsInWater__detour::attach(GetAddress(Simulator::cPlanetModel, IsInWater));
		ToSurface__detour::attach(GetAddress(Simulator::cPlanetModel, ToSurface));
		GetHeightAt__detour::attach(GetAddress(Simulator::cPlanetModel, GetHeightAt));

		AnimWorldCallback__detour::attach(Address(0xB341F0));

		UnkAnim__detour::attach(Address(0xAF3750));
	}

	FlatTerrain::FlatTerrain()
		: mpTerrainMapSet()
		, mpTerrainStateMgr()
		, mGrassTrampling()
	{
		mpTerrainMapSet = new cTerrainMapSet();

		auto heightMap = new cTerrainMap16();
		heightMap->size = 512;
		heightMap->data.resize(heightMap->size * heightMap->size * 6);

		auto normalMap = new cTerrainMap32();
		normalMap->size = 512;
		normalMap->data.resize(heightMap->size * heightMap->size * 6);

		mpTerrainMapSet->SetMap(TerrainMapIndex::HeightMap, heightMap);
		mpTerrainMapSet->SetMap(TerrainMapIndex::NormalMap, normalMap);

		for (unsigned int i = 0; i < normalMap->data.size(); ++i)
		{
			normalMap->data[i] = 0x0000FF00;
		}

		Vector3 normal;
		CALL(Address(0xF930B0), void, Args(cTerrainMapSet*, Vector3&, const Vector3&), Args(mpTerrainMapSet.get(), normal, { 3, 3, 3 }));
		// GetNormal F930B0

		App::ConsolePrintF("%f, %f, %f", normal.x, normal.y, normal.z);
	}


	FlatTerrain::~FlatTerrain()
	{
		if (mpTerrainStateMgr) delete mpTerrainStateMgr;
	}

	// For internal use, do not modify.
	int FlatTerrain::AddRef()
	{
		return DefaultRefCounted::AddRef();
	}

	// For internal use, do not modify.
	int FlatTerrain::Release()
	{
		return DefaultRefCounted::Release();
	}


	App::PropertyList* FlatTerrain::FlatTerrain::GetPropertyList() { assert(false); return 0; }

	cTerrainMapSet* FlatTerrain::GetTerrainMapSet() 
	{ 
		return mpTerrainMapSet.get();
	}

	cTerrainStateMgr* FlatTerrain::GetTerrainStateManager() 
	{ 
		return mpTerrainStateMgr;
	}


	cWeatherManager* FlatTerrain::GetWeatherManager() { assert(false); return 0; }
	uint32_t FlatTerrain::GetSeed() { assert(false); return 0; }
	void FlatTerrain::SetDisplayType(DisplayType pathType) { assert(false); }
	ITerrain::DisplayType FlatTerrain::GetDisplayType() { assert(false); return DisplayType::Default; }
	void FlatTerrain::func24h(Object*) { assert(false); }
	Object* FlatTerrain::func28h() { assert(false); return 0; }
	bool FlatTerrain::HasViewer() { assert(false); return 0; }
	bool FlatTerrain::IsLoading() { assert(false); return 0; }
	int FlatTerrain::GetLoadMode() { assert(false); return 0; }
	void FlatTerrain::SetOnLoadFinish(OnLoadFinish_t f, void* object) { assert(false); }
	void FlatTerrain::ForceLoadEnd() { assert(false); }
	void FlatTerrain::func40h() { assert(false); }
	void FlatTerrain::func44h(bool) { assert(false); }
	uint8_t FlatTerrain::func48h(uint8_t) { assert(false); return 0; }
	bool FlatTerrain::func4Ch(bool) { assert(false); return 0; }
	bool FlatTerrain::func50h() { assert(false); return 0; }
	uint32_t FlatTerrain::AddModelModification(const Transform& transform, App::PropertyList* propList) { assert(false); return 0; }
	bool FlatTerrain::RemoveModelModification(uint32_t modid, bool) { assert(false); return 0; }
	bool FlatTerrain::HasModelModification(uint32_t modid) { assert(false); return 0; }
	int FlatTerrain::ResetModelModifications() { assert(false); return 0; }
	uint32_t FlatTerrain::AddPlayerModification(const Transform& transform, uint32_t effectID, uint32_t groupID) { assert(false); return 0; }
	bool FlatTerrain::RemovePlayerModification(uint32_t modid) { assert(false); return 0; }
	bool FlatTerrain::HasPlayerModification(uint32_t modid) { assert(false); return 0; }
	void FlatTerrain::ResetPlayerModifications() { assert(false); }  // also removes certain properties
	void FlatTerrain::SetPlanetInfo(int terrainType, int atmosphereType, int waterType) { assert(false); }
	void FlatTerrain::GetPlanetInfo(int* dstTerrainType, int* dstAtmosphereType, int* dstWaterType) { assert(false); }
	void FlatTerrain::func7Ch(int terrainType, int atmosphereType, int waterType, Vector3*, Vector3*, Vector3*) { assert(false); }  // related with colors or maps
	void FlatTerrain::func80h() { assert(false); }  // related with player effects
	void FlatTerrain::func84h() { assert(false); }
	void FlatTerrain::func88h() { assert(false); }
	bool FlatTerrain::AddToRender(Graphics::IRenderManager*) { assert(false); return 0; }
	void FlatTerrain::RemoveFromRender(Graphics::IRenderManager*) { assert(false); }
	bool FlatTerrain::IsAddedToRender(Graphics::IRenderManager*) { assert(false); return 0; }
	void FlatTerrain::SetVisible(bool visible) { assert(false); }

	void FlatTerrain::Update(App::cViewer* pViewer, int deltaTime) 
	{

	}

	void FlatTerrain::funcA0h() { assert(false); }  //PLACEHOLDER adds effect surfaces?
	void FlatTerrain::funcA4h() { assert(false); }  // Unloads effect surfaces?
	void FlatTerrain::LoadAssets() { assert(false); }
	void FlatTerrain::UnloadAssets() { assert(false); }
	size_t FlatTerrain::GetModels(ModelPtr*& dst) { assert(false); return 0; }
	size_t FlatTerrain::GetModelsInfo(ResourceKey** dstKeys, Transform** dstTransforms) { assert(false); return 0; }
	int FlatTerrain::funcB8h(int) { assert(false); return 0; }
	int FlatTerrain::funcBCh(int) { assert(false); return 0; }
	int FlatTerrain::funcC0h(int) { assert(false); return 0; }
	int FlatTerrain::funcC4h(int) { assert(false); return 0; }
	size_t FlatTerrain::funcC8h() { assert(false); return 0; }
	int FlatTerrain::funcCCh() { assert(false); return 0; }
	int FlatTerrain::funcD0h() { assert(false); return 0; }
	void FlatTerrain::funcD4h(float) { assert(false); }
	bool FlatTerrain::funcD8h() { assert(false); return 0; }
	bool FlatTerrain::GetAllowUnderwaterObjects() { assert(false); return 0; }
	void FlatTerrain::SetAllowUnderwaterObjects(bool) { assert(false); }
	void FlatTerrain::AddUnderwaterModelWorld(Graphics::IModelWorld* world, int flags) { assert(false); }
	void FlatTerrain::RemoveUnderwaterModelWorld(Graphics::IModelWorld*) { assert(false); }
	void FlatTerrain::ClearUnderwaterModelWorlds() { assert(false); }
	void FlatTerrain::AddUnderwaterAnimWorld(Anim::IAnimWorld* world, int flags) { assert(false); }
	void FlatTerrain::RemoveUnderwaterAnimWorld(Anim::IAnimWorld*) { assert(false); }
	void FlatTerrain::ClearUnderwaterAnimWorlds() { assert(false); }

	Vector4* FlatTerrain::GetGrassTrampling() 
	{
		return mGrassTrampling;
	}
	int FlatTerrain::GetMaxGrassTrampling()
	{
		return 8; 
	}
	void FlatTerrain::ResetGrassTrampling() 
	{
		memset(&mGrassTrampling, 0, sizeof(mGrassTrampling));
	}

	Vector3 FlatTerrain::Raycast(const Vector3& pos, const Vector3& dir) 
	{ 
		// For now, just return intersection with plane z=0
		if (pos.z == 0) return pos;
		if ((pos.z < 0 && dir.z < 0) || (pos.z > 0 && dir.z > 0)) return Vector3::ZERO;

		Vector3 unitDir = dir.Normalized();
		if (pos.z < 0) {
			return pos + unitDir * (-pos.z);
		}
		else {
			return pos + unitDir * pos.z;
		}
	}

	Quaternion FlatTerrain::GetOrientation(const Vector3& upAxis) {
		return Quaternion();
	}

	void FlatTerrain::Dispose() { assert(false); }
	void FlatTerrain::ParseProp(App::PropertyList* pPropList) { assert(false); }
	const Vector3& FlatTerrain::GetCameraPos() { assert(false); return {}; }
	const Vector3& FlatTerrain::GetCameraDir() { assert(false); return {}; }
	const Vector3& FlatTerrain::GetSunDir() { assert(false); return {}; }
	bool FlatTerrain::LoadInternal(bool weather) { assert(false); return 0; }

	bool FlatTerrain::Load() 
	{
		mpTerrainStateMgr = new cTerrainStateMgr(this);
		CALL(Address(0xFC0BA0), void, Args(cTerrainStateMgr*, bool), Args(mpTerrainStateMgr, false));

		IsFlatTerrain = true;

		return true;
	}
}