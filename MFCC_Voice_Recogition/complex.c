#include "complex.h"


cplx cexp(float temp) {
	cplx result;
	result.imag = sin(temp);
	result.real = cos(temp);
	return result;
}

cplx mul(cplx x, cplx y) {
	cplx z;
	z.real = x.real * y.real - x.imag * y.imag;
	z.imag = x.real * y.imag + x.imag * y.real;
	return z;
}

cplx add(cplx x, cplx y) {
	cplx z;
	z.real = x.real + y.real;
	z.imag = y.imag + x.imag;
	return z;
}

cplx sub(cplx x, cplx y) {
	cplx z;
	z.real = x.real - y.real;
	z.imag = x.imag - y.imag;
	return z;
}

void show(const char * s, cplx buf[], int n) {
	printf("%s", s);
	for (int i = 0; i < n; i++)
		if (cimag(buf[i].imag)) {
			printf("%.2f ", buf[i].real);
		}
		else {
			printf("(%.2f, %.2f) ", buf[i].real, buf[i].imag);
		}
}

int cimag(float temp) {
	if (temp == 0)
		return 1;
	else return 0;
}

double fast_sine(float x) {
	/*********************************************************
	* high precision sine/cosine
	*********************************************************/
	double sin;
	//always wrap input angle to -PI..PI
	if (x < -3.14159265359)
		x += 6.28318530718;
	else
		if (x > 3.14159265359)
			x -= 6.28318530718;

	//compute sine
	if (x < 0)
	{
		sin = 1.27323954 * x + .405284735 * x * x;

		if (sin < 0)
			sin = .225 * (sin *-sin - sin) + sin;
		else
			sin = .225 * (sin * sin - sin) + sin;
	}
	else
	{
		sin = 1.27323954 * x - 0.405284735 * x * x;

		if (sin < 0)
			sin = .225 * (sin *-sin - sin) + sin;
		else
			sin = .225 * (sin * sin - sin) + sin;
	}

	//compute cosine: sin(x + PI/2) = cos(x)
	x += 1.57079632679;
	if (x > 3.14159265)
		x -= 6.28318531;
	return sin;
}

double fast_cosine(float x) {
	return fast_sine(x + 1.57079632679);
}