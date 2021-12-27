#ifndef MATH_H
#define MATH_H

#define _pi 3.1415926535897932384626433832795028841971693993
#define _e 2.71828182845904523536
#define NAN (__builtin_nanf(""))
#define HUGE_VAL __builtin_huge_val()
#define HUGE_VALF __builtin_huge_valf()
#define HUGE_VALL __builtin_huge_vall()
#define INFINITY __builtin_huge_inf()
#define math_errhandling 0 // TODO

#ifdef _cplusplus
extern "C" {
#endif

int abs(int n);
double fabs(double x);

double ceil(double x);
double floor(double x);
double trunc(double x);
double round(double x);
long int lround(double x);
long long int llround(double x);
double rint(double x);
long int lrint(double x);
long long int llrint(double x);
double nearbyint(double x);

double fmod(double numerator, double denominator);
double remainder(double numerator, double denominator);
double remquo(double numerator, double denominator, int* quotient);

double cos(double x);
double sin(double x);
double tan(double x);

double cosh(double x);
double sinh(double x);
double tanh(double x);

double acos(double x);
double asin(double x);
double atan(double x);
double atan2(double x, double y);

double acosh(double x);
double asinh(double x);
double atanh(double x);

double pow(double base, double exponent);
double sqrt(double x);
double cbrt(double x);

double hypot(double x, double y);

double scalbn(double x, int n);
double scalbln(double x, long int n);

double exp(double x);
double exp2(double x);
double expm1(double x);
double frexp(double x, int* exponent);
double ldexp(double x, int exponent);

double log(double x);
double log1p(double x);
double log2(double x);
double log10(double x);
double logb(double x);
int ilogb(double x);

double modf(double x, double* intpart);

double copysign(double x, double y);

double nan(const char* tagp);

double nextafter(double x, double y);
double nexttoward(double x, long double y);

double fdim(double x, double y);
double fmin(double x, double y);
double fmax(double x, double y);

double fma(double x, double y, double z);

#ifdef _cplusplus
}
#endif

#endif
