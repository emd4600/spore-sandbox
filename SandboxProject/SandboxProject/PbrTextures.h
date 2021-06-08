#pragma once

#include <Spore\BasicIncludes.h>

struct PbrTextures
{
	TexturePtr albedoRoughnessMap;
	TexturePtr normalMap;
	TexturePtr metallicMap;
	TexturePtr aoMap;

	PbrTextures();

	bool Load(App::PropertyList* propList, uint32_t propertyID);

	void PrepareMaterial(Graphics::Material* material);
};