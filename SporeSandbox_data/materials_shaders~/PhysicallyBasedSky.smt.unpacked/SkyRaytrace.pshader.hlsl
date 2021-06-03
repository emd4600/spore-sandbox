#include "PBSkyDefines.hlsl"

struct cFragIn
{
	float3 pos : POSITION0;
	float3 viewRay : NORMAL;
};

float rayleighPhaseFunction(float nu) 
{
	float k = 3.0 / (16.0 * PI);
	return k * (1.0 + nu*nu);
}

float miePhaseFunction(float g, float nu)
{
	float k = 3.0 / (8.0 * PI) * (1.0 - g*g) / (2.0 + g*g);
	return k * (1.0 + nu*nu) / pow(1.0 + g*g - 2.0*g*nu, 1.5);
}

// r in [bottomRadius, topRadius]
// mu in [-1, 1]
// mu_s in [-1, 1]
// nu in [-1, 1]
float4 getScatteringTextureUvwzFromRMuMuSNu(AtmosphereParameters atmosphere,
  float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground)
{
	// Distance to top atmosphere boundary for a horizontal ray at ground level
	float H = sqrt(atmosphere.topRadius*atmosphere.topRadius - 
	   atmosphere.bottomRadius*atmosphere.bottomRadius);
	// Distance to the horizon
	float rho = safeSqrt(r*r - atmosphere.bottomRadius*atmosphere.bottomRadius);
	float u_r = getTexCoordFromUnitRange(rho / H, 
	  atmosphere.scatteringTextureSizeR);
	
	// Discriminant of the quadratic equation for the intersections of the ray
	// (r, mu) with the ground
	float r_mu = r * mu;
	float discriminant = r_mu*r_mu - r*r + 
	  atmosphere.bottomRadius*atmosphere.bottomRadius;
	float u_mu;
	if (ray_r_mu_intersects_ground) {
		// Distance to the ground for the ray (r,mu) and its min and max
		// values over all mu - obtained for (r,-1) and (r,mu_horizon)
		float d = -r_mu - safeSqrt(discriminant);
		float d_min = r - atmosphere.bottomRadius;
		float d_max = rho;
		u_mu = 0.5 - 0.5 * getTexCoordFromUnitRange(d_max == d_min ? 0.0 :
		  (d - d_min) / (d_max - d_min), atmosphere.scatteringTextureSizeMu / 2);
	}
	else {
		// Distance to the top atmosphere boundary for the ray (r,mu), and its
		// minimum and maximum values over all mu - obtained for (r,1) and
		// (r,mu_horizon).
		float d = -r_mu + safeSqrt(discriminant + H*H);
		float d_min = atmosphere.topRadius - r;
		float d_max = rho + H;
		u_mu = 0.5 + 0.5 * getTexCoordFromUnitRange(
		  (d - d_min) / (d_max - d_min), atmosphere.scatteringTextureSizeMu / 2);
	}
	
	float d = distanceToTopAtmosphereBoundary(atmosphere,
	  atmosphere.bottomRadius, mu_s);
	float d_min = atmosphere.topRadius - atmosphere.bottomRadius;
	float d_max = H;
	float a = (d - d_min) / (d_max - d_min);
	float A = -2.0 * atmosphere.mu_s_min * atmosphere.bottomRadius / (d_max - d_min);
	float u_mu_s = getTexCoordFromUnitRange(max(1.0-a/A, 0.0) / (1.0+a),
	   atmosphere.scatteringTextureSizeMuS);
	
	float u_nu = (nu + 1.0) / 2.0;
	return float4(u_nu, u_mu_s, u_mu, u_r);
}

float3 getExtrapolatedSingleMieScattering(AtmosphereParameters atmosphere, float4 scattering)
{
	if (scattering.r == 0.0) {
		return 0.0;
	}
	return scattering.rgb * scattering.a / scattering.r *
	    (atmosphere.rayleighScattering.r / atmosphere.mieScattering.r) *
	    (atmosphere.mieScattering / atmosphere.rayleighScattering);
}

