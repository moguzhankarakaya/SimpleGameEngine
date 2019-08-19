#include "Explosion.h"

Explosion::Explosion(DirectX::XMFLOAT3 _position, const std::vector<ExplosionInfo>& _info)
{
	int selectedExplosion = selectExplosion(_info);

	std::cout << "Explosion " << _info[selectedExplosion].explosionName << " is selected." << std::endl;

	position = _position;
	radius = _info[selectedExplosion].spriteRadius;
	textureIndex = _info[selectedExplosion].spriteIndex;
	animation = 0;
	opacity = _info[selectedExplosion].spriteOpacity;
	duration = _info[selectedExplosion].animationDuration;
}

int Explosion::selectExplosion(const std::vector<ExplosionInfo>& _info)
{
	intRandomGenerator.setParams(0, _info.size()*3);
	return intRandomGenerator.generate() % (_info.size());
}

void Explosion::generateParticles(int minNumParticles, int maxNumParticles, std::list<ExplosionParticle>& g_particles)
{
	intRandomGenerator.setParams(minNumParticles, maxNumParticles);
	int numParticles = intRandomGenerator.generate();

	ExplosionParticle particle;
	particle.position = position;
	particle.animation = 0.0f;

	for (int i = 0; i < numParticles; i++)
	{
		floatRandomGenerator.setParams(-1.0f, 1.0f);
		DirectX::XMFLOAT3 velocity(floatRandomGenerator.generate(), floatRandomGenerator.generate(), floatRandomGenerator.generate());
		DirectX::XMStoreFloat3(&(particle.velocity), DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSet(velocity.x, velocity.y, velocity.z, 0.0f)), 100.0f));

		floatRandomGenerator.setParams(1.0f, 5.0f);
		particle.radius = floatRandomGenerator.generate();

		floatRandomGenerator.setParams(10.0f, 50.0f);
		particle.gravity = floatRandomGenerator.generate();

		floatRandomGenerator.setParams(0.0f, 1.0f);
		particle.opacity = floatRandomGenerator.generate();

		intRandomGenerator.setParams(0, 0);
		particle.textureIndex = intRandomGenerator.generate();

		g_particles.push_back(particle);
	}

}

void Explosion::update(const float fElapsedTime)
{
	animation += fElapsedTime / duration;
}

bool Explosion::isStale()
{
	if (animation > 1)
		return true;
	return false;
}