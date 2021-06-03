#include "stdafx.h"
#include "SphereCamera.h"

SphereCamera::SphereCamera()
	: mInput()
	, mNearPlane(0.01f)
	, mFarPlane(1000.0f)
	, mDistance(4.0f)
	, mLongitude(0)
	, mLatitude(0)
	, mTarget(0, 0, 0)
	, mRotateSpeed(PI)
{
}

SphereCamera::~SphereCamera()
{

}

// For internal use, do not touch this.
int SphereCamera::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not touch this.
int SphereCamera::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* SphereCamera::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(ICamera);
	CLASS_CAST(SphereCamera);
	return nullptr;
}


bool SphereCamera::OnAttach(App::ICameraManager* pManager) {
	return true;
}

bool SphereCamera::OnDeattach() {
	return true;
}

void SphereCamera::OnEnter() {

}

void SphereCamera::OnExit() {

}

/// UPDATE FUNCTION ///
void SphereCamera::Update(int deltaTime, App::cViewer* pViewer) 
{
	pViewer->SetFarPlane(mFarPlane);
	pViewer->SetNearPlane(mNearPlane);

	if (mInput.IsMouseDown(MouseButton::kMouseButtonLeft) && !mInput.mouseState.IsCtrlDown) {
		Point deltaMouse = mInput.mousePosition - mLastMouse;
		mLastMouse = mInput.mousePosition;

		mLongitude -= deltaMouse.x * mRotateSpeed / pViewer->GetViewport().Width;
		mLatitude += deltaMouse.y * mRotateSpeed / pViewer->GetViewport().Height;

		mLatitude = max(min(mLatitude, PI*0.95f / 2.0f), -PI*0.95f / 2.0f);
		mLongitude = fmodf(mLongitude, PI * 2.0f);
	}

	// Spherical coordinates
	float colatitude = Math::PI / 2.0f - mLatitude;
	Vector3 position = {
		mDistance * cosf(mLongitude) * sinf(colatitude),
		mDistance * sinf(mLongitude) * sinf(colatitude),
		mDistance * cosf(colatitude)
	};
	position += mTarget;

	pViewer->SetCameraTransform(Transform().SetOffset(position).SetRotation(Matrix3::LookAt(position, mTarget)));
}

void SphereCamera::func24h(bool arg) {
	if (!arg) mInput.Reset();
}


//// INPUT LISTENERS ////

// Called when a keyboard key button is pressed.
bool SphereCamera::OnKeyDown(int virtualKey, KeyModifiers modifiers)
{
	mInput.OnKeyDown(virtualKey, modifiers);
	
	// Return true if the keyboard event has been handled in this method.
	return false;
}

// Called when a keyboard key button is released.
bool SphereCamera::OnKeyUp(int virtualKey, KeyModifiers modifiers)
{
	mInput.OnKeyUp(virtualKey, modifiers);

	if (modifiers.IsCtrlDown && virtualKey == VK_SUBTRACT) {
		mDistance *= 1.2f;
	}
	else if (modifiers.IsCtrlDown && virtualKey == VK_ADD) {
		mDistance /= 1.2f;
	}
	
	// Return true if the keyboard event has been handled in this method.
	return false;
}

// Called when a mouse button is pressed (this includes the mouse wheel button).
bool SphereCamera::OnMouseDown(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseDown(mouseButton, mouseX, mouseY, mouseState);
	mLastMouse = { mouseX, mouseY };
	
	// Return true if the mouse event has been handled in this method.
	return false;
}

// Called when a mouse button is released (this includes the mouse wheel button).
bool SphereCamera::OnMouseUp(MouseButton mouseButton, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseUp(mouseButton, mouseX, mouseY, mouseState);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}

// Called when the mouse is moved.
bool SphereCamera::OnMouseMove(float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseMove(mouseX, mouseY, mouseState);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}

// Called when the mouse wheel is scrolled. 
// This method is not called when the mouse wheel is pressed.
bool SphereCamera::OnMouseWheel(int wheelDelta, float mouseX, float mouseY, MouseState mouseState)
{
	mInput.OnMouseWheel(wheelDelta, mouseX, mouseY, mouseState);

	mDistance -= wheelDelta / (120*3.0f);
	
	// Return true if the mouse event has been handled in this method.
	return false;
}


bool SphereCamera::func40h(int) {
	return false;
}
bool SphereCamera::func44h(int) {
	return false;
}
bool SphereCamera::func48h(int) {
	return false;
}

App::PropertyList* SphereCamera::GetPropertyList() {
	return nullptr;
}

void SphereCamera::Initialize() {
		
}

void SphereCamera::func54h(Vector3& dst) {
	dst = { 0, 0, 0 };
}


void SphereCamera::SetTarget(const Vector3& position) {
	mTarget = position;
}