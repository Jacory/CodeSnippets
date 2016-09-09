#ifndef GDALHEADER_H
#define GDALHEADER_H

// ================ useful inline functions =============

/*!判断浮点数是否NaN值*/
inline bool isnan(const float&v)	{ return _isnan(v) ? true : false; }
/*!判断double数是否NaN值*/
inline bool isnan(const double&v)	{ return _isnan(v) ? true : false; }
/*!获取double的NaN值*/
inline double nan()	{ return numeric_limits<double>::quiet_NaN(); }



// ================ useful defines =============

/// 释放指针数组
#define RELEASE(x)	if(x!=NULL){delete[]x;x=NULL;}

// 定义PI=3.141592653...以及度和弧度转换
#ifndef M_PI
/*!定义圆周率PI*/
#define M_PI	3.1415926535897932384626433832795
/*!定义圆周率2*PI*/
#define M_PI2	6.283185307179586476925286766559
/*!弧度转度*/
#define DEG_PER_RAD	((double)(180.0/M_PI))
/*!度转弧度*/
#define RAD_PER_DEG	((double)(M_PI/180.0))
#endif

// 平方
#ifndef M_SQUARE
#define M_SQUARE(x)	(x)*(x)
#endif
// 立方
#ifndef M_CUBE
#define M_CUBE(x)	(x)*(x)*(x)
#endif

// float类型比较
#ifndef FLT_EQUALS
#define FLT_EQUALS(x,y) (fabs((double)x-y)<FLT_EPSILON) /*!浮点数是否相等*/
#define FLT_EQUALS_N(x,y,z)	(fabs((double)x-y)<z) /*!浮点数是否相等(指定比较阈值)*/
#endif
// float是否为0
#ifndef FLT_ZERO
#define FLT_ZERO(x)	(fabs(x)<FLT_EPSILON) /*!浮点数是否为0*/
#endif


#endif
