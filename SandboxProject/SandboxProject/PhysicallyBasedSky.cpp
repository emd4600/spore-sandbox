#include "stdafx.h"
#include "PhysicallyBasedSky.h"
#include <Spore\Graphics\GeneratedMesh.h>
#include <d3dx9tex.h>
#include <d3dx9math.h>

const Vector3 PhysicallyBasedSky::kColorLambdas = { 680.0, 555.0, 440.0 };

PhysicallyBasedSky::PhysicallyBasedSky()
	: mpTransmittanceLUT()
	, mpSingleScatteringLUT()
	, mShaderParams()
	, mpPropList()
	, mForceNoOzone(false)
{
}

PhysicallyBasedSky::~PhysicallyBasedSky()
{
}

// For internal use, do not modify.
int PhysicallyBasedSky::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int PhysicallyBasedSky::Release()
{
	return DefaultRefCounted::Release();
}


void PhysicallyBasedSky::Render(int flags, int layerIndex, App::cViewer** pViewers, void*) 
{
	/*PrecomputeTransmittance(false);

	intrusive_ptr<Graphics::GeneratedMesh<Graphics::PosUvVertex>> plane = Graphics::GenerateScreenPlane();
	int materialIndex = plane->AddMaterial(id("TextureTestMaterial"));
	plane->SetTexture(materialIndex, 0, mpTransmittanceLUT);
	plane->Render();*/

	pViewers[0]->LoadTransformations();

	D3DMATRIX viewMatrix;
	Graphics::Renderer::GetDevice()->GetTransform(D3DTS_VIEW, &viewMatrix);
	if (!D3DXMatrixInverse((D3DXMATRIX*)&mShaderParams.invViewMatrix, NULL, (D3DXMATRIX*)&viewMatrix)) {
		App::ConsolePrintF("ERROR: Cannot invert view matrix.");
	}

	D3DMATRIX projMatrix;
	Graphics::Renderer::GetDevice()->GetTransform(D3DTS_PROJECTION, &projMatrix);
	if (!D3DXMatrixInverse((D3DXMATRIX*)&mShaderParams.invProjMatrix, NULL, (D3DXMATRIX*)&projMatrix)) {
		App::ConsolePrintF("ERROR: Cannot invert projection matrix.");
	}

	mShaderParams.exposure = 10;
	mShaderParams.cameraPos = {0, 0, 0};
	//mShaderParams.sunDirection = Vector3(0, 1.0, 2.0).Normalized();
	mShaderParams.earthCenter = { 0, 0, -mShaderParams.atmosphere.bottomRadius };

	// customParams
	Graphics::Renderer::SetShaderData(0x206, &mShaderParams);

	intrusive_ptr<Graphics::GeneratedMesh<Graphics::PosUvVertex>> plane = Graphics::GenerateScreenPlane(1.0f);
	int materialIndex = plane->AddMaterial(id("SkyRaytrace"));
	plane->SetTexture(materialIndex, 0, mpSingleScatteringLUT);
	plane->SetTexture(materialIndex, 1, mpTransmittanceLUT);

	plane->Render();
}

