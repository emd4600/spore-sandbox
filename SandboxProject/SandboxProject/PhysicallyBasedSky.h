#pragma once

#include <Spore\BasicIncludes.h>

#define PhysicallyBasedSkyPtr intrusive_ptr<PhysicallyBasedSky>

class PhysicallyBasedSky 
	: public Graphics::IRenderable
	, public DefaultRefCounted
{
public:
	PhysicallyBasedSky();
	~PhysicallyBasedSky();

	int AddRef() override;
	int Release() override;
	void Render(int flags, int layerIndex, App::cViewer**, void*) override;

	bool Load(uint32_t instanceID, uint32_t groupID);

	void Precompute();

	void SetSun(float zenithAngle, float azimuthAngle);

	bool GetForceNoOzone() const;
	void SetForceNoOzone(bool value);

protected:
	struct DensityProfileLayer {
		float expTerm;
		float expScale;
		float linearTerm;
		float constantTerm;
		float width;
	private:
		int padding[3];
	};
	struct DensityProfile
	{
		DensityProfileLayer layers[2];
	};
	struct AtmosphereParameters
	{
		// The solar irradiance at the top of the atmosphere.
		Vector3 solarIrradiance; int padding1[1];

		// The sun's angular radius. Warning: the implementation uses approximations
		// that are valid only if this angle is smaller than 0.1 radians.
		float sunAngularRadius; int padding2[3];

		// The cosine of the maximum Sun zenith angle for which atmospheric scattering
		// must be precomputed (for maximum precision, use the smallest Sun zenith
		// angle yielding negligible sky light radiance values. For instance, for the
		// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
		float mu_s_min; int padding3[3];

		// The scattering coefficient of air molecules at the altitude where their
		// density is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
		Vector3 rayleighScattering; int padding4[1];

		// The scattering coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'mie_scattering' times 'mie_density' at this altitude.
		Vector3 mieScattering; int padding5[1];

		// The extinction coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The extinction coefficient at altitude h is equal to
		// 'mie_extinction' times 'mie_density' at this altitude.
		Vector3 mieExtinction; int padding6[1];

		// The extinction coefficient of molecules that absorb light (e.g. ozone) at
		// the altitude where their density is maximum, as a function of wavelength.
		// The extinction coefficient at altitude h is equal to
		// 'absorption_extinction' times 'absorption_density' at this altitude.
		Vector3 absorptionExtinction; int padding7[1];

		// The asymetry parameter for the Cornette-Shanks phase function for the
		// aerosols.
		float miePhaseFunctionG; int padding8[3];

		DensityProfile rayleighDensity;
		DensityProfile mieDensity;
		DensityProfile absorptionDensity;

		// distance between planet center and planet surface
		float bottomRadius; int padding9[3];

		// distance between planet center and top atmosphere boundary
		float topRadius; int padding10[3];

		float transmittanceTextureWidth; int padding11[3];
		float transmittanceTextureHeight; int padding12[3];
		float scatteringTextureSizeNu; int padding13[3]; 
		float scatteringTextureSizeMuS; int padding14[3];
		float scatteringTextureSizeMu; int padding15[3];
		float scatteringTextureSizeR; int padding16[3];

		// Used for precomputing 3D textures
		float volumeTextureLevel; int padding17[3];
	};

	struct InputParameters
	{
		AtmosphereParameters atmosphere;
		Matrix4 invViewMatrix;
		Matrix4 invProjMatrix;
		Vector3 cameraPos; int padding1[1];
		Vector3 earthCenter; int padding2[1];
		Vector3 sunDirection; int padding3[1];
		Vector2 sunSize; int padding4[2];
		float exposure; int padding5[3];
	};

	// The wavelengths for red, green and blue colors, in nanometers
	static const Vector3 kColorLambdas;

	void ParseProp();
	void PrecomputeTransmittance(bool saveTexture);
	void PrecomputeSingleScattering();
	bool ParseDensityProfile(const ResourceKey& key, DensityProfileLayer& layer, float lengthUnitInMeters);

	PropertyListPtr mpPropList;
	InputParameters mShaderParams;
	bool mForceNoOzone;

	RenderWare::Raster* mpTransmittanceLUT;
	RenderWare::Raster* mpSingleScatteringLUT;

	//TODO eventually, change ambient light in lighting world
};
