#include "Gun.h"


Gun::Gun(GunInfo& _gunInfo, KeyBindings* _bindings) : gunInfo(_gunInfo)
{	
	animation         = nullptr;
	bKeyDown          = false;
	equiped           = true;
	deltaTime         = 0.0f;

	if (_gunInfo.type == GunInfo::Type::GATLING)
	{
		keyBind		 = _bindings->getBindings()->getGatlingKey();
	}
	else if (_gunInfo.type == GunInfo::Type::PLASMA)
	{
		keyBind = _bindings->getBindings()->getPlasmaKey();
	}
}

void Gun::equip()
{
	equiped = true;
}

void Gun::unequip()
{
	equiped = false;
}

UINT Gun::gunKey()
{
	return keyBind;
}

void Gun::gunKeyState(bool _bKeyDown)
{
	bKeyDown = _bKeyDown;
}

GunInfo Gun::info()
{
	return gunInfo;
}

void Gun::setAnimation(AbsAnimation* _animation)
{
	animation = _animation;
}

bool Gun::fire(const float fElapsedTime, 
			   const CFirstPersonCamera* g_camera, 
			   std::list<Projectile>& g_projectiles)
{
	if (animation != nullptr)
	{
		if ( !(animation->isPlaying()) && bKeyDown)
		{
			if (calculateProjectile(fElapsedTime, g_camera, g_projectiles))
			{
				animation->play();
				return true;
			}
			return false;
		}

		return false;
	}
	else
	{
		if (bKeyDown)
		{
			return calculateProjectile(fElapsedTime, g_camera, g_projectiles);
		}

		return false;
	}
}

bool Gun::calculateProjectile(const float fElapsedTime,
							  const CFirstPersonCamera* g_camera,
							  std::list<Projectile>& g_projectiles)
{
	if (deltaTime > gunInfo.cooldown)
	{
		Projectile projectile;
		DirectX::XMVECTOR projectilePosition = DirectX::XMVector3Transform(DirectX::XMVectorSet(gunInfo.posX, gunInfo.posY, gunInfo.posZ, 1.0f), g_camera->GetWorldMatrix());
		DirectX::XMStoreFloat3(&(projectile.position), projectilePosition);
		DirectX::XMVECTOR projectileVelocity = DirectX::XMVectorScale(g_camera->GetWorldAhead(), gunInfo.speed);
		DirectX::XMStoreFloat3(&(projectile.velocity), projectileVelocity);
		projectile.isAliveFor = 0.0f;
		projectile.animation = 0.0f;
		projectile.opacity = 1.0f;
		projectile.SetGunInfo(&gunInfo);
		g_projectiles.push_back(projectile);

		deltaTime = 0;
		return true;
	}
	else
	{
		deltaTime += fElapsedTime;
		return false;
	}
}