#include "PBSkyDefines.hlsl"

struct cFragIn
{
	float3 pos : POSITION0;
	float3 uv : TEXCOORD0;
};

// Number of intervals for the numerical integration
static const int SAMPLE_COUNT = 50;

sampler2D transmittanceTexture;

void computeSingleScatteringIntegrand(AtmosphereParameters atmosphere,
   float r, float mu, float mu_s, float nu, float d, 
   bool ray_r_mu_intersects_ground, out float3 rayleigh, out float3 mie)
{
	float r_d = clampRadius(atmosphere, sqrt(d*d + 2.0*r*mu*d + r*r));
	float mu_s_d = clampCosine((r*mu_s + d*nu) / r_d);
	float3 transmittance = getTransmittance(
	    atmosphere, transmittanceTexture, r, mu, d, ray_r_mu_intersects_ground) * 
	  getTransmittanceToSun(
	    atmosphere, transmittanceTexture, r_d, mu_s_d);
	
	rayleigh = transmittance * getProfileDensity(atmosphere.rayleighDensity, 
	               r_d - atmosphere.bottomRadius);
	mie = transmittance * getProfileDensity(atmosphere.mieDensity,
		       r_d - atmosphere.bottomRadius);
};

void getRMuMuSNuFromScatteringTextureUvwz(AtmosphereParameters atmosphere,
  float4 uvwz, out float r, out float mu, out float mu_s, out float nu, 
  out bool ray_r_mu_intersects_ground)
{
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	float H = sqrt(atmosphere.topRadius*atmosphere.topRadius - 
	  atmosphere.bottomRadius*atmosphere.bottomRadius);	
	// Disntace to the horizon
	float rho = H * getUnitRangeFromTexCoord(uvwz.w, 
	  atmosphere.scatteringTextureSizeR);
	r = sqrt(rho*rho + atmosphere.bottomRadius*atmosphere.bottomRadius);
	
	if (uvwz.z < 0.5) {
		// Distance to the ground for the ray (r,mu), and its minimum and maximum
		// values over all mu - obtained for (r,-1) and (r,mu_horizon) - from which
		// we can recover mu:
		float d_min = r - atmosphere.bottomRadius;
		float d_max = rho;
		float d = d_min + (d_max - d_min) * getUnitRangeFromTexCoord(
		  1.0 - 2.0*uvwz.z, atmosphere.scatteringTextureSizeMu / 2);
		
		mu = d == 0.0 ? -1.0 : clampCosine(-(rho*rho + d*d) / (2.0*r*d));
		ray_r_mu_intersects_ground = true;
	}
	else {
		// Distance to the top atmosphere boundary for the ray (r,mu), and its
		// minimum and maximum values over all mu - obtained for (r,1) and
		// (r,mu_horizon) - from which we can recover mu:
		float d_min = atmosphere.topRadius - r;
		float d_max = rho + H;
		float d = d_min + (d_max - d_min) * getUnitRangeFromTexCoord(
		  2.0 * uvwz.z - 1.0, atmosphere.scatteringTextureSizeMu / 2);
		
		mu = d == 0.0 ? 1.0 : clampCosine((H*H - rho*rho - d*d) / (2.0*r*d));
		ray_r_mu_intersects_ground = false;
	}
	
	float x_mu_s = getUnitRangeFromTexCoord(uvwz.y, atmosphere.scatteringTextureSizeMuS);
	float d_min = atmosphere.topRadius - atmosphere.bottomRadius;
	float d_max = H;
	float A = -2.0 * atmosphere.mu_s_min * atmosphere.bottomRadius / (d_max - d_min);
	float a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
	float d = d_min + min(a, A) * (d_max - d_min);
	
	mu_s = d == 0.0 ? 1.0 : clampCosine((H*H - d*d) / (2.0 * atmosphere.bottomRadius * d));
	nu = clampCosine(uvwz.x * 2.0 - 1.0);
}

void getRMuMuSNuFromScatteringTextureFragCoord(AtmosphereParameters atmosphere,
  float3 frag_coord, out float r, out float mu, out float mu_s, out float nu,
  out bool ray_r_mu_intersects_ground)
{
	const float4 SCATTERING_TEXTURE_SIZE = float4(
	  atmosphere.scatteringTextureSizeNu - 1,
	  atmosphere.scatteringTextureSizeMuS,
	  atmosphere.scatteringTextureSizeMu,
	  atmosphere.scatteringTextureSizeR);
	
	float frag_coord_nu = floor(frag_coord.x / SCATTERING_TEXTURE_SIZE.y);
	float frag_coord_mu_s = frag_coord.x % SCATTERING_TEXTURE_SIZE.y;
	float4 uvwz = float4(frag_coord_nu, frag_coord_mu_s, frag_coord.y, 
	  frag_coord.z) / SCATTERING_TEXTURE_SIZE;
	
	getRMuMuSNuFromScatteringTextureUvwz(atmosphere, uvwz, r, mu, mu_s,
	  nu, ray_r_mu_intersects_ground);
	// Clamp nu to its valid range of values, given mu and mu_s
	nu = clamp(nu, mu*mu_s - sqrt((1.0-mu*mu) * (1.0-mu_s*mu_s)),
	  mu*mu_s + sqrt((1.0-mu*mu) * (1.0-mu_s*mu_s)));
}

// r in [bottomRadius, topRadius]
// mu in [-1, 1]
// mu_s in [-1, 1]
// nu in [-1, 1]
void computeSingleScattering(AtmosphereParameters atmosphere, 
   float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground,
   out float3 rayleigh, out float3 mie)
{
	float dx = distanceToNearestAtmosphereBoundary(atmosphere,
	   r, mu, ray_r_mu_intersects_ground) / SAMPLE_COUNT;
	float3 rayleigh_sum = 0.0;
	float3 mie_sum = 0.0;
	
	for (int i = 0; i <= SAMPLE_COUNT; ++i) {
		float d_i = i*dx;
		float3 rayleigh_i;
		float3 mie_i;
		computeSingleScatteringIntegrand(atmosphere, 
		  r, mu, mu_s, nu, d_i,
		  ray_r_mu_intersects_ground, rayleigh_i, mie_i);
		
		float weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		rayleigh_sum += rayleigh_i * weight_i;
		mie_sum += mie_i * weight_i;
	}
	rayleigh = rayleigh_sum * dx * atmosphere.solarIrradiance * 
	  atmosphere.rayleighScattering;
	mie = mie_sum * dx * atmosphere.solarIrradiance * 
	  atmosphere.mieScattering;
}

extern uniform AtmosphereParameters customParams;

float4 main(float4 pos : POSITION0, float4 fragCoord : VPOS) : COLOR0
{
	float3 frag_coord = float3(fragCoord.x, fragCoord.y, customParams.volumeTextureLevel + 0.5);

	float r, mu, mu_s, nu;
	bool ray_r_mu_intersects_ground;
	getRMuMuSNuFromScatteringTextureFragCoord(customParams, frag_coord, 
	  r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	
	float3 rayleigh;
	float3 mie;
	computeSingleScattering(customParams, 
	  r, mu, mu_s, nu, ray_r_mu_intersects_ground, rayleigh, mie);
//	return float4(rayleigh, 1.0);
	//return float4(luminanceFromRadiance * rayleigh, (luminanceFromRadiance * mie).x);
	return float4(rayleigh, mie.x);
};
