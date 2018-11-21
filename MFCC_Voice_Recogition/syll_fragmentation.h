#ifndef SYLL_FRAGMENTATION_H
#define SYLL_FRAGMENTATION_H
/*define-----------------------
------------------------------*/
#include <stdint.h>
#include "filter.h"
#include "mfcc.h"
#include "windows.h"
#include "Sysinfoapi.h"
#include "portaudio.h"
#include "pa_ringbuffer.h"
#include "pa_util.h"

#ifdef _WIN64
#include <windows.h>
#include <process.h>
#endif


static ring_buffer_size_t rbs_min(ring_buffer_size_t a, ring_buffer_size_t b)
{
	return (a < b) ? a : b;
}

#define SAMPLE float
/*constants, variables--------
------------------------------*/
long long start, stop, sample_start,last;
int tflag = 0;
int tdem = 0;
int *sent_buff;

#define PRINT_TIME_PROCESS_START(s) do {	\
	s = PerformanceCounter();				\
}while(0)

#define PRINT_TIME_PROCESS_STOP(s,message,threshold) do {											\
	double dftDuration2 = (double)(PerformanceCounter() - s) * 1000.0 / (double)Frequency.QuadPart;	\
	if (dftDuration2 > threshold)																	\
		printf(message ": %f\n", dftDuration2);														\
}while(0)

#define PRINT_TIME_SAMPLE_START()					PRINT_TIME_PROCESS_START(sample_start)
#define PRINT_TIME_SAMPLE_STOP(message,threshold)	PRINT_TIME_PROCESS_STOP(sample_start,message,threshold)

/*defines, constants
------------------------------------------
------------------------------------------*/
#define QUEUE_SIZE	1120
#define MAX_WORD_BUFFER	200

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define FILE_NAME       "audio_data.raw"
#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER (160)
#define NUM_SECONDS     (0.01)
#define NUM_CHANNELS    (1)
#define NUM_WRITES_PER_BUFFER   (1)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/

typedef int(*ThreadFunctionType)(void*);


//typedef struct
//{
//	unsigned            frameIndex;
//	int                 threadSyncFlag;
//	SAMPLE             *ringBufferData;
//	PaUtilRingBuffer    ringBuffer;
//	FILE               *file;
//	void               *threadHandle;
//}paTestData;

typedef struct
{
	uint16_t formatType;
	uint8_t numberOfChannels;
	uint32_t sampleRate;
	size_t size;
	float *recordedSamples;
} AudioData;

typedef struct
{
	float *snippet;
	size_t size;
} AudioSnippet;

AudioData initAudioData(uint32_t sampleRate, uint16_t channels, int type)
{
	AudioData data;
	data.formatType = type;
	data.numberOfChannels = channels;
	data.sampleRate = sampleRate;
	data.size = 0;
	data.recordedSamples = NULL;
	return data;
}

/*functions--------------------
------------------------------*/
void real_time_predict(struct svm_model *model, SAMPLE *sum_normal, char *def_path, char *sent_path);
void real_time_predict2(struct svm_model *model, SAMPLE *sum_normal, char *def_path, char *sent_path);
void Push(float *data, int index, float *word);
void write_to_syll(int *d_word, char *def_name, char *ext, char *path, int *dist, float *word, struct svm_model *model, SAMPLE *sum_normal, filter_bank fbank);
int silence_detect(float *data, size_t length, int *time, int *cond_flag, int *dist, float *word, float *peak, float *syll, float *lowPeak1, float *lowPeak2,
	int *d_word, char *def_name, char *ext, char *path, float *A, float *d1, float *d2, float *d3, float *d4, float *w0, float *w1, float *w2, float *w3, float *w4, float *x, struct svm_model *model, SAMPLE *sum_normal, filter_bank fbank, PaStream *stream);
void check_sentence_formation(char *path, char *ext, int sent_len, char *wtemp);
inline long long PerformanceCounter();
int check_word(int word, int*pword);

////
static int threadFunctionWriteToRawFile(void* ptr);
static int threadFunctionReadFromRawFile(void* ptr);

#endif // !SYLL_FRAGMENTATION_H
