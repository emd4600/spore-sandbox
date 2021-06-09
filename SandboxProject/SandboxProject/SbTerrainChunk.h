#pragma once

#include <Spore\BasicIncludes.h>
#include <Spore\Graphics\GeneratedMesh.h>

#define SbTerrainChunkPtr intrusive_ptr<SbTerrainChunk>

class SbTerrainSystem;

/// Each of the individual tiles in which the terrain is divided.
/// This represents a square with a side of a few meters, with evenly distributed vertices.
class SbTerrainChunk 
	: public Object
	, public DefaultRefCounted
{
public:
	static const uint32_t TYPE = id("SbTerrainChunk");
	
	SbTerrainChunk(SbTerrainSystem* pTerrain, int tileX, int tileY);
	~SbTerrainChunk();

	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;

	void Generate();
	void Render();

private:
	typedef Graphics::StandardVertexCompact Vertex;

	SbTerrainSystem* mpTerrain;
	GeneratedMeshPtr(Vertex) mMesh;
	Matrix4 mTransform;
	int mTileX;
	int mTileY;
};
