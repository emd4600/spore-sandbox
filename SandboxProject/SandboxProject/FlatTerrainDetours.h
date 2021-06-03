#pragma once

#include <Spore\BasicIncludes.h>
#include <Spore\Anim\AnimatedCreature.h>
#include "FlatTerrain.h"

namespace Terrain
{
	member_detour(IsInWater__detour, Simulator::cPlanetModel, bool(const Vector3&))
	{
		bool detoured(const Vector3 & position)
		{
			if (FlatTerrain::IsFlatTerrain) return false;
			else return original_function(this, position);
		}
	};

	member_detour(ToSurface__detour, Simulator::cPlanetModel, Vector3* (Vector3&, const Vector3&))
	{
		Vector3* detoured(Vector3 & dst, const Vector3 & position)
		{
			if (FlatTerrain::IsFlatTerrain) {
				dst = { position.x, position.y, 0 };
			}
			else {
				original_function(this, dst, position);
			}
			return &dst;
		}
	};

	member_detour(GetHeightAt__detour, Simulator::cPlanetModel, float(const Vector3&))
	{
		float detoured(const Vector3 & position) 
		{
			if (FlatTerrain::IsFlatTerrain) return position.z;
			else return original_function(this, position);
		}
	};


	static_detour(AnimWorldCallback__detour, float(void*, Anim::AnimatedCreature*, const Quaternion&, Vector3*, bool, bool))
	{
		float detoured(void* object, Anim::AnimatedCreature* creature, const Quaternion& q, Vector3* dstNormal, bool arg_10, bool arg_14)
		{
			if (FlatTerrain::IsFlatTerrain)
			{
				if (dstNormal) {
					*dstNormal = { 0, 0, 1.0f };
				}
				return 0;
			}
			else return original_function(object, creature, q, dstNormal, arg_10, arg_14);
		}
	};

	// constructor at sub_AF46A0
	struct UnkAnimStruct
	{
		/* 00h */	int field_0;
		/* 04h */	int field_4;
		/* 08h */	int field_8;
		/* 0Ch */	int field_C;
		/* 10h */	int field_10;
		/* 14h */	int field_14;
		/* 18h */	int field_18;
		/* 1Ch */	int field_1C;
		/* 20h */	Simulator::cPlanetModel* pPlanetModel;
		/* 24h */	int field_24;
		/* 28h */	Vector3 field_28;
		/* 34h */	Vector3 field_34;
		/* 40h */	bool field_40;
		/* 44h */	float field_44;
		/* 48h */	Vector3 field_48;
		/* 54h */	float field_54;
		/* 58h */	BoundingBox field_60;  // MAX_FLT, MIN_FLT
		/* 70h */	Transform field_70;
		/* A8h */	char padding_A8[0xCC - 0xA8];
		/* CCh */	Vector3 field_CC;
		/* D8h */	Vector3 field_D8;
		/* E4h */	Vector3 field_E4;
	};

	static_detour(UnkAnim__detour, bool(Vector3&, Vector3&, float, int, float, float, UnkAnimStruct&, float*, bool))
	{
		// a1 is creature position
		bool detoured(Vector3& a1, Vector3& a2, float a3, int a4, float a5, float a6, UnkAnimStruct& dst, float* a8, bool a9)
		{
			/*if (*a8 < 1.52587890625E-5) return false;
			// also checks planet radius ...

			float height = PlanetModel.GetHeightAt(a1);*/

			App::ConsolePrintF("(%f, %f, %f) (%f, %f, %f) %f %d %f %f %f %d", 
				a1.x, a1.y, a1.z, a2.x, a2.y, a2.z, a3, a4, a5, a6, *a8, a9);

			bool result = original_function(a1, a2, a3, a4, a5, a6, dst, a8, a9);

			App::ConsolePrintF("  (%f, %f, %f) (%f, %f, %f) (%f, %f, %f) (%f, %f, %f)", dst.field_28.x, dst.field_28.y, dst.field_28.z,
				dst.field_CC.x, dst.field_CC.y, dst.field_CC.z, dst.field_D8.x, dst.field_D8.y, dst.field_D8.z, 
				dst.field_E4.x, dst.field_E4.y, dst.field_E4.z);

			//dst.field_28 = { 2, 2, 0 };
			dst.field_28 = { 3, 0, 1 };
			dst.field_48 = { 3, 0, 1 };
			/*dst.field_CC = { 3, 0.3, 1 };
			dst.field_D8 = { 3, -0.3, 1 };
			dst.field_E4 = { 3, 0, 1 };*/

			return result;
		}
	};
}