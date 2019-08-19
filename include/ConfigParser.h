#pragma once
#include<map>
#include<vector>
#include<string>
#include <tchar.h>

struct GunInfo {
	enum class Type{GATLING, PLASMA, UNKNOWN};
	Type type;
	int spriteIndex; 
	std::string trigger;
	float posX, posY, posZ, speed, gravity, cooldown, damage, spriteRadius;

	GunInfo(std::string typeName) {
		if (typeName == "Gatling_Gun") type = Type::GATLING;
		else if (typeName == "Plasma_Gun") type = Type::PLASMA;
		else type = Type::UNKNOWN;
	}
};

struct MeshInfo {
	std::string myName;
	std::string filename_t3d;
	std::string filename_dds_diffuse;
	std::string filename_dds_specular;
	std::string filename_dds_glow;
};

struct EnemyInfo {
	std::string enemyName, meshName, animation, trigger;
	int unitSize, speed; 
	float health, scaling, rotX, rotY, rotZ, trnX, trnY, trnZ;
};

struct SpawnInfo {
	float timeInfo, heightLow, heightHigh;
};

struct ObjectInfo {
	enum class Type{GRAOUND, WORLD, COCKPIT};
	Type type;
	std::string myName, animation, trigger;
	float scaling, rotX, rotY, rotZ, trnX, trnY, trnZ;

	ObjectInfo( std::string typeName) {
		if      (typeName == "CockpitObject") type = Type::COCKPIT;
		else if (typeName == "GroundObject")  type = Type::GRAOUND;
		else if (typeName == "WorldObject")   type = Type::WORLD;
	}
};

struct SkyboxInfo {
	float skyboxRadius;
	std::string filename_t3d;
	std::string filename_dds;
};

struct TerrainInfo {
	enum class Texture { HEIGHTMAP, COLOR, NORMAL };
	std::string heightMap, color, normal;
	float width, depth, height;
};

struct ExplosionInfo {
	std::string explosionName;
	float spriteRadius, animationDuration, spriteOpacity;
	int spriteIndex;
};

struct AnimationInfo
{
	enum class Type{ROTATION, TRANSLATION};
	std::string animationName, meshName;
	Type animationType;
	float vectorX, vectorY, vectorZ, duration;
	void setType(std::string type)
	{
		if (type == "Rotation")
			animationType = Type::ROTATION;
		else if (type == "Translation")
			animationType = Type::TRANSLATION;
	}
};

struct ComplexAnimationInfo
{
	int numberOfSubAnimations;
	std::string animationName;
	std::vector<std::string> animationList;
};

class ConfigParser
{
public:
	// Constructor and Destructor
	ConfigParser();
	~ConfigParser();

	// Helper Functions
	void load(std::string);
	
	// Getter Functions
	float getTerrainWidth()  { return terrainInfo.width;  }
	float getTerrainDepth()  { return terrainInfo.depth;  }
	float getTerrainHeight() { return terrainInfo.height; }
		
	size_t getNumOfMesh() { return meshes.size(); }
	size_t getNumOfObjects() { return objects.size(); }
	
	const SpawnInfo& getSpawnInfo() const { return spawnInfo; }
	const SkyboxInfo& getSkyboxInfo() const { return skyboxInfo; }

	std::wstring getTerrainPath(TerrainInfo::Texture texture);

	const std::vector<GunInfo>& getGunInfo() const { return guns; }
	const std::vector<MeshInfo>& getMeshInfo() const { return meshes; }
	const std::vector<EnemyInfo>& getEnemyInfo() const { return enemies; }
	const std::vector<ObjectInfo>& getObjectInfo() const { return objects; }
	const std::vector<std::wstring>& getSpriteFiles() const { return sprites; }
	const std::vector<ExplosionInfo>& getExplosionInfo() const { return explosions; }
	const std::vector<AnimationInfo>& getAnimationInfo() const { return animations; }
	const std::vector<ComplexAnimationInfo>& getComplexAnimationInfo() const { return complexAnimations; }

private:
	float spinning;
	float spinSpeed;
	float terrainWidth;
	float terrainDepth;
	float terrainHeight;
	
	std::vector<std::wstring> sprites;
	
	std::vector<GunInfo> guns;
	std::vector<MeshInfo> meshes;
	std::vector<EnemyInfo> enemies;
	std::vector<ObjectInfo>	objects;
	std::vector<AnimationInfo> animations;
	std::vector<ExplosionInfo> explosions;
	std::vector<ComplexAnimationInfo> complexAnimations;
	
	SpawnInfo spawnInfo;
	SkyboxInfo skyboxInfo;
	TerrainInfo terrainInfo;
};

extern ConfigParser g_configParser;
