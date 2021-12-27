/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "math.h"
#include "float.h"


int abs(int n)
{
    return (n<0?-n:n);
}
double fabs(double x)
{
    double result;
    __asm__("fabs" : "=t"(result) : "0"(x));
    return result;
}

double ceil(double x)
{
    double result;
    __asm__("frndint" :"=t"(result) : "0"(x));
    if (result < x)
    {
        return result + 1;
    }
    return result;
}
double floor(double x)
{
    double result;
    __asm__("frndint" :"=t"(result) : "0"(x));
    if (result > x)
    {
        return result - 1;
    }

    return result;
}

double fmod(double numerator, double denominator)
{
    return (numerator - (double)((int)(numerator/denominator)) * denominator);
}

double cos(double x)
{
    double result;
    __asm__("fcos" : "=t"(result) : "0"(x));
    return result;
}
double sin(double x)
{
    double result;
    __asm__("fsin" : "=t"(result) : "0"(x));
    return result;
}
double tan(double x)
{
    double result;
    __asm__("fptan; fstp %%st(0)": "=t"(result) : "0"(x));
    return result;
}

double cosh(double x)
{
    return ((pow(_e, x)+pow(_e, -x))/2.0);
}
double sinh(double x)
{
    return ((pow(_e, x)-pow(_e, -x))/2.0);
}
double tanh(double x)
{
    return (1.0 - 2.0/(pow(_e, 2.0*x) + 1.0));
}

double acos(double x)
{
    if (x < -1 || x > 1)
        return NAN;

    return (_pi / 2 - asin(x));
}
double asin(double x)
{
    if (x < -1 || x > 1)
        return NAN;

    return (2 * atan(x / (1 + sqrt(1 - (x * x)))));
}
double atan(double x)
{
    double result;
    __asm__("fld1; fpatan" : "=t"(result) : "0"(x));
    return result;
}
double atan2(double x, double y)
{
    double result;
    __asm__("fpatan" : "=t"(result) : "0"(y), "u"(x));
    return result;
}

double acosh(double x)
{
    return log(x + sqrt(x * x - 1));
}
double asinh(double x)
{
    return log(x + sqrt(x * x + 1));
}
double atanh(double x)
{
    return log((1+x)/(1-x)) / 2;
}

double pow(double base, double exponent)
{
    if (base == 0.0)
    {
        if (exponent < 0.0)
            return HUGE_VAL;
        else if (exponent == 0.0)
            return 1.0;
        else
            return 0.0;
    }

    double result;
    __asm__("fyl2x" : "=t"(result) : "0"(base), "u"(exponent));

    double retVal;
    __asm__("f2xm1" : "=t"(retVal) : "0"(result));
    return (retVal+1.0);
}
double sqrt(double x)
{
    if (x <  0.0)
        return NAN;

    double result;
    __asm__("fsqrt" : "=t"(result) : "0"(x));
    return result;
}
double cbrt(double x)
{
    return pow(x, 1.0 / 3.0);
}

double hypot(double x, double y)
{
    return sqrt(x*x + y*y);
}

double scalbn(double x, int n); // TODO
double scalbln(double x, long int n); // TODO

double exp(double x)
{
    double result;
    __asm__("fldl2e": "=t"(result));
    return pow(exp2(result), x);
}
double exp2(double x)
{
    double rndResult;
    double powResult = 1;
    double result;
    double fl = 0;
    int i;

    __asm__("frndint" : "=t"(rndResult) : "0"(x));

    if (rndResult > x)
    {
        fl = x - (rndResult - 1.0);
        rndResult -= 1.0;
    }
    else if (rndResult < x)
    {
        fl = x - rndResult;
    }


    for (i = 1; i <= rndResult; i++)
    {
        powResult *= 2.0;
    }
    __asm__("f2xm1" : "=t"(result) : "0"(fl));

    if (x >= 0)
    {
        return (result + 1.0) * powResult;
    }
    return 1.0 / ((result + 1.0) * powResult);
}
double expm1(double x)
{
    double result;
    __asm__("fldl2e": "=t"(result));
    return pow(exp2(result), x) - 1.0;
}

double frexp(double x, int* exponent)
{
    int sign = 1;
    if (x < 0)
    {
        x *=-1;
        sign = -1;

    }
    double e = log(x);
    int po2x;
    *exponent =(int) ceil(e);
    if (*exponent == e)
    {
        return 1.0;
    }
    po2x = (int) exp2((double)*exponent);
    return (x / po2x) *sign;
}
double ldexp(double x, int exponent)
{
    return (x*pow(2, exponent));
}

double log(double x)
{
    if (x <= 0.0)
        return NAN;

    double retVal;
    __asm__(
        "fyl2x;"
        "fldln2;"
        "fmulp" : "=t"(retVal) : "0"(x), "u"(1.0));

    return retVal;
}
double log1p(double x)
{
    return log(x+1.0);
}
double log2(double x)
{
    return log(x) / log(2.0);
}
double log10(double x)
{
    if (x <= 0.0)
        return NAN;

    double retVal;
    __asm__(
        "fyl2x;"
        "fldlg2;"
        "fmulp" : "=t"(retVal) : "0"(x), "u"(1.0));

    return retVal;
}
double logb(double x)
{
    return log(x) / log(FLT_RADIX);
}
int ilogb(double x)
{
    return logb(x);
}

double modf(double x, double* intpart)
{
    *intpart = (double)((int)x);
    return (x-*intpart);
}

double copysign(double x, double y)
{
    if ((x < 0 && y < 0) || (x > 0 && y > 0))
        return x;
    return -x;
}

double nan(const char* tagp); // TODO

double nextafter(double x, double y); // TODO
double nexttoward(double x, long double y); // TODO

double fdim(double x, double y)
{
    if (x > y)
        return x - y;
    return y - x;
}
double fmin(double x, double y)
{
    if (x > y)
        return y;
    return x;
}
double fmax(double x, double y)
{
    if (x < y)
        return y;
    return x;
}

double fma(double x, double y, double z)
{
    return x*y + z;
}


/*
* Copyright (c) 2010-2015 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
