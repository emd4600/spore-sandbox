#include "stdafx.h"
#include "CreatureController.h"
#include <Spore\Anim\IAnimManager.h>

CreatureController::CreatureController(Anim::AnimatedCreature* creature, Graphics::IModelWorld* groundWorld)
	: mpAnimatedCreature(creature)
	, mpModelWorld(groundWorld)
	, mRealTargetPosition()
	, mTargetPosition()
	, mCurrentPosition()
	, mTargetAngle()
	, mCurrentAngle()
	, mMovementSpeed(2.5f)
	, mAngularSpeed(PI)
	, mTargetLookAt()
	, mCurrentLookAt()
	, mSmoothLookAt(true)
	, field_55()
{
}


CreatureController::~CreatureController()
{
}

// For internal use, do not modify.
int CreatureController::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int CreatureController::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* CreatureController::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(CreatureController);
	return nullptr;
}

Vector3 Vector2PointsMaxLength(const Vector3& p1, const Vector3& p2, float maxLength)
{
	Vector3 v = p2 - p1;
	if (maxLength * maxLength < v.SquaredLength()) {
		return p1 + v.Normalized() * maxLength;
	}
	else return p2;
}

void CreatureController::Update(float dt)
{
	if (!mpAnimatedCreature || !mpModelWorld) return;

	float var_74 = 1.0f;
	float speed = 1.0f;

	uint32_t animID;
	Anim::AnimIndex animIndex;
	if (mpAnimatedCreature->GetCurrentAnimation(&animID, nullptr, nullptr, &animIndex) 
		&& !AnimManager.GetAnimGroup(animID)->allowLocomotion) 
	{
		Vector3 dir = { 0.0f, -1.5f, 0.5f };
		mTargetLookAt = mpAnimatedCreature->mPosition + mpAnimatedCreature->mOrientation.ToMatrix() * dir;
		mCurrentLookAt = mTargetLookAt;
		var_74 = 0.0f;
		speed = 0.0f;
	}

	if (Anim::AnimatedCreature::IsIdleWalkLookatStart(animID)) 
	{
		float var_50;
		if (mpAnimatedCreature->func5Ch(animIndex, &var_74, &var_50))
		{
			speed = clamp(var_50 / var_74, 0.0f, 1.0f);
		}
		else {
			speed = 0.0f;
		}
	}
	else {
		speed = var_74;
	}

	mCurrentPosition = Vector2PointsMaxLength(mCurrentPosition, mTargetPosition,
		speed * mMovementSpeed * dt);
	// Here Spore would limit mCurrentPosition to a radius of 5

	mCurrentAngle = IncrementAngleTo(mCurrentAngle, mTargetAngle,
		speed * mAngularSpeed * dt);

	Vector3 targetLookAtDir = mTargetLookAt - mCurrentPosition;
	Vector3 v1 = targetLookAtDir;
	Vector3 currentLookAtDir = mCurrentLookAt - mCurrentPosition;
	if (targetLookAtDir != Vector3::ZERO) v1 = targetLookAtDir = targetLookAtDir.Normalized();
	if (currentLookAtDir != Vector3::ZERO) currentLookAtDir = currentLookAtDir.Normalized();

	if (mSmoothLookAt) {
		float angle = IncrementAngleTo(
			Vector3::OrientedAngle(currentLookAtDir, Y_AXIS, Z_AXIS),
			Vector3::OrientedAngle(targetLookAtDir, Y_AXIS, Z_AXIS),
			5.0f * dt);
		mCurrentLookAt += { 10.0f * cosf(angle), 10.0f * sinf(angle), 0.0f };
	}
	else {
		mCurrentLookAt += 10.0f * Vector2PointsMaxLength(currentLookAtDir, targetLookAtDir, 2.0f * dt);
	}

	if (field_55) {
		v1.z = 0.0f;
		float angle = -Vector3::OrientedAngle(v1, -Y_AXIS, Z_AXIS);
		angle = CorrectAngleRange(CorrectAngleRange(mCurrentAngle) - CorrectAngleRange(angle));
		if (abs(angle) > PI / 4) {
			float rotAngle = mCurrentAngle - (angle < 0 ? -1 : 1) * PI / 4;
			mCurrentLookAt += Matrix3::FromAxisAngle(Z_AXIS, rotAngle) * mpAnimatedCreature->mPosition;
		}
	}

	mpAnimatedCreature->mPosition = mCurrentPosition;
	mpAnimatedCreature->mOrientation = Quaternion::FromRotation(Z_AXIS, mCurrentAngle);

	if (Anim::AnimatedCreature::IsIdleWalk(animID) && 
		((bool*)mpAnimatedCreature->p_cid)[0x2D4] &&
		 !mSmoothLookAt)
	{
		Vector3 v = { mRealTargetPosition.x - mTargetPosition.x, mRealTargetPosition.y - mTargetPosition.y, 0.0f };
		if (v.Length() < 0.1f)
		{
			v = mpAnimatedCreature->mOrientation.ToMatrix() * v;
		}
		else {
			v = v.Normalized();
			static const float eps = 1e-8f;
			if (v.x < eps && v.y < eps && v.z < eps) {
				v = -Y_AXIS;
			}
		}

		mpAnimatedCreature->field_164 = lerp(mpAnimatedCreature->field_164, mCurrentPosition + 10.0f * v, 0.85f);
		if (abs(mpAnimatedCreature->field_164.z) > 0.000015258789) {
			mpAnimatedCreature->field_164.z = 0.0f;
		}
		mTargetLookAt = mpAnimatedCreature->field_164;
		mCurrentLookAt = mpAnimatedCreature->field_164;
	}
	else {
		mpAnimatedCreature->field_164 = mCurrentLookAt;
	}

	// field_54
	if (!false) {
		Vector3 point = { mpAnimatedCreature->mPosition.x, mpAnimatedCreature->mPosition.y, 500.0f };
		Graphics::FilterSettings settings;
		settings.requiredGroupFlags |= 1LL << ModelManager.GetGroupFlag((uint32_t)Graphics::ModelGroups::TestEnv);
		Vector3 intersection;
		mpModelWorld->Raycast(point, point - Vector3(0, 0, 1000.0f), nullptr, &intersection, nullptr, settings);
	}
}

void CreatureController::SetTargetPosition(const Vector3& pos) {
	mTargetPosition = pos;
	mRealTargetPosition = pos;
}
Vector3 CreatureController::GetTargetPosition() const {
	return mTargetPosition;
}

void CreatureController::SetTargetAngle(float angle) {
	mTargetAngle = angle;
}
float CreatureController::GetTargetAngle() const {
	return mTargetAngle;
}

void CreatureController::SetMovementSpeed(float speed) {
	mMovementSpeed = speed;
}
float CreatureController::GetMovementSpeed() const {
	return mMovementSpeed;
}

void CreatureController::SetAngularSpeed(float speed) {
	mAngularSpeed = speed;
}
float CreatureController::GetAngularSpeed() const {
	return mAngularSpeed;
}

Vector3 CreatureController::GetCurrentPosition() const {
	return mCurrentPosition;
}
float CreatureController::GetCurrentAngle() const {
	return mCurrentAngle;
}
