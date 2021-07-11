#ifndef __THREAD_FIXED_POINT_H
#define __THREAD_FIXED_POINT_H

//定点值的基本定义。
typedef int fixed_t;
//16 LSB用于小数部分。
#define FP_SHIFT_AMOUNT 16
//将值转换为定点值。
#define FP_CONST(A) ((fixed_t)(A<<FP_SHIFT_AMOUNT))
//两个定点值相加。
#define FP_ADD(A,B) (A+B)
//定点值a加上整型值B。
#define FP_ADD_MIX(A,B) (A+(B<<FP_SHIFT_AMOUNT))
//两个定点值相减。
#define FP_SUB(A,B) (A-B)
//定点值a减去整型值B。 
#define FP_SUB_MIX(A,B) (A-(B<<FP_SHIFT_AMOUNT))
//定点值a乘以整型值B。
#define FP_MULT_MIX(A,B) (A*B)
//定点值a除以整型值B。
#define FP_DIV_MIX(A,B) (A/B)
//两个定点值相乘。
#define FP_MULT(A,B) ((fixed_t)(((int64_t)A)*B>>FP_SHIFT_AMOUNT))
//两个定点值相除。
#define FP_DIV(A,B) ((fixed_t)((((int64_t)A)<<FP_SHIFT_AMOUNT)/B))
//获取定点值的整数部分。
#define FP_INT_PART(A) (A>>FP_SHIFT_AMOUNT)
//获取定点值的舍入整数。
#define FP_ROUND(A) (A>=0?((A+(1<<(FP_SHIFT_AMOUNT-1)))>>FP_SHIFT_AMOUNT):((A-(1<<(FP_SHIFT_AMOUNT-1)))>>FP_SHIFT_AMOUNT))

#endif //threads/fixed_point.h
