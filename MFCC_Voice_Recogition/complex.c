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