#include "PBSkyDefines.hlsl"

struct cFragIn
{
	float3 pos : POSITION0;
	float2 uv : TEXCOORD0;
};

// Number of intervals for the numerical integration
static const int SAMPLE_COUNT = 250;

// r = radius, [bottomRadius, topRadius]
// mu = cosine of zenith angle, [-1, 1]
float computeOpticalLengthToTopAtmosphereBoundary(
   AtmosphereParameters atmosphere, DensityProfile profile, float r, float mu) 
{
	// We use the trapezoid rule for integration
	// The length of each integration interval
	float dx = distanceToTopAtmosphereBoundary(atmosphere, r, mu) / SAMPLE_COUNT;
	float result = 0.0;
	for (int i = 0; i <= SAMPLE_COUNT; ++i)
	{
		float di = i*dx;
		// distance between current sample point and the planet center
		float ri = sqrt(di*di + 2.0*r*mu*di + r*r);
		
		// density at the current sample point divided by the number density
		// at the bottom of the atmosphere
		float yi = getProfileDensity(profile, ri - atmosphere.bottomRadius);
		// sample weight (trapezoidal rule)
		float weighti = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		result += yi * weighti * dx;
	}
	return result;	
};

// r = radius, [bottomRadius, topRadius]
// mu = cosine of zenith angle, [-1, 1]
float3 computeTransmittanceToTopAtmosphereBoundary(
   AtmosphereParameters atmosphere, float r, float mu)
{
	return exp(-(
	   atmosphere.rayleighScattering * 
		computeOpticalLengthToTopAtmosphereBoundary(
			atmosphere, atmosphere.rayleighDensity, r, mu) +
	   atmosphere.mieExtinction * 
		computeOpticalLengthToTopAtmosphereBoundary(
			atmosphere, atmosphere.mieDensity, r, mu) + 
	   atmosphere.absorptionExtinction * 
		computeOpticalLengthToTopAtmosphereBoundary(
			atmosphere, atmosphere.absorptionDensity, r, mu))
	);
};

//TODO this is based on Eric Bruneton implementation, which uses GLSL
// does the UV coordinate system change affect anything?
void getRMuFromTransmittanceTextureUv(
   AtmosphereParameters atmosphere, float2 uv, out float r, out float mu)
{
	float x_mu = getUnitRangeFromTexCoord(uv.x, atmosphere.transmittanceTextureWidth);
	float x_r = getUnitRangeFromTexCoord(uv.y, atmosphere.transmittanceTextureHeight);
	
	// Distance to top atmosphere boundary for a horizontal ray at ground level
	float H = sqrt(atmosphere.topRadius*atmosphere.topRadius - atmosphere.bottomRadius*atmosphere.bottomRadius);
	
	// Distnace to the horizon, from which we can compute r
	float rho = H * x_r;
	r = sqrt(rho*rho + atmosphere.bottomRadius*atmosphere.bottomRadius);
	
	// Distance to the top atmosphere boundary for the ray (r,mu) and its
	// minumum and maximum values over all mu - obtained for (r,1) and 
	// (r, mu_horizon) - from which we can recover mu
	float d_min = atmosphere.topRadius - r;
	float d_max = rho + H;
	float d = d_min + x_mu * (d_max - d_min);
	mu = d == 0.0 ? 1.0 : (H*H - rho*rho - d*d) / (2.0*r*d);
	mu = clampCosine(mu);
};


extern uniform AtmosphereParameters customParams;

float4 main(float4 pos : POSITION0, float4 fragCoord : VPOS) : COLOR0
{
	float2 textureSize = 
	  float2(customParams.transmittanceTextureWidth, customParams.transmittanceTextureHeight);
	//float x_mu = getUnitRangeFromTexCoord(fragCoord.x / textureSize.x, customParams.transmittanceTextureWidth);
	//float x_r = getUnitRangeFromTexCoord(fragCoord.y / textureSize.y, customParams.transmittanceTextureHeight);
	//return float4(x_mu, x_r, 0.0, 1.0);

	float r, mu;
	getRMuFromTransmittanceTextureUv(customParams, fragCoord / textureSize, r, mu);
	return float4(computeTransmittanceToTopAtmosphereBoundary(customParams, r, mu), 1.0);
};