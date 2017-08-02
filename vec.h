#ifndef __VEC_H__
#define __VEC_H__

#define PI 3.14159265f
#define RAD  PI / 180.0f

#include <math.h>

template <class T>
class vec4
{
public:
	union {
		T data[4];
		struct { T x, y, z, w; };
	};
	
};

template<class T>
class mat4 {
public:
	vec4<T> data[4];
};


template<class T>
mat4<T> rotation_matrix(T angle, int x, int y, int z)
{
	T cx = cos(angle* PI / 180);
	T sx = sin(angle* PI / 180);

	if (x)
	{
		mat4<T> rm = { {
			{ 1,0,0,0 },
			{ 0,cx,-sx,0 },
			{ 0,sx,cx,0 },
			{ 0,0,0,1 }
			} };
		return rm;
	}
	else if (y)
	{
		mat4<T> rm = { {
			{ cx,0,sx,0 },
			{ 0,1,0,0 },
			{ -sx,0,cx,0 },
			{ 0,0,0,1 }
			} };
		return rm;
	}
	else
	{
		mat4<T> rm = { {
			{ cx,-sx,0,0 },
			{ sx,cx,0,0 },
			{ 0, 0, 1, 0 },
			{ 0,0,0,1 }
			} };
		return rm;
	}
}



#endif //__VEC_H__