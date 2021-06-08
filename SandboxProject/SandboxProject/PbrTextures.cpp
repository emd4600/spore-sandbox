#include "stdafx.h"
#include "PbrTextures.h"

PbrTextures::PbrTextures()
	: albedoRoughnessMap()
	, normalMap()
	, metallicMap()
	, aoMap()
{
}

bool PbrTextures::Load(App::PropertyList* propList, uint32_t propertyID)
{
	ResourceKey* dst;
	size_t dstCount;
	if (App::Property::GetArrayKey(propList, propertyID, dstCount, dst) && dstCount == 4) {
		albedoRoughnessMap = TextureManager.GetTexture(dst[0], Graphics::ITextureManager::kForceLoad);
		normalMap = TextureManager.GetTexture(dst[1], Graphics::ITextureManager::kForceLoad);
		metallicMap = TextureManager.GetTexture(dst[2], Graphics::ITextureManager::kForceLoad);
		aoMap = TextureManager.GetTexture(dst[3], Graphics::ITextureManager::kForceLoad);
		return albedoRoughnessMap && normalMap && metallicMap && aoMap;
	}
	return false;
}

void PbrTextures::PrepareMaterial(Graphics::Material* material)
{
	material->states[0]->SetRaster(0, albedoRoughnessMap->GetRaster());
	material->states[0]->SetRaster(1, normalMap->GetRaster());
	material->states[0]->SetRaster(2, metallicMap->GetRaster());
	material->states[0]->SetRaster(3, aoMap->GetRaster());
}