float3 getCombinedScattering(AtmosphereParameters atmosphere,
  sampler3D scatteringTexture, float r, float mu, float mu_s, float nu,
  bool ray_r_mu_intersects_ground, out float3 singleMieScattering)
{
	float4 uvwz = getScatteringTextureUvwzFromRMuMuSNu(
	  atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	
	float tex_coord_x = uvwz.x * (atmosphere.scatteringTextureSizeNu - 1);
	float tex_x = floor(tex_coord_x);
	float lerp = tex_coord_x - tex_x;
	float3 uvw0 = float3((tex_x + uvwz.y) / atmosphere.scatteringTextureSizeNu,
	   uvwz.z, uvwz.w);
	float3 uvw1 = float3((tex_x + 1.0 + uvwz.y) / atmosphere.scatteringTextureSizeNu,
	   uvwz.z, uvwz.w);
	
	float4 combined_scattering = tex3D(scatteringTexture, uvw0) * (1.0 - lerp) + 
			    	     tex3D(scatteringTexture, uvw1) * lerp;
	singleMieScattering = getExtrapolatedSingleMieScattering(atmosphere, combined_scattering);
	
	return combined_scattering.xyz;
}

float3 getSkyRadiance(AtmosphereParameters atmosphere,
  sampler2D transmittanceTexture, sampler3D scatteringTexture,
  float3 camera, float3 viewRay, float shadowLength, float3 sunDirection,
  out float3 transmittance)
{
	// Compute the distance to the top atmosphere boundary along the view ray,
	// assuming the viewer is in space (or NaN if the view ray does not intersect
	// the atmosphere).	
	float r = length(camera);
	float rmu = dot(camera, viewRay);
	float distanceToTopAtmosphereBoundary = -rmu -
	  sqrt(rmu*rmu - r*r + atmosphere.topRadius*atmosphere.topRadius);
	// If the viewer is in space and the view ray intersects the atmosphere,
	// move the viewer to the top atmosphere boundary (along the view ray)
	if (distanceToTopAtmosphereBoundary > 0.0)
	{
		camera = camera + viewRay*distanceToTopAtmosphereBoundary;
		r = atmosphere.topRadius;
		rmu += distanceToTopAtmosphereBoundary;
	}
	else if (r > atmosphere.topRadius) {
		// If the view ray does not intersect the atmosphere, return 0
		transmittance = 1.0;
		return 0.0;
	}
	// Compute the r, mu, mu_s, nu parameters needed for texture lookups
	float mu = rmu / r;
	float mu_s = dot(camera, sunDirection) / r;
	float nu = dot(viewRay, sunDirection);
	bool ray_r_mu_intersects_ground = rayIntersectsGround(atmosphere, r, mu);
	if (ray_r_mu_intersects_ground) {
		discard;
	}
	
	transmittance = ray_r_mu_intersects_ground ? 0.0 : 
	  getTransmittanceToTopAtmosphereBoundary(atmosphere, transmittanceTexture, r, mu);
	float3 scattering;
	float3 singleMieScattering;
	//if (shadowLength == 0.0)
	//{
		scattering = getCombinedScattering(atmosphere, scatteringTexture,
		  r, mu, mu_s, nu, ray_r_mu_intersects_ground, singleMieScattering);
	//}
	//else //TODO light shafts
	
	//float4 uvwz = getScatteringTextureUvwzFromRMuMuSNu(
	//  atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	//scattering = tex3D(scatteringTexture, float3(uvwz.x, uvwz.z, 0.5));
	//return scattering; // * rayleighPhaseFunction(nu);
	//return uvwz.w;
	//return ray_r_mu_intersects_ground ? 0.0 : 1.0;
	
	return scattering * rayleighPhaseFunction(nu) + 
	  singleMieScattering * miePhaseFunction(atmosphere.miePhaseFunctionG, nu);
	//return scattering * rayleighPhaseFunction(nu);
}

float3 getSolarRadiance(AtmosphereParameters atmosphere) 
{
	return atmosphere.solarIrradiance / (PI * atmosphere.sunAngularRadius * atmosphere.sunAngularRadius);
}

extern uniform InputParameters customParams;
// Order is important: if transmittance texture is not used, the order would change
// and become an undetectable error...
sampler3D scatteringTexture;
sampler2D transmittanceTexture;

float4 main(cFragIn In) : COLOR0
{
	//float4 colortest = tex2D(transmittanceTexture, float2(0.5, 0.5));
	//return float4(tex3D(scatteringTexture, float3(0.5, 0.5, 0.5)).xyz, colortest.w);

	float3 viewDirection = normalize(In.viewRay);
	//return float4(normalize(In.viewRay), 1.0);
	
	float3 transmittance;
	float3 radiance = getSkyRadiance(customParams.atmosphere,
	  transmittanceTexture, scatteringTexture,
	  customParams.cameraPos - customParams.earthCenter, viewDirection, 0.0,
	  customParams.sunDirection, transmittance);
	
	// If the view ray intersects the Sun, add the Sun radiance
	if (dot(viewDirection, customParams.sunDirection) > customParams.sunSize.y) {
		radiance = radiance + transmittance * getSolarRadiance(customParams.atmosphere);
	}
	
	float3 color = pow(1.0 - exp(-radiance * customParams.exposure), 1.0 / 2.2);
	//float3 color = radiance;
	//float3 color = tex3D(scatteringTexture, float3(0.1, 0.1, 0.1)).xyz;
	return float4(color, 1.0);
}