#pragma once

#include <Spore\BasicIncludes.h>

#define CreatureControllerPtr intrusive_ptr<CreatureController>

class CreatureController 
	: public Object
	, public DefaultRefCounted
{
public:
	static const uint32_t TYPE = id("CreatureController");
	
	CreatureController(Anim::AnimatedCreature* creature, Graphics::IModelWorld* groundWorld);
	~CreatureController();

	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;

	void Update(float dt);

	void SetTargetPosition(const Vector3& pos);
	Vector3 GetTargetPosition() const;

	void SetTargetAngle(float angle);
	float GetTargetAngle() const;

	void SetMovementSpeed(float speed);
	float GetMovementSpeed() const;

	void SetAngularSpeed(float speed);
	float GetAngularSpeed() const;

	Vector3 GetCurrentPosition() const;
	float GetCurrentAngle() const;

private:
	AnimatedCreaturePtr mpAnimatedCreature;
	IModelWorldPtr mpModelWorld;
	Vector3 mRealTargetPosition;
	Vector3 mTargetPosition;
	Vector3 mCurrentPosition;
	float mTargetAngle;
	float mCurrentAngle;
	float mMovementSpeed;
	float mAngularSpeed;
	Math::Vector3 mTargetLookAt;
	Math::Vector3 mCurrentLookAt;
	bool mSmoothLookAt;
	bool field_55;
};
