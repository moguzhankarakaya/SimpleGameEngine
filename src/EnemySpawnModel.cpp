#include "EnemySpawnModel.h"

EnemySpawnModel::EnemySpawnModel( ConfigParser& configParser ) :
	_configParser(configParser)
{

	size_t numberOfEnemies = configParser.getEnemyInfo().size();
	_enemySelector.setParams(0, numberOfEnemies - 1);
	_circlePosSelector.setParams(0, DirectX::XM_2PI);
	_heightSelector.setParams(configParser.getSpawnInfo().heightLow,
							  configParser.getSpawnInfo().heightHigh);

	_C_outer = (configParser.getTerrainWidth() + configParser.getTerrainDepth()) / 6.00f;
	_C_inner = (configParser.getTerrainWidth() + configParser.getTerrainDepth()) / 26.0f;

}

void EnemySpawnModel::generateEnemyInstance(EnemyInstance& anInstance)
{
	int enemySelected = _enemySelector.generate();
	float outerCircleAlpha = _circlePosSelector.generate();
	float innerCircleAlpha = _circlePosSelector.generate();
	float randomHeight = _heightSelector.generate() * _configParser.getTerrainHeight();
	DirectX::XMVECTOR pos1 = {};
	pos1.m128_f32[0] = _C_outer * std::sin(outerCircleAlpha);
	pos1.m128_f32[1] = randomHeight;
	pos1.m128_f32[2] = _C_outer * std::cos(outerCircleAlpha);
	DirectX::XMVECTOR pos2 = {};
	pos2.m128_f32[0] = _C_inner * std::sin(outerCircleAlpha);
	pos2.m128_f32[1] = randomHeight;
	pos2.m128_f32[2] = _C_inner * std::cos(outerCircleAlpha);
	DirectX::XMVECTOR velocity = {};
	velocity.m128_f32[0] = pos2.m128_f32[0] - pos1.m128_f32[0];
	velocity.m128_f32[1] = pos2.m128_f32[1] - pos1.m128_f32[1];
	velocity.m128_f32[2] = pos2.m128_f32[2] - pos1.m128_f32[2];
	float speed = DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity));
	DirectX::XMVECTOR velocityNorm = DirectX::XMVector3Normalize(velocity);
	EnemyInfo selectedEnemyInfo = g_configParser.getEnemyInfo()[enemySelected];

	DirectX::XMStoreFloat3(&(anInstance.pos), pos1);
	DirectX::XMStoreFloat3(&(anInstance.vel), velocityNorm);
	anInstance.enemyInfo = selectedEnemyInfo;
	anInstance.speed = speed;

}
