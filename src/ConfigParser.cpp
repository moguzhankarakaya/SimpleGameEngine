#include "ConfigParser.h"
#include <fstream>
#include <iostream>

ConfigParser::ConfigParser()
{
}

ConfigParser::~ConfigParser()
{
}


void ConfigParser::load(std::string s) {
	std::ifstream ifs;
	ifs.open(s, std::ifstream::in);
	std::string line;
	std::string arg;

	if (ifs.is_open()) {
		std::cout << "File '" << s << "' opened successfully \n";
		std::cout << "\t...parsing the parameters has started\n";

		while (ifs >> line) 
		{
			if (line[0] == '#')
			{
				ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				continue;
			}
			else if (line == "Terrain") 
			{
				ifs >> terrainInfo.heightMap >> terrainInfo.color >> terrainInfo.normal \
					>> terrainInfo.width     >> terrainInfo.depth >> terrainInfo.height;
			}
			else if (line == "Spawn") 
			{
				ifs >> spawnInfo.timeInfo >> spawnInfo.heightLow >> spawnInfo.heightHigh;
			}
			else if (line == "Mesh") 
			{
				MeshInfo mesh;
				ifs >> mesh.myName >> mesh.filename_t3d >> mesh.filename_dds_diffuse \
					>> mesh.filename_dds_specular       >> mesh.filename_dds_glow;
				meshes.push_back(mesh);
			}
			else if (line == "CockpitObject" || line == "GroundObject" || line == "WorldObject") 
			{
				ObjectInfo object(line);
				ifs >> object.myName >> object.scaling >> object.rotX >> object.rotY \
					>> object.rotZ   >> object.trnX    >> object.trnY >> object.trnZ \
					>> object.animation >> object.trigger;
				objects.push_back(object);
			}
			else if (line == "EnemyType") 
			{
				EnemyInfo object;
				ifs >> object.enemyName >> object.health  >> object.unitSize >> object.speed \
					>> object.meshName  >> object.scaling >> object.rotX     >> object.rotY  \
					>> object.rotZ      >> object.trnX    >> object.trnY     >> object.trnZ  \
					>> object.animation >> object.trigger;
				enemies.push_back(object);
			}
			else if (line == "Sprite") 
			{
				ifs >> arg;
				sprites.push_back(std::wstring(arg.begin(), arg.end()));
			}
			else if (line == "Gatling_Gun" || line == "Plasma_Gun") 
			{
				GunInfo gun(line);
				ifs >> gun.posX >> gun.posY >> gun.posZ >> gun.speed >> gun.gravity >> gun.cooldown \
					>> gun.damage >> gun.spriteIndex >> gun.spriteRadius >> gun.trigger;
				guns.push_back(gun);
			}
			else if (line == "Explosion") 
			{
				ExplosionInfo explosion;
				ifs >> explosion.explosionName >> explosion.spriteIndex >> explosion.spriteRadius \
					>> explosion.animationDuration >> explosion.spriteOpacity;
				explosions.push_back(explosion);
			}
			else if (line == "Skybox")
			{
				ifs >> skyboxInfo.filename_t3d >> skyboxInfo.filename_dds >> skyboxInfo.skyboxRadius;
			}
			else if (line == "Animation")
			{
				AnimationInfo animation;
				std::string animationType;
				ifs >> animation.animationName >> animation.meshName >> animationType \
					>> animation.vectorX >> animation.vectorY >> animation.vectorZ >> animation.duration;
				animation.setType(animationType);
				animations.push_back(animation);
			}
			else if (line == "ComplexAnimation")
			{
				ComplexAnimationInfo complexAnimation;
				std::string subAnimation;
				ifs >> complexAnimation.animationName >> complexAnimation.numberOfSubAnimations;
				for (int i = 0; i < complexAnimation.numberOfSubAnimations; i++)
				{
					ifs >> subAnimation;
					complexAnimation.animationList.push_back(subAnimation);
				}
				complexAnimations.push_back(complexAnimation);
			}
			else
				std::cout << "\tunkown type of parameter is encountered: " << line << "\n";

		}
		std::cout << "\t...parsing the parameters has finished\n\n";
	}

	else
		std::cout << "File could not be opened.. \n\n";

	ifs.close();
}

std::wstring ConfigParser::getTerrainPath(TerrainInfo::Texture texture) {
	std::wstring widestr;
	if (texture == TerrainInfo::Texture::HEIGHTMAP)
		widestr = std::wstring(terrainInfo.heightMap.begin(), terrainInfo.heightMap.end());
	else if (texture == TerrainInfo::Texture::NORMAL)
		widestr = std::wstring(terrainInfo.normal.begin(), terrainInfo.normal.end());
	else if (texture == TerrainInfo::Texture::COLOR)
		widestr = std::wstring(terrainInfo.color.begin(), terrainInfo.color.end());
	return widestr;
}