/*
* ������������
*/

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

// ��Ӧ D3DMATERIAL9
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

	float emissiveR, emissiveG, emissiveB, emissiveA;		// �ڸ��������Ϸ�����Ĺ�
	float ambientR, ambientG, ambientB, ambientA;			// ������س����еĻ���ɫ��Ŀ
	float diffuseR, diffuseG, diffuseB, diffuseA;			// ���������������(������ȷ���Ĺ���)
	float specularR, specularG, specularB, specularA;		// ������س����еľ۹�����
	float power;											// ָ�����������������ȣ�Խ��������Խ��
};

#endif