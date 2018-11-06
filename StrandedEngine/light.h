/*
* 创建灯源结构
*/

#ifndef __LIGHT_H__
#define __LIGHT_H__

// 对应D3DLIGHT9
struct stLight
{
	stLight()
	{
		type = 0;
		posX = 0, posY = 0, posZ = 0;
		dirX = 0, dirY = 0, dirZ = 0;
		ambientR = ambientG = ambientB = ambientA = 1;
		diffuseR = diffuseG = diffuseB = diffuseA = 1;
		specularR = specularG = specularB = specularA = 0;
		range = 0;
		falloff = 0;
		attenuation0 = 0;
		attenuation1 = 0;
		attenuation2 = 0;
		theta = 0;
		phi = 0;
	}

	int type;												// 光源类型
	float posX, posY, posZ;									// 在光源是点光源或聚光光源时，指光源位置
	float dirX, dirY, dirZ;									// 光发出的方向(可以是聚光，也可以是方向光)
	float ambientR, ambientG, ambientB, ambientA;			// 场景中的全局光照量
	float diffuseR, diffuseG, diffuseB, diffuseA;			// 漫反射在场景中的光数量
	float specularR, specularG, specularB, specularA;		// 镜面光反射在场景中的该类型光的数量
	float range;											// 光作用的距离，点光源作用半径
	float falloff;											// 聚光光源在内部和外部锥体中的亮度衰减量
	float attenuation0;										// Attenuation* 指明光在距离上亮度的改变方式，只有点光源和聚光光源才用到(常量)
	float attenuation1;										// (线性)
	float attenuation2;										// (二次)
	float theta;											// 聚光光源内部的锥体弧度角
	float phi;												// 聚光光源外面的锥体弧度角
};

#endif