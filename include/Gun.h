#pragma once
#ifndef GUN_G
#include <list>
#include <windows.h>

#include <DXUT.h>
#include <DXUTcamera.h>
#include <DirectXMath.h>

#include "SpriteRenderer.h"
#include "ConfigParser.h"
#include "AbsAnimation.h"
#include "Keybindings.h"
#include "Mesh.h"

class Animation;

struct Projectile : SpriteVertex
{
	DirectX::XMFLOAT3 velocity;
	float isAliveFor;
	const GunInfo* info;

	void SetGunInfo(const GunInfo* _info)
	{
		info = _info;
		radius = info->spriteRadius;
		textureIndex = info->spriteIndex;
	}

	bool updateProjectile(float fElapsedTime)
	{
		DirectX::XMStoreFloat3(&velocity, DirectX::XMVectorAdd(XMLoadFloat3(&velocity), DirectX::XMVectorSet(0, -info->gravity*fElapsedTime, 0, 0)));
		DirectX::XMStoreFloat3(&position, DirectX::XMVectorAdd(XMLoadFloat3(&position), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&velocity), fElapsedTime)));

		isAliveFor += fElapsedTime;
		if (isAliveFor > 2)
			return true;
		return false;
	}
};

struct Gun
{
public:
	Gun(GunInfo&, KeyBindings*);
	void equip();
	UINT gunKey();
	void unequip();
	GunInfo info();
	void gunKeyState(bool);
	void setAnimation(AbsAnimation* _animation);
	bool fire(const float, const CFirstPersonCamera*, std::list<Projectile>&);

private:
	bool calculateProjectile(const float, const CFirstPersonCamera*, std::list<Projectile>&);

private:
	AbsAnimation*     animation;
	float	          deltaTime;
	bool              bKeyDown;
	bool              equiped;
	UINT              keyBind;
	GunInfo           gunInfo;
};


#endif // GUN_G
