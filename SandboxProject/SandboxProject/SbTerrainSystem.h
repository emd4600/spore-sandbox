#pragma once

#include <Spore\BasicIncludes.h>
#include "SbTerrainChunk.h"
#include "PbrTextures.h"

#define SbTerrainSystemPtr intrusive_ptr<SbTerrainSystem>

class SbTerrainSystem 
	: public Object
	, public DefaultRefCounted
	, public Graphics::IRenderable
{
public:
	static const uint32_t TYPE = id("SbTerrainSystem");
	
	SbTerrainSystem();
	~SbTerrainSystem();

	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;

	void Render(int flags, int layerIndex, App::cViewer**, void*) override;

	void SetLightingWorld(Graphics::ILightingWorld* world);
	void Generate();

	// Length of the sides of the chunk, in meters
	float chunkSize;
	// Number of triangles per side in a chunk
	int chunkResolution;
	// How many UV tiles are displayed per side of chunk
	float uvScale;
	uint32_t baseMaterial;
	PbrTextures baseTextures;

private:
	ILightingWorldPtr mpLightingWorld;
	vector<SbTerrainChunkPtr> mChunks;

	void LoadConfiguration();
};
