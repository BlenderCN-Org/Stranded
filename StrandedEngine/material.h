/*
* 创建材质数据
*/

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

// 对应 D3DMATERIAL9
struct stMaterial
{
	stMaterial()
	{
		emissiveR = emissiveG = emissiveB = emissiveA = 0;
		ambientR = ambientG = ambientB = ambientA = 1;
		diffuseR = diffuseG = diffuseB = diffuseA = 1;
		specularR = specularG = specularB = specularA = 0;
		power = 0;
	}

	float emissiveR, emissiveG, emissiveB, emissiveA;		// 在给定表面上发射出的光
	float ambientR, ambientG, ambientB, ambientA;			// 对象反射回场景中的环境色数目
	float diffuseR, diffuseG, diffuseB, diffuseA;			// 对象的漫反射属性(对象均匀反射的光量)
	float specularR, specularG, specularB, specularA;		// 对象反射回场景中的聚光数量
	float power;											// 指定镜面亮光区的亮度，越大，亮光区越亮
};

#endif