float Interpolate(const vector<float>& wavelengths, const vector<float>& wavelengthFunction, float wavelength)
{
	assert(wavelengths.size() == wavelengthFunction.size());

	if (wavelength <= wavelengths[0]) return wavelengthFunction[0];
	for (unsigned int i = 0; i < wavelengths.size() - 1; ++i)
	{
		if (wavelength < wavelengths[i + 1]) {
			float u = (wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
			return wavelengthFunction[i] * (1.0f - u) + wavelengthFunction[i + 1] * u;
		}
	}
	return wavelengthFunction[1];
}

Vector3 Interpolate(const vector<float>& wavelengths, const vector<float>& v,
	const Vector3& lambdas, float scale) {
	float r = Interpolate(wavelengths, v, lambdas.x) * scale;
	float g = Interpolate(wavelengths, v, lambdas.y) * scale;
	float b = Interpolate(wavelengths, v, lambdas.z) * scale;
	return { r, g, b };
}

bool PhysicallyBasedSky::Load(uint32_t instanceID, uint32_t groupID)
{
	if (!PropManager.GetPropertyList(instanceID, groupID, mpPropList)) return false;
	ParseProp();
	return true;
}

void PhysicallyBasedSky::ParseProp()
{
	AtmosphereParameters& mAtmosphere = mShaderParams.atmosphere;

	float lengthUnitInMeters = 1000.0;
	App::Property::GetFloat(mpPropList.get(), id("atmosphereLengthUnitInMeters"), lengthUnitInMeters);

	int lambdaMin = 360;
	int lambdaMax = 830;
	App::Property::GetInt32(mpPropList.get(), id("atmosphereLambdaMin"), lambdaMin);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereLambdaMax"), lambdaMax);

	App::Property::GetFloat(mpPropList.get(), id("atmosphereBottomRadius"), mAtmosphere.bottomRadius);
	App::Property::GetFloat(mpPropList.get(), id("atmosphereTopRadius"), mAtmosphere.topRadius);
	mAtmosphere.bottomRadius = mAtmosphere.bottomRadius / lengthUnitInMeters;
	mAtmosphere.topRadius = mAtmosphere.topRadius / lengthUnitInMeters;

	float rayleigh = 1.24062e-6f;
	App::Property::GetFloat(mpPropList.get(), id("atmosphereRayleigh"), rayleigh);

	float* ozoneCrossSection;
	size_t ozoneCrossSectionCount = 0;
	bool useOzone = false;
	if (App::Property::GetArrayFloat(mpPropList.get(), id("atmosphereOzoneCrossSection"), ozoneCrossSectionCount, ozoneCrossSection)
		&& ozoneCrossSectionCount > 0) {
		useOzone = true;
	}
	float maxOzoneNumberDensity = 300.0f * 2.687e20f / 15000.0f;
	App::Property::GetFloat(mpPropList.get(), id("atmosphereMaxOzoneNumberDensity"), maxOzoneNumberDensity);

	useOzone = useOzone && !mForceNoOzone;

	float miePhaseFunctionG = 0.8f;
	float mieSingleScatteringAlbedo = 0.9f;
	float mieAngstromBeta = 5.328e-3f / 1200.0f;
	App::Property::GetFloat(mpPropList.get(), id("atmosphereMieAngstromBeta"), mieAngstromBeta);
	App::Property::GetFloat(mpPropList.get(), id("atmosphereMieSingleScatteringAlbedo"), mieSingleScatteringAlbedo);
	App::Property::GetFloat(mpPropList.get(), id("atmosphereMiePhaseFunctionG"), miePhaseFunctionG);
	mAtmosphere.miePhaseFunctionG = miePhaseFunctionG;

	float constantSolarIrradiance = 1.5f;
	float maxSunZenithAngle = 102.0f / 180.0f * Math::PI;
	float* solarIrradianceValues;
	size_t solarIrradianceCount = 0;
	if (!App::Property::GetArrayFloat(mpPropList.get(), id("atmosphereSolarIrradiance"), solarIrradianceCount, solarIrradianceValues)
		|| solarIrradianceCount == 0) {
		App::Property::GetFloat(mpPropList.get(), id("atmosphereSolarIrradiance"), constantSolarIrradiance);
	}
	App::Property::GetFloat(mpPropList.get(), id("atmosphereSunAngularRadius"), mAtmosphere.sunAngularRadius);
	App::Property::GetFloat(mpPropList.get(), id("atmosphereMaxSunZenithAngle"), maxSunZenithAngle);
	mAtmosphere.mu_s_min = cosf(maxSunZenithAngle);

	float sunSizeFactor = 1.0;
	App::Property::GetFloat(mpPropList.get(), id("atmosphereSunSizeFactor"), sunSizeFactor);
	mShaderParams.sunSize = {
		tanf(mShaderParams.atmosphere.sunAngularRadius * sunSizeFactor),
		cosf(mShaderParams.atmosphere.sunAngularRadius * sunSizeFactor) };

	vector<float> wavelengths;
	vector<float> rayleighScattering;
	vector<float> solarIrradiance;
	vector<float> mieScattering;
	vector<float> mieExtinction;
	vector<float> absorptionExtinction;

	for (int l = lambdaMin; l <= lambdaMax; l += 10) 
	{
		float lambda = static_cast<float>(l) * 1e-3f;  // micrometers
		float mie = mieAngstromBeta;
		wavelengths.push_back(static_cast<float>(l));
		rayleighScattering.push_back(rayleigh * pow(lambda, -4));
		mieScattering.push_back(mie * mieSingleScatteringAlbedo);
		mieExtinction.push_back(mie);
		absorptionExtinction.push_back(useOzone ? (maxOzoneNumberDensity * ozoneCrossSection[(l - lambdaMin) / 10]) : 0.0f);

		if (solarIrradianceCount == 0) {
			solarIrradiance.push_back(constantSolarIrradiance);
		}
		else {
			solarIrradiance.push_back(solarIrradianceValues[(l - lambdaMin) / 10]);
		}
	}

	mAtmosphere.rayleighScattering = Interpolate(wavelengths, rayleighScattering, kColorLambdas, lengthUnitInMeters);
	mAtmosphere.solarIrradiance = Interpolate(wavelengths, solarIrradiance, kColorLambdas, 1.0);
	mAtmosphere.mieScattering = Interpolate(wavelengths, mieScattering, kColorLambdas, lengthUnitInMeters);
	mAtmosphere.mieExtinction = Interpolate(wavelengths, mieExtinction, kColorLambdas, lengthUnitInMeters);
	mAtmosphere.absorptionExtinction = Interpolate(wavelengths, absorptionExtinction, kColorLambdas, lengthUnitInMeters);

	ResourceKey* profileKeys;
	unsigned int profileCount;
	// They must be defined from top to bottom
	if (App::Property::GetArrayKey(mpPropList.get(), id("atmosphereRayleighDensityProfile"), profileCount, profileKeys))
	{
		profileCount = min(profileCount, (unsigned int)2);
		for (unsigned int i = 0; i < profileCount; ++i)
		{
			ParseDensityProfile(profileKeys[i], mAtmosphere.rayleighDensity.layers[1 - i], lengthUnitInMeters);
		}
	}

	// They must be defined from top to bottom
	if (App::Property::GetArrayKey(mpPropList.get(), id("atmosphereMieDensityProfile"), profileCount, profileKeys))
	{
		profileCount = min(profileCount, (unsigned int)2);
		for (unsigned int i = 0; i < profileCount; ++i)
		{
			ParseDensityProfile(profileKeys[i], mAtmosphere.mieDensity.layers[1 - i], lengthUnitInMeters);
		}
	}

	// They must be defined from top to bottom
	if (App::Property::GetArrayKey(mpPropList.get(), id("atmosphereOzoneDensityProfile"), profileCount, profileKeys))
	{
		profileCount = min(profileCount, (unsigned int)2);
		for (unsigned int i = 0; i < profileCount; ++i)
		{
			ParseDensityProfile(profileKeys[i], mAtmosphere.absorptionDensity.layers[1 - i], lengthUnitInMeters);
		}
	}
}

bool PhysicallyBasedSky::ParseDensityProfile(const ResourceKey& key, DensityProfileLayer& layer, float lengthUnitInMeters)
{
	PropertyListPtr rayleighProfile;
	if (PropManager.GetPropertyList(key.instanceID, key.groupID, rayleighProfile))
	{
		App::Property::GetFloat(rayleighProfile.get(), id("atmosphereProfileWidth"), layer.width);
		App::Property::GetFloat(rayleighProfile.get(), id("atmosphereProfileExpTerm"), layer.expTerm);
		App::Property::GetFloat(rayleighProfile.get(), id("atmosphereProfileExpScale"), layer.expScale);
		App::Property::GetFloat(rayleighProfile.get(), id("atmosphereProfileLinearTerm"), layer.linearTerm);
		App::Property::GetFloat(rayleighProfile.get(), id("atmosphereProfileConstantTerm"), layer.constantTerm);

		layer.width /= lengthUnitInMeters;
		layer.expScale *= lengthUnitInMeters;
		layer.linearTerm *= lengthUnitInMeters;
		return true;
	}
	return false;
}

void PhysicallyBasedSky::PrecomputeTransmittance(bool saveTexture)
{
	AtmosphereParameters& mAtmosphere = mShaderParams.atmosphere;

	IDirect3DSurface9* oldRenderTarget;
	Graphics::Renderer::GetDevice()->GetRenderTarget(0, &oldRenderTarget);

	Clock clock(Clock::Mode::Milliseconds);
	clock.Start();

	int transmittanceTextureWidth, transmittanceTextureHeight;
	App::Property::GetInt32(mpPropList.get(), id("atmosphereTransmittanceTextureWidth"), transmittanceTextureWidth);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereTransmittanceTextureHeight"), transmittanceTextureHeight);

	mAtmosphere.transmittanceTextureWidth = float(transmittanceTextureWidth);
	mAtmosphere.transmittanceTextureHeight = float(transmittanceTextureHeight);

	Raster* renderTarget = new Raster();
	Raster::CreateRaster(renderTarget,
		transmittanceTextureWidth, transmittanceTextureHeight,
		1, Raster::kTypeRenderTarget, D3DFORMAT::D3DFMT_A16B16G16R16F);

	intrusive_ptr<Graphics::GeneratedMesh<Graphics::PosUvVertex>> plane = Graphics::GenerateScreenPlane();
	plane->AddMaterial(id("PrecomputeTransmittance"));

	IDirect3DSurface9* renderSurface;
	renderTarget->pTexture->GetSurfaceLevel(0, &renderSurface);
	Graphics::Renderer::GetDevice()->SetRenderTarget(0, renderSurface);
	Graphics::Renderer::GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);

	// customParams
	Graphics::Renderer::SetShaderData(0x206, &mAtmosphere);
	plane->Render();
	
	if (mpTransmittanceLUT) {
		delete mpTransmittanceLUT;
	}
	mpTransmittanceLUT = new Raster();
	Raster::CreateRaster(mpTransmittanceLUT,
		transmittanceTextureWidth, transmittanceTextureHeight,
		1, Raster::kTypeRenderTarget, D3DFORMAT::D3DFMT_A16B16G16R16F);

	/*mpTransmittanceLUT = new Raster();
	mpTransmittanceLUT->width = transmittanceTextureWidth;
	mpTransmittanceLUT->height = transmittanceTextureHeight;
	mpTransmittanceLUT->format = D3DFORMAT::D3DFMT_A16B16G16R16F;
	mpTransmittanceLUT->levels = 1;
	mpTransmittanceLUT->flags = Raster::kTypeTexture;
	mpTransmittanceLUT->depth = 64 / 8;
	mpTransmittanceLUT->CreateTexture(0, D3DPOOL_SYSTEMMEM);*/

	IDirect3DSurface9* textureSurface;
	mpTransmittanceLUT->pTexture->GetSurfaceLevel(0, &textureSurface);
	/*Graphics::Renderer::GetDevice()->GetRenderTargetData(renderSurface, textureSurface);*/
	Graphics::Renderer::GetDevice()->StretchRect(renderSurface, NULL, textureSurface, NULL, D3DTEXTUREFILTERTYPE::D3DTEXF_NONE);

	delete renderTarget;

	clock.Pause();
	App::ConsolePrintF("Transmittance LUT Precomputed in %f ms", clock.GetElapsed());

	if (saveTexture) {
		D3DXSaveTextureToFileA("E:\\Eric\\SporeModder FX 2.0.0\\Projects\\SandboxMode\\AtmosphericTextures\\transmittanceLUT.dds",
			D3DXIFF_DDS, mpTransmittanceLUT->pTexture, nullptr);
	}

	Graphics::Renderer::GetDevice()->SetRenderTarget(0, oldRenderTarget);

	if (renderSurface) renderSurface->Release();

	/*

	mpTransmittanceLUT = new Raster();
	mpTransmittanceLUT->width = transmittanceTextureWidth;
	mpTransmittanceLUT->height = transmittanceTextureHeight;
	mpTransmittanceLUT->format = D3DFORMAT::D3DFMT_A8R8G8B8;
	mpTransmittanceLUT->levels = 1;
	mpTransmittanceLUT->flags = Raster::kTypeTexture | Raster::kFlagDynamicUsage;
	mpTransmittanceLUT->depth = 64;
	mpTransmittanceLUT->CreateTexture(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);

	int bufferSize = transmittanceTextureWidth * transmittanceTextureHeight * 4;
	uint8_t* buffer = new uint8_t[bufferSize];
	for (int i = 0; i < bufferSize; ++i) {
		buffer[i] = 0xFF;
	}

	D3DLOCKED_RECT rect;
	mpTransmittanceLUT->pTexture->LockRect(0, &rect, NULL, 0);
	memcpy_s(rect.pBits, bufferSize, buffer, bufferSize);
	mpTransmittanceLUT->pTexture->UnlockRect(0);*/
}

