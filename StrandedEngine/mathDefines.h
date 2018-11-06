#ifndef __MATH_DEFFINES_H__
#define __MATH_DEFFINES_H__

//平面结果
#define STRANDED_FRONT 0
#define STRANDED_BACK 1
#define STRANDED_ON_PLANE 2
#define STRANDED_CLIPPED 3
#define STRANDED_CULLED 4
#define STRANDED_VISIBLE 5

//圆周率
#ifndef PI
#define PI 3.14159265358979323846
#endif

//将度转换成弧度的宏
#define GET_RADIANS(degree) (float)(degree / 180.0f * PI)

#endif