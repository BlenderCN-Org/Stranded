#ifndef __MATH_DEFFINES_H__
#define __MATH_DEFFINES_H__

//ƽ����
#define STRANDED_FRONT 0
#define STRANDED_BACK 1
#define STRANDED_ON_PLANE 2
#define STRANDED_CLIPPED 3
#define STRANDED_CULLED 4
#define STRANDED_VISIBLE 5

//Բ����
#ifndef PI
#define PI 3.14159265358979323846
#endif

//����ת���ɻ��ȵĺ�
#define GET_RADIANS(degree) (float)(degree / 180.0f * PI)

#endif