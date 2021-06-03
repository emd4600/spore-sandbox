#pragma once

#include <Spore\BasicIncludes.h>
#include <Spore\Terrain\ITerrain.h>

#define FlatTerrainPtr intrusive_ptr<FlatTerrain>

namespace Terrain
{
	class FlatTerrain
		: public Terrain::ITerrain
		, public DefaultRefCounted
	{
	public:
		static const uint32_t TYPE = id("FlatTerrain");

		FlatTerrain();
		~FlatTerrain();

		int AddRef() override;
		int Release() override;

		/* 08h */	virtual App::PropertyList* GetPropertyList() override;
		/* 0Ch */	virtual cTerrainMapSet* GetTerrainMapSet() override;
		/* 10h */	virtual cTerrainStateMgr* GetTerrainStateManager() override;
		/* 14h */	virtual cWeatherManager* GetWeatherManager() override;
		/* 18h */	virtual uint32_t GetSeed() override;
		/* 1Ch */	virtual void SetDisplayType(DisplayType pathType) override;
		/* 20h */	virtual DisplayType GetDisplayType() override;
		/* 24h */	virtual void func24h(Object*) override;
		/* 28h */	virtual Object* func28h() override;
		/* 2Ch */	virtual bool HasViewer() override;
		/* 30h */	virtual bool IsLoading() override;
		/* 34h */	virtual int GetLoadMode() override;
		/* 38h */	virtual void SetOnLoadFinish(OnLoadFinish_t f, void* object) override;
		/* 3Ch */	virtual void ForceLoadEnd() override;
		/* 40h */	virtual void func40h() override;
		/* 44h */	virtual void func44h(bool) override;
		/* 48h */	virtual uint8_t func48h(uint8_t) override;
		/* 4Ch */	virtual bool func4Ch(bool) override;
		/* 50h */	virtual bool func50h() override;
		/* 54h */	virtual uint32_t AddModelModification(const Transform& transform, App::PropertyList* propList) override;
		/* 58h */	virtual bool RemoveModelModification(uint32_t modid, bool) override;
		/* 5Ch */	virtual bool HasModelModification(uint32_t modid) override;
		/* 60h */	virtual int ResetModelModifications() override;
		/* 64h */	virtual uint32_t AddPlayerModification(const Transform& transform, uint32_t effectID, uint32_t groupID = 0) override;
		/* 68h */	virtual bool RemovePlayerModification(uint32_t modid) override;
		/* 6Ch */	virtual bool HasPlayerModification(uint32_t modid) override;
		/* 70h */	virtual void ResetPlayerModifications() override;  // also removes certain properties
		/* 74h */	virtual void SetPlanetInfo(int terrainType, int atmosphereType, int waterType) override;
		/* 78h */	virtual void GetPlanetInfo(int* dstTerrainType, int* dstAtmosphereType, int* dstWaterType) override;
		/* 7Ch */	virtual void func7Ch(int terrainType, int atmosphereType, int waterType, Vector3*, Vector3*, Vector3*) override;  // related with colors or maps
		/* 80h */	virtual void func80h() override;  // related with player effects
		/* 84h */	virtual void func84h() override;
		/* 88h */	virtual void func88h() override;
		/* 8Ch */	virtual bool AddToRender(Graphics::IRenderManager*) override;
		/* 90h */	virtual void RemoveFromRender(Graphics::IRenderManager*) override;
		/* 94h */	virtual bool IsAddedToRender(Graphics::IRenderManager*) override;
		/* 98h */	virtual void SetVisible(bool visible) override;
		/* 9Ch */	virtual void Update(App::cViewer* pViewer, int deltaTime) override;
		/* A0h */	virtual void funcA0h() override;  //PLACEHOLDER adds effect surfaces?
		/* A4h */	virtual void funcA4h() override;  // Unloads effect surfaces?
		/* A8h */	virtual void LoadAssets() override;
		/* ACh */	virtual void UnloadAssets() override;
		/* B0h */	virtual size_t GetModels(ModelPtr*& dst) override;
		/* B4h */	virtual size_t GetModelsInfo(ResourceKey** dstKeys, Transform** dstTransforms) override;
		/* B8h */	virtual int funcB8h(int) override;
		/* BCh */	virtual int funcBCh(int) override;
		/* C0h */	virtual int funcC0h(int) override;
		/* C4h */	virtual int funcC4h(int) override;
		/* C8h */	virtual size_t funcC8h() override;
		/* CCh */	virtual int funcCCh() override;
		/* D0h */	virtual int funcD0h() override;
		/* D4h */	virtual void funcD4h(float) override;
		/* D8h */	virtual bool funcD8h() override;
		/* DCh */	virtual bool GetAllowUnderwaterObjects() override;
		/* E0h */	virtual void SetAllowUnderwaterObjects(bool) override;
		/* E4h */	virtual void AddUnderwaterModelWorld(Graphics::IModelWorld* world, int flags = 0) override;
		/* E8h */	virtual void RemoveUnderwaterModelWorld(Graphics::IModelWorld*) override;
		/* ECh */	virtual void ClearUnderwaterModelWorlds() override;
		/* F0h */	virtual void AddUnderwaterAnimWorld(Anim::IAnimWorld* world, int flags) override;
		/* F4h */	virtual void RemoveUnderwaterAnimWorld(Anim::IAnimWorld*) override;
		/* F8h */	virtual void ClearUnderwaterAnimWorlds() override;
		/* FCh */	virtual Vector4* GetGrassTrampling() override;
		/* 100h */	virtual int GetMaxGrassTrampling() override;  // returns 8
		/* 104h */	virtual void ResetGrassTrampling() override;
		/* 108h */	virtual Vector3 Raycast(const Vector3& pos, const Vector3& dir) override;
		/* 10Ch */	virtual Quaternion GetOrientation(const Vector3& upAxis) override;
		/* 110h */	virtual void Dispose() override;
		/* 114h */	virtual void ParseProp(App::PropertyList* pPropList) override;
		/* 118h */	virtual const Vector3& GetCameraPos() override;
		/* 11Ch */	virtual const Vector3& GetCameraDir() override;
		/* 120h */	virtual const Vector3& GetSunDir() override;
		/* 124h */	virtual bool LoadInternal(bool weather) override;
		/* 128h */	virtual bool Load() override;

		static bool IsFlatTerrain;

		static void AttachDetours();

	private:
		cTerrainMapSetPtr mpTerrainMapSet;
		cTerrainStateMgr* mpTerrainStateMgr;

		Vector4 mGrassTrampling[8];
	};

}