/*void PhysicallyBasedSky::PrecomputeSingleScattering()
{
	AtmosphereParameters& mAtmosphere = mShaderParams.atmosphere;

	IDirect3DSurface9* oldRenderTarget;
	Graphics::Renderer::GetDevice()->GetRenderTarget(0, &oldRenderTarget);

	if (mpSingleScatteringLUT) {
		delete mpSingleScatteringLUT;
	}

	Clock clock(Clock::Mode::Milliseconds);
	clock.Start();

	int scatteringTextureSizeR, scatteringTextureSizeMuS, scatteringTextureSizeMu, scatteringTextureSizeNu;
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeR"), scatteringTextureSizeR);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeMuS"), scatteringTextureSizeMuS);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeMu"), scatteringTextureSizeMu);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeNu"), scatteringTextureSizeNu);

	mAtmosphere.scatteringTextureSizeR = float(scatteringTextureSizeR);
	mAtmosphere.scatteringTextureSizeMuS = float(scatteringTextureSizeMuS);
	mAtmosphere.scatteringTextureSizeMu = float(scatteringTextureSizeMu);
	mAtmosphere.scatteringTextureSizeNu = float(scatteringTextureSizeNu);

	int width = scatteringTextureSizeNu * scatteringTextureSizeMuS;
	int height = scatteringTextureSizeMu * scatteringTextureSizeR;

	Raster* renderTarget = new Raster();
	Raster::CreateRaster(renderTarget, width, height,
		1, Raster::kTypeRenderTarget, D3DFORMAT::D3DFMT_A16B16G16R16F);

	intrusive_ptr<Graphics::GeneratedMesh<Graphics::PosUvVertex>> plane = Graphics::GenerateScreenPlane();
	plane->AddMaterial(id("PrecomputeSingleScattering"));

	IDirect3DSurface9* renderSurface;
	renderTarget->pTexture->GetSurfaceLevel(0, &renderSurface);
	Graphics::Renderer::GetDevice()->SetRenderTarget(0, renderSurface);
	Graphics::Renderer::GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);

	// customParams
	Graphics::Renderer::SetShaderData(0x206, &mAtmosphere);
	plane->Render();

	mpSingleScatteringLUT = new Raster();
	Raster::CreateRaster(mpSingleScatteringLUT, width, height,
		1, Raster::kTypeTexture, D3DFORMAT::D3DFMT_A16B16G16R16F);

	IDirect3DSurface9* textureSurface;
	mpSingleScatteringLUT->pTexture->GetSurfaceLevel(0, &textureSurface);
	Graphics::Renderer::GetDevice()->StretchRect(renderSurface, NULL, textureSurface, NULL, D3DTEXTUREFILTERTYPE::D3DTEXF_NONE);

	clock.Pause();
	App::ConsolePrintF("Single Scattering LUT Precomputed in %f ms", clock.GetElapsed());
}*/

