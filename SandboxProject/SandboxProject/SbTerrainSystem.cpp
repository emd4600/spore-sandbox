#include "stdafx.h"
#include "SbTerrainSystem.h"

SbTerrainSystem::SbTerrainSystem()
	: mpLightingWorld()
	, mChunks()
{
	LoadConfiguration();
}


SbTerrainSystem::~SbTerrainSystem()
{
}

// For internal use, do not modify.
int SbTerrainSystem::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int SbTerrainSystem::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* SbTerrainSystem::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(SbTerrainSystem);
	return nullptr;
}



void SbTerrainSystem::LoadConfiguration() {
	PropertyListPtr propList;
	if (PropManager.GetPropertyList(0xae6209af, 0xc620ccc3, propList)) {
		chunkSize = 30.0;
		chunkResolution = 32;
		uvScale = 3.0;
		App::Property::GetFloat(propList.get(), id("sbTerrainChunkSize"), chunkSize);
		App::Property::GetInt32(propList.get(), id("sbTerrainChunkResolution"), chunkResolution);
		App::Property::GetFloat(propList.get(), id("sbTerrainUVScale"), uvScale);

		bool texturesLoaded = baseTextures.Load(propList.get(), id("sbTerrainTextures1"));

		App::Property::GetUInt32(propList.get(), id("sbTerrainMaterial"), baseMaterial);

		assert(texturesLoaded);
	}
}

void SbTerrainSystem::SetLightingWorld(Graphics::ILightingWorld* world) {
	mpLightingWorld = world;
}

void SbTerrainSystem::Generate() {
	//TODO

	for (int i = -5; i <= 5; ++i) {
		for (int j = -5; j <= 5; ++j) {
			SbTerrainChunkPtr chunk = new SbTerrainChunk(this, i, j);
			chunk->Generate();
			mChunks.push_back(chunk);
		}
	}
}

void SbTerrainSystem::Render(int flags, int layerIndex, App::cViewer** viewers, void*) {
	//TODO

	if (mpLightingWorld) mpLightingWorld->PrepareRender(viewers[0]);

	for (auto chunk : mChunks) {
		chunk->Render();
	}
}