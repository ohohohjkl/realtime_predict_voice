#ifndef MFCC_H
#define MFCC_H

/*includes
------------------------------------------
------------------------------------------*/
#include "record.h"
#include "utils.h"
#include "complex.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "windows.h"
#include <process.h>
#include "Sysinfoapi.h"
#include "kiss_fft.h"
#include "gemm.h"
/*defines, constants
------------------------------------------
------------------------------------------*/

long long start, stop, sample_start,last;
#define PRINT_TIME_PROCESS_START(s) do {	\
	s = PerformanceCounter();				\
}while(0)

#define PRINT_TIME_PROCESS_STOP(s,message,threshold) do {	\
	double dftDuration2 = (double)(PerformanceCounter() - s) * 1000.0 / (double)Frequency.QuadPart;	\
	if (dftDuration2 > threshold)				\
		printf(message ": %f\n", dftDuration2);	\
}while(0)

#define PRINT_TIME_SAMPLE_START()					PRINT_TIME_PROCESS_START(sample_start)
#define PRINT_TIME_SAMPLE__STOP(message,threshold)	PRINT_TIME_PROCESS_STOP(sample_start,message,threshold)

//#define PRINT_TIME_PROCESS_STOP(message,threshold)
//#define PRINT_TIME_PROCESS_START()

#define PI 3.14159265359
#define false 0
#define true 1
#define FEATSIZE 91

#ifdef __cplusplus
extern "C" {
#endif
	typedef struct SIGNAL {
		SAMPLE *raw_signal;
		int frame_length;		//so sample trong 1 frame
		int step_lengh;		//do dai bc nhay
		int num_frame;			//so frame trong 1 audio signal
		int signal_length;
	}SIGNAL;

	typedef struct COMPLEX {
		float img;
		float real;
		float magnitude;
	}COMPLEX;

	typedef struct hyper_vector {
		int row;
		int col;
		int dim;
		SAMPLE *data;
	}hyper_vector;

	typedef struct filter_bank {
		float *data;
		int nfilt;
		int filt_len;
	}filter_bank;
	/*functions
	------------------------------------------
	------------------------------------------*/
	filter_bank getFBank(float *fbank, int nfilt, int filt_len);
	int getLength(SAMPLE *a);
	SIGNAL setSignal(SAMPLE *a, int size);
	SIGNAL setSignal2(SAMPLE * a, int size);
	hyper_vector setHVector(SAMPLE *a, int col, int row, int dim);
	hyper_vector setEHVector(int col, int row, int dim);
	hyper_vector setHVector2(SAMPLE * a, int col, int row, int dim);
	hyper_vector getFrames(struct SIGNAL a);
	void append_energy(hyper_vector dct, hyper_vector pow_spec);

	//COMPLEX *DFT(hyper_vector a, int pointFFT
	hyper_vector DCT(hyper_vector a, int num_ceps);
	bool FastDctLee_transform(double vector[], size_t len);
	static void forwardTransform(double vector[], double temp[], size_t len);
	hyper_vector DFT_PowerSpectrum(hyper_vector a, int pointFFT);

	float magnitude(float real, float img);
	filter_bank filterbank(int nfilt, int NFFT);
	float HammingWindow(float a, int frameLength);
	SIGNAL preEmphasis(SAMPLE *a, int size, float preemh);

	float hz2mel(float hz);
	float mel2hz(float hz);

	hyper_vector multiply(hyper_vector matrix1, hyper_vector matrix2);
#ifdef USE_MULTI_THREAD
	hyper_vector multiply_multithread(hyper_vector matrix1, hyper_vector matrix2);
#endif // USE_MULTI_THREAD

	hyper_vector transpose(hyper_vector matrix);

	SIGNAL silence_trim(SIGNAL a);
	SAMPLE *reverse(SIGNAL a);

	//////////////////////////////////final_feature/////////////////////////////
	hyper_vector cov(hyper_vector mfcc);
	void normalize(char *path_nor, char *path_mean, char*path_sum, int* label, float * data, int row, int col);
	void Get_normalize(int label, float * data, int row, int col);
	void normalize2(int label, float * data, int row, int col);
	hyper_vector var(hyper_vector);
	hyper_vector get_feature_vector_from_signal(SIGNAL a, hyper_vector fbank);
	hyper_vector get_feature_vector_from_signal2(hyper_vector a, hyper_vector fbank);
	void write_feature_vector_to_database(hyper_vector feature_vector, char *name);

	////////////////////////////////

	int check_path(char *path);

	//////////////////////test_signal_via_matlab/////////////////////////
	void writeDBFS(SAMPLE* raw_signal, int trim_ms, int signal_len);
	void create_database(char *path, int max_index, hyper_vector fbank);
	void normalize_db(char *path_nor, char *path_mean, char *path_db, char *path_info, char*path_sum, int max_index);
	void normalize_from_file(char *path_nor, char *path_mean, char *filename, char*path_sum, int row, int col);

	///////////////////////new FFT//////////////////////////////////
	void _fft(cplx buf[], cplx out[], int n, int step);
	hyper_vector fft(hyper_vector frames, int n);
	cplx * get_complex_from_hyper_vector(hyper_vector temp);
	void mfcc_load_normalized_sum(SAMPLE *sum_normal, char *path);
#ifdef __cplusplus
}
#endif // __cplusplus



#endif // !MFCC_H