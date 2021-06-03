struct DensityProfileLayer {
	// expTerm, expScale, linearTerm, constantTerm
	float4 terms;
	float width;
};
struct DensityProfile
{
	DensityProfileLayer layers[2];
};

struct AtmosphereParameters
{
	// The solar irradiance at the top of the atmosphere.
	float3 solarIrradiance;
	// The sun's angular radius. Warning: the implementation uses approximations
	// that are valid only if this angle is smaller than 0.1 radians.
	float sunAngularRadius;
	// The cosine of the maximum Sun zenith angle for which atmospheric scattering
  	// must be precomputed (for maximum precision, use the smallest Sun zenith
	// angle yielding negligible sky light radiance values. For instance, for the
	// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
	float mu_s_min;
	
	float3 rayleighScattering;
	float3 mieScattering;
	float3 mieExtinction;
	float3 absorptionExtinction;
	float miePhaseFunctionG;
	
	DensityProfile rayleighDensity;
	DensityProfile mieDensity;
	DensityProfile absorptionDensity;
	
	// distance between planet center and planet surface
	float bottomRadius;
	// distance between planet center and top atmosphere boundary
	float topRadius;
	
	float transmittanceTextureWidth;
	float transmittanceTextureHeight;
	float scatteringTextureSizeNu;
	float scatteringTextureSizeMuS;
	float scatteringTextureSizeMu;
	float scatteringTextureSizeR;
	
	// Used for precomputing 3D textures
	float volumeTextureLevel;
};

struct InputParameters
{
	AtmosphereParameters atmosphere;
	float4x4 viewToWorld;
	float4x4 clipToView;
	float3 cameraPos;
	float3 earthCenter;
	float3 sunDirection;
	float2 sunSize;
	float exposure;
};

static const float PI = 3.1415926535897932;


// safety functions, to avoid NaNs
float clampCosine(float mu) {
	return clamp(mu, -1.0, 1.0);
}
float clampPositive(float d) {
	return max(d, 0.0);
}
float clampRadius(AtmosphereParameters atmosphere, float r) {
	return clamp(r, atmosphere.bottomRadius, atmosphere.topRadius);
}
float safeSqrt(float a) {
	return sqrt(max(a, 0.0));
}


float getTexCoordFromUnitRange(float x, float textureSize) {
	return 0.5/textureSize + x*(1.0 - 1.0/textureSize);
};
float getUnitRangeFromTexCoord(float u, float textureSize) {
	return (u - 0.5/textureSize) / (1.0 - 1.0/textureSize);
};


float getLayerDensity(DensityProfileLayer layer, float altitude)
{
	float density = layer.terms.x * exp(layer.terms.y * altitude) +
		layer.terms.z * altitude + layer.terms.w;
	return clamp(density, 0.0, 1.0);
};

float getProfileDensity(DensityProfile profile, float altitude) {
	return altitude < profile.layers[0].width ? 
		getLayerDensity(profile.layers[0], altitude) : 
		getLayerDensity(profile.layers[1], altitude);
}


// r = radius, <= topRadius
// mu = cosine of zenith angle, [-1, 1]
float distanceToTopAtmosphereBoundary(AtmosphereParameters atmosphere, float r, float mu)
{
	float d = r*r*(mu*mu-1.0) + atmosphere.topRadius*atmosphere.topRadius;
	return clampPositive(-r*mu + safeSqrt(d));
};

// r = radius, >= bottomRadius
// mu = cosine of zenith angle, [-1, 1]
float distanceToBottomAtmosphereBoundary(AtmosphereParameters atmosphere, float r, float mu)
{
	float d = r*r*(mu*mu-1.0) + atmosphere.bottomRadius*atmosphere.bottomRadius;
	return clampPositive(-r*mu + safeSqrt(d));
};

float distanceToNearestAtmosphereBoundary(AtmosphereParameters atmosphere, float r, float mu, 
  bool ray_r_mu_intersects_ground)
{
	if (ray_r_mu_intersects_ground) {
		return distanceToBottomAtmosphereBoundary(atmosphere, r, mu);
	} else {
		return distanceToTopAtmosphereBoundary(atmosphere, r, mu);
	}
}

// r = radius, >= bottomRadius
// mu = cosine of zenith angle, [-1, 1]
float rayIntersectsGround(AtmosphereParameters atmosphere, float r, float mu)
{
	return mu < 0.0 && r*r*(mu*mu - 1.0) + 
	  atmosphere.bottomRadius*atmosphere.bottomRadius >= 0.0;
};


// TRANSMITTANCE

// r in [bottomRadius, topRadius]
// mu in [-1, 1]
float2 getTransmittanceTextureUvFromRMu(AtmosphereParameters atmosphere,
  float r, float mu)
{
	// Distance to top atmosphere bnoundary for a horizontal ray at ground level
	float H = sqrt(atmosphere.topRadius*atmosphere.topRadius -
	   atmosphere.bottomRadius*atmosphere.bottomRadius);
	// Distance to the horizon
	float rho = safeSqrt(r*r - atmosphere.bottomRadius*atmosphere.bottomRadius);
	// Distance to the top atmosphere boundary for the ray (r,mu) and its
	// min and max values over all mu - obtained for (r,1) and (r,mu_horizon)
	float d = distanceToTopAtmosphereBoundary(atmosphere, r, mu);
	float d_min = atmosphere.topRadius - r;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;
	return float2(
	   getTexCoordFromUnitRange(x_mu, atmosphere.transmittanceTextureWidth),
	   getTexCoordFromUnitRange(x_r, atmosphere.transmittanceTextureHeight));
}

float3 getTransmittanceToTopAtmosphereBoundary(AtmosphereParameters atmosphere,
  sampler2D transmittanceTexture, float r, float mu)
{
	float2 uv = getTransmittanceTextureUvFromRMu(atmosphere, r, mu);
	return tex2D(transmittanceTexture, uv).xyz;
}

// r in [bottomRAdius, topRadius]
// mu in [-1, 1]
// d >= 0 
float3 getTransmittance(AtmosphereParameters atmosphere, 
  sampler2D transmittanceTexture, float r, float mu, float d, 
  bool ray_r_mu_intersects_ground)
{
	float r_d = clampRadius(atmosphere, sqrt(d*d + 2.0*r*mu*d + r*r));	
	float mu_d = clampCosine((r*mu + d) / r_d);
	
	if (ray_r_mu_intersects_ground) {
		return min(getTransmittanceToTopAtmosphereBoundary(
		  atmosphere, transmittanceTexture, r_d, -mu_d) / 
		getTransmittanceToTopAtmosphereBoundary(
		  atmosphere, transmittanceTexture, r, -mu), 1.0);
	}
	else {
		return min(getTransmittanceToTopAtmosphereBoundary(
		  atmosphere, transmittanceTexture, r, mu) / 
		getTransmittanceToTopAtmosphereBoundary(
		  atmosphere, transmittanceTexture, r_d, mu_d), 1.0);
	}
}

float3 getTransmittanceToSun(AtmosphereParameters atmosphere, 
  sampler2D transmittanceTexture, float r, float mu_s)
{
	float sin_theta_h = atmosphere.bottomRadius / r;
	float cos_theta_h = -sqrt(max(1.0 - sin_theta_h*sin_theta_h, 0.0));
	return getTransmittanceToTopAtmosphereBoundary(atmosphere,
	     transmittanceTexture, r, mu_s) * 
	  smoothstep(-sin_theta_h * atmosphere.sunAngularRadius,
	             sin_theta_h * atmosphere.sunAngularRadius,
 		     mu_s - cos_theta_h);
}
