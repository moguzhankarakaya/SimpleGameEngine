#pragma once
#include "Random.h"
#include "DirectXMath.h"

#include "ConfigParser.h"

struct EnemyInstance
{
	float speed;
	EnemyInfo enemyInfo;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
};

class EnemySpawnModel
{
private:
	UniformRandom<float> _circlePosSelector;
	UniformRandom<float> _heightSelector;
	UniformRandom<int> _enemySelector;
	ConfigParser _configParser;
	float _C_outer, _C_inner;
public:
	EnemySpawnModel() = default;

	~EnemySpawnModel() = default;

	EnemySpawnModel(ConfigParser& configParser);
	
	float get_C_Outer() { return _C_outer; }

	float get_C_Inner() { return _C_inner; }

	void generateEnemyInstance(EnemyInstance& anInstance);
};

#pragma once