void PhysicallyBasedSky::PrecomputeSingleScattering()
{
	AtmosphereParameters& mAtmosphere = mShaderParams.atmosphere;

	IDirect3DSurface9* oldRenderTarget;
	Graphics::Renderer::GetDevice()->GetRenderTarget(0, &oldRenderTarget);

	if (mpSingleScatteringLUT) {
		delete mpSingleScatteringLUT;
	}

	Clock clock(Clock::Mode::Milliseconds);
	clock.Start();

	int scatteringTextureSizeR, scatteringTextureSizeMuS, scatteringTextureSizeMu, scatteringTextureSizeNu;
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeR"), scatteringTextureSizeR);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeMuS"), scatteringTextureSizeMuS);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeMu"), scatteringTextureSizeMu);
	App::Property::GetInt32(mpPropList.get(), id("atmosphereScatteringTextureSizeNu"), scatteringTextureSizeNu);

	mAtmosphere.scatteringTextureSizeR = float(scatteringTextureSizeR);
	mAtmosphere.scatteringTextureSizeMuS = float(scatteringTextureSizeMuS);
	mAtmosphere.scatteringTextureSizeMu = float(scatteringTextureSizeMu);
	mAtmosphere.scatteringTextureSizeNu = float(scatteringTextureSizeNu);

	int width = scatteringTextureSizeNu * scatteringTextureSizeMuS;
	int height = scatteringTextureSizeMu;
	int depth = scatteringTextureSizeR;

	mpSingleScatteringLUT = new Raster();
	mpSingleScatteringLUT->width = width;
	mpSingleScatteringLUT->height = height;
	mpSingleScatteringLUT->volumeDepth = depth;
	mpSingleScatteringLUT->format = D3DFORMAT::D3DFMT_A16B16G16R16F;
	mpSingleScatteringLUT->levels = 1;
	mpSingleScatteringLUT->flags = Raster::kTypeTexture | Raster::kFlagVolumeTexture | Raster::kFlagDynamicUsage;
	mpSingleScatteringLUT->depth = 64;

	Graphics::Renderer::GetDevice()->CreateVolumeTexture(width, height, depth, 1, D3DUSAGE_DYNAMIC, D3DFMT_A16B16G16R16F,
		D3DPOOL_DEFAULT, &mpSingleScatteringLUT->pVolumeTexture, NULL);
	/*IDirect3DVolume9* test;
	mpSingleScatteringLUT->pVolumeTexture->GetVolumeLevel(0, &test);
	D3DXLoadVolumeFromFileA(test, NULL, NULL, "E:\\Eric\\SporeModder FX 2.0.0\\Projects\\SandboxMode\\AtmosphericTextures\\singleScatteringLUT.dds", NULL, 0, 0, 0);

	D3DXCreateTextureFromFileA(Graphics::Renderer::GetDevice(), 
		"E:\\Eric\\SporeModder FX 2.0.0\\Projects\\SandboxMode\\AtmosphericTextures\\singleScatteringLUT.dds", 
		&mpSingleScatteringLUT->pTexture);

	mpSingleScatteringLUT = new Raster();
	mpSingleScatteringLUT->width = width;
	mpSingleScatteringLUT->height = height;
	mpSingleScatteringLUT->volumeDepth = depth;
	mpSingleScatteringLUT->format = D3DFORMAT::D3DFMT_A8R8G8B8;
	mpSingleScatteringLUT->levels = 1;
	mpSingleScatteringLUT->flags = Raster::kTypeTexture | Raster::kFlagVolumeTexture | Raster::kFlagDynamicUsage;
	mpSingleScatteringLUT->depth = 64;

	Graphics::Renderer::GetDevice()->CreateVolumeTexture(width, height, depth, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT, &mpSingleScatteringLUT->pVolumeTexture, NULL);

	IDirect3DVolumeTexture9* volumeTexture;
	Graphics::Renderer::GetDevice()->CreateVolumeTexture(width, height, depth, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM, &volumeTexture, NULL);*/

	/*int bufferSize = width * height * depth * 4;
	uint8_t* buffer = new uint8_t[bufferSize];
	for (int i = 0; i < bufferSize; ++i) {
		buffer[i] = 0x80;
	}

	D3DLOCKED_BOX rect;
	mpSingleScatteringLUT->pVolumeTexture->LockBox(0, &rect, NULL, 0);
	memcpy_s(rect.pBits, bufferSize, buffer, bufferSize);
	mpSingleScatteringLUT->pVolumeTexture->UnlockBox(0);

	//Graphics::Renderer::GetDevice()->UpdateTexture(volumeTexture, mpSingleScatteringLUT->pVolumeTexture);
	return;*/


	Raster* tempRenderTarget = new Raster();
	Raster::CreateRaster(tempRenderTarget, width, height,
		1, Raster::kTypeRenderTarget, D3DFORMAT::D3DFMT_A16B16G16R16F);

	Raster* tempTexture = new Raster();
	tempTexture->width = width;
	tempTexture->height = height;
	tempTexture->format = D3DFORMAT::D3DFMT_A16B16G16R16F;
	tempTexture->levels = 1;
	tempTexture->flags = Raster::kTypeTexture;
	tempTexture->depth = 64;
	tempTexture->CreateTexture(0, D3DPOOL_SYSTEMMEM);

	IDirect3DSurface9* renderSurface;
	IDirect3DSurface9* tempSurface;
	tempRenderTarget->pTexture->GetSurfaceLevel(0, &renderSurface);
	tempTexture->pTexture->GetSurfaceLevel(0, &tempSurface);
	Graphics::Renderer::GetDevice()->SetRenderTarget(0, renderSurface);


	intrusive_ptr<Graphics::GeneratedMesh<Graphics::PosUvVertex>> plane = Graphics::GenerateScreenPlane();
	int materialIndex = plane->AddMaterial(id("PrecomputeSingleScattering"));
	plane->SetTexture(materialIndex, 0, mpTransmittanceLUT);
	// customParams
	Graphics::Renderer::SetShaderData(0x206, &mAtmosphere);

	int volumeStride = mpSingleScatteringLUT->depth / 8 * width * height;
	char* volumeBuffer = new char[volumeStride * depth];

	for (int i = 0; i < depth; ++i)
	{
		mAtmosphere.volumeTextureLevel = float(i);
		Graphics::Renderer::GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
		plane->Render();

		Graphics::Renderer::GetDevice()->GetRenderTargetData(renderSurface, tempSurface);

		D3DLOCKED_RECT rect;
		tempTexture->pTexture->LockRect(0, &rect, nullptr, D3DLOCK_READONLY);
		memcpy_s(volumeBuffer + volumeStride * i, volumeStride, rect.pBits, volumeStride);
		tempTexture->pTexture->UnlockRect(0);
	}

	D3DLOCKED_BOX lockedVolume;
	mpSingleScatteringLUT->pVolumeTexture->LockBox(0, &lockedVolume, NULL, D3DLOCK_DISCARD);
	memcpy_s(lockedVolume.pBits, volumeStride * depth, volumeBuffer, volumeStride * depth);
	mpSingleScatteringLUT->pVolumeTexture->UnlockBox(0);

	delete[] volumeBuffer;
	delete tempRenderTarget;
	delete tempTexture;

	clock.Pause();
	App::ConsolePrintF("Single Scattering LUT Precomputed in %f ms", clock.GetElapsed());

	IDirect3DVolume9* volume;
	mpSingleScatteringLUT->pVolumeTexture->GetVolumeLevel(0, &volume);
	D3DXSaveVolumeToFileA("E:\\Eric\\SporeModder FX 2.0.0\\Projects\\SandboxMode\\AtmosphericTextures\\singleScatteringLUT.dds",
		D3DXIFF_DDS, volume, nullptr, nullptr);

	Graphics::Renderer::GetDevice()->SetRenderTarget(0, oldRenderTarget);
}


void PhysicallyBasedSky::Precompute()
{
	ParseProp();
	PrecomputeTransmittance(false);
	PrecomputeSingleScattering();
}

void PhysicallyBasedSky::SetSun(float zenithAngle, float azimuthAngle)
{
	mShaderParams.sunDirection = {
		cosf(azimuthAngle) * sinf(zenithAngle),
		sinf(azimuthAngle) * sinf(zenithAngle),
		cosf(zenithAngle)
	};
}

void PhysicallyBasedSky::SetForceNoOzone(bool value)
{
	mForceNoOzone = value;
}

bool PhysicallyBasedSky::GetForceNoOzone() const {
	return mForceNoOzone;
}