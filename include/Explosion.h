#pragma once
#ifndef EXPLOSION_G
#include <list>
#include <windows.h>

#include <DXUT.h>
#include "DXUTcamera.h"

#include "Random.h"
#include "ConfigParser.h"
#include "SpriteRenderer.h"

struct ExplosionParticle : SpriteVertex 
{
	DirectX::XMFLOAT3 velocity;
	float gravity;
	
	bool updateParticle(float fElapsedTime)
	{
		DirectX::XMStoreFloat3(&velocity, DirectX::XMVectorAdd(XMLoadFloat3(&velocity), DirectX::XMVectorSet(0, -gravity*fElapsedTime, 0, 0)));
		DirectX::XMStoreFloat3(&position, DirectX::XMVectorAdd(XMLoadFloat3(&position), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&velocity), fElapsedTime)));

		radius     *= 0.97f;
		opacity    *= 0.90f;
		animation += fElapsedTime / 2.0f;
		
		if (animation > 1)
			return true;
		return false;
	}
};

struct Explosion : SpriteVertex
{
public:
	float duration;

	Explosion(DirectX::XMFLOAT3 _position, const std::vector<ExplosionInfo>& _info);

	void generateParticles(int minNumParticles, int maxNumParticles, std::list<ExplosionParticle>& g_particles);

	void update(const float fElapsedTime);

	bool isStale();

private:
	UniformRandom<int> intRandomGenerator;
	UniformRandom<float> floatRandomGenerator;

	int selectExplosion(const std::vector<ExplosionInfo>& _info);
};

#endif // EXPLOSION_G