#include "stdafx.h"
#include "SbTerrainChunk.h"
#include "SbTerrainSystem.h"
#include <EASTL\unique_ptr.h>

SbTerrainChunk::SbTerrainChunk(SbTerrainSystem* pTerrain, int tileX, int tileY)
	: mpTerrain(pTerrain)
	, mTileX(tileX)
	, mTileY(tileY)
{
}


SbTerrainChunk::~SbTerrainChunk()
{
}

// For internal use, do not modify.
int SbTerrainChunk::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int SbTerrainChunk::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* SbTerrainChunk::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(SbTerrainChunk);
	return nullptr;
}


void SbTerrainChunk::Generate() {
	const int resolution = mpTerrain->chunkResolution;
	const float size = mpTerrain->chunkSize;
	const float spacing = size / float(resolution);
	const int rowVerts = resolution + 1;

	//TODO generate using triangle strips, more efficient
	mMesh = new Graphics::GeneratedMesh<Vertex>(rowVerts*rowVerts, resolution*resolution*2);

	auto normals = eastl::make_unique<Vector3[]>(mMesh->GetVertexCount());
	auto tangents = eastl::make_unique<Vector3[]>(mMesh->GetVertexCount());

	int vertexIndex = 0;
	for (int row = 0; row <= resolution; ++row) {
		for (int col = 0; col <= resolution; ++col) {
			Vertex vertex;
			vertex.pos = { col * spacing, row * spacing, 0.0f };
			vertex.uv = { float(col) / resolution * mpTerrain->uvScale, float(row) / resolution * mpTerrain->uvScale };
			normals[vertexIndex] = Math::Z_AXIS;
			tangents[vertexIndex] = { 0, 0, 0 };
			mMesh->SetVertex(vertexIndex, vertex);
			++vertexIndex;
		}
	}

	int triangleIndex = 0;
	for (int row = 0; row < resolution; ++row) {
		for (int col = 0; col < resolution; ++col) {
			mMesh->SetTriangle(triangleIndex, 
				row * rowVerts + col + 1,
				(row + 1) * rowVerts + col,
				row * rowVerts + col);
			mMesh->SetTriangle(triangleIndex + 1,
				row * rowVerts + col + 1,
				(row + 1) * rowVerts + col + 1,
				(row + 1) * rowVerts + col);
			triangleIndex += 2;
		}
	}

	// We calculate tangents, probably not the most efficient way
	for (int tindex = 0; tindex < mMesh->GetTriangleCount(); ++tindex) {
		int i, j, k;
		mMesh->GetTriangle(tindex, i, j, k);

		auto v0_co = mMesh->GetVertex(i).pos;
		auto v1_co = mMesh->GetVertex(j).pos;
		auto v2_co = mMesh->GetVertex(k).pos;

		auto v0_uv = mMesh->GetVertex(i).uv;
		auto v1_uv = mMesh->GetVertex(j).uv;
		auto v2_uv = mMesh->GetVertex(k).uv;

		auto dco1 = v1_co - v0_co;
		auto dco2 = v2_co - v0_co;
		auto duv1 = v1_uv - v0_uv;
		auto duv2 = v2_uv - v0_uv;
		auto tangent = dco2 * duv1.y - dco1 * duv2.y;
		auto bitangent = dco2 * duv1.x - dco1 * duv2.x;
		if (dco2.Cross(dco1).Dot(bitangent.Cross(tangent)) < 0) {
			tangent = -tangent;
			bitangent = -bitangent;
		}
		tangents[i] += tangent;
		tangents[j] += tangent;
		tangents[k] += tangent;
	}

	for (int i = 0; i < mMesh->GetVertexCount(); ++i) {
		Vector3 tangent = (tangents[i] - normals[i] * tangents[i].Dot(normals[i])).Normalized();
		mMesh->ModifyVertex(i).SetTangent(tangent);
		mMesh->ModifyVertex(i).SetNormal(normals[i]);
	}

	//TODO maybe delete the vertices and triangles vectors once we have done this, to use less memory
	mMesh->SubmitGeometry();

	mMesh->AddMaterial(mpTerrain->baseMaterial);
}

void SbTerrainChunk::Render() {
	auto material = mMesh->GetMaterial(0);
	mpTerrain->baseTextures.PrepareMaterial(material);
	mMesh->Render();
}
