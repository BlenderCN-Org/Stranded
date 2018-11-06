/*
* ������Դ�ṹ
*/

#ifndef __LIGHT_H__
#define __LIGHT_H__

// ��ӦD3DLIGHT9
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

	int type;												// ��Դ����
	float posX, posY, posZ;									// �ڹ�Դ�ǵ��Դ��۹��Դʱ��ָ��Դλ��
	float dirX, dirY, dirZ;									// �ⷢ���ķ���(�����Ǿ۹⣬Ҳ�����Ƿ����)
	float ambientR, ambientG, ambientB, ambientA;			// �����е�ȫ�ֹ�����
	float diffuseR, diffuseG, diffuseB, diffuseA;			// �������ڳ����еĹ�����
	float specularR, specularG, specularB, specularA;		// ����ⷴ���ڳ����еĸ����͹������
	float range;											// �����õľ��룬���Դ���ð뾶
	float falloff;											// �۹��Դ���ڲ����ⲿ׶���е�����˥����
	float attenuation0;										// Attenuation* ָ�����ھ��������ȵĸı䷽ʽ��ֻ�е��Դ�;۹��Դ���õ�(����)
	float attenuation1;										// (����)
	float attenuation2;										// (����)
	float theta;											// �۹��Դ�ڲ���׶�廡�Ƚ�
	float phi;												// �۹��Դ�����׶�廡�Ƚ�
};

#endif