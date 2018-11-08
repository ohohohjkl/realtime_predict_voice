#ifndef COMPLEX_H
#define COMPLEX_H

/*define-----------------------
------------------------------*/

/*constants, variables--------
------------------------------*/

typedef struct cplx {
	float real;
	float imag;
}cplx;

/*functions--------------------
------------------------------*/
cplx cexp(float temp);
cplx mul(cplx x, cplx y);
cplx add(cplx x, cplx y);
cplx sub(cplx x, cplx y);
int cimag(float temp);
void show(const char * s, cplx buf[], int n);
double fast_sine(float x);
double fast_cosine(float x);
#endif //COMPLEX_H