#include "syll_fragmentation.h"

void check_sentence_formation(char *path, char *ext, int sent_len, char *wtemp) {
	FILE *fsent = fopen(path, "r");

	if (fsent == NULL) {
		printf("FILE DOESN'T EXIST!!");
		return;
	}
	int num, dtemp, succeed = 0;
	fscanf(fsent, "%d", &num);

	for (int i = 0; i < num; i++) {
		fscanf(fsent, "%d", &dtemp);
		if (dtemp == sent_buff[i]) {
			succeed = 0;
		}
		else {
			succeed = 1;
			break;
		}
	}
	if (succeed == 0) {
		printf("VALID SENTENCE: ");	//return true
		for (int i = 0; i < num; i++) {
			fscanf(fsent, "%s", wtemp);
			printf("%s ", wtemp);
		}
	}
	else {
		printf("INVALID SENTENCE!! ");	//return true
	}
	fclose(fsent);
}



void Push(float *data, int index, float *word) {
	int dem = 480;
	if (index == 0) {
		for (int i = 0; i < 160; ++i) {
			word[i] = data[dem];
			dem++;
		}
	}
	else
	{
		for (int i = index * 160; i < (index + 1) * 160; i++) {
			if (index * 160 < (MAX_WORD_BUFFER * FRAMES_PER_BUFFER)) {
				word[i] = data[dem];
				dem++;
			}
			else
			{
				break;
			}
		}
	}
}

int silence_detect(float *data, size_t length, int *time, int *cond_flag, int *dist, float *word, float *peak, float *syll, float *lowPeak1, float *lowPeak2,
	int *d_word, char *def_name, char *ext, char *path, float *A, float *d1, float *d2, float *d3, float *d4, float *w0, float *w1, float *w2, float *w3, float *w4, float *x, struct svm_model *model, SAMPLE *sum_normal, filter_bank fbank, PaStream *stream) {
	x = butterworth_bandpass_v2(2, data, length, 16000, 4000, 500, A, d1, d2, d3, d4, w0, w1, w2, w3, w4, x);
	int chunk_size = 160;
	float sum = 0;
	int trim_ms = 0;
	int dem = 0;
	int succeed = 1;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	float *db = (float *)malloc(sizeof(float) * 7);
	while (trim_ms < length)
	{
		sum = 0;
		for (int i = trim_ms; i < trim_ms + chunk_size; i++) {
			sum += x[i] * x[i];
		}
		sum = sqrt(sum / chunk_size);
		sum = 20 * log10(sum);
		db[dem] = sum;
		dem++;
		trim_ms += chunk_size;
	}

	if (*time) {
		syll[0] = (db[0] + db[1] + db[2] + db[3] + db[4] + db[5]) / 6;
		syll[1] = (db[1] + db[2] + db[3] + db[4] + db[5] + db[6]) / 6;
		*lowPeak1 = syll[0];
		*lowPeak2 = 0;
	}
	else
	{
		syll[1] = (db[1] + db[2] + db[3] + db[4] + db[5] + db[6]) / 6;
	}
	switch (*cond_flag)
	{
	case 0:
		if (*lowPeak1 < syll[1]) {
			//printf("case 0, cond 0\n");
			*peak = syll[1];
			*cond_flag = 1;
			(*dist) += 1;
			//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
			if (!(*time)) {
				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
				Push(x, *dist - 1, word);
				*time = 0;
			}
			else
			{
				Push(x, *dist - 1, word);
				*time = 0;
			}
		}
		else if (*lowPeak1 >= syll[1]) {
			//printf("case 0, cond 1\n");
			*lowPeak1 = syll[1];
			/*free(word);
			word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER);*/
			//word = (float *)realloc(word, sizeof(float) * FRAMES_PER_BUFFER);
		}
		break;
	case 1:
		if (*peak < syll[1]) {
			//printf("case1 cond 0.0\n");
			*peak = syll[1];
			*dist += 1;
			//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
			//word = realloc_same_add(data, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
			Push(x, *dist - 1, word);
		}
		else
		{
			if (fabs(*peak - *lowPeak1) > 15) {
				//printf("case 1 cond 0\n");
				*lowPeak2 = syll[1];
				*cond_flag = 2;
				*dist += 1;
				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
				Push(x, *dist - 1, word);
			}
			else
			{
				//printf("case 1 cond 1\n");
				*cond_flag = 0;
				*dist = 0;
				*lowPeak1 = syll[1];
				/*free(word);
				word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);*/
				//word = (float *)realloc(word, sizeof(float) * FRAMES_PER_BUFFER);
			}
		}
		break;
	case 2:
		if (*lowPeak2 > syll[1]) {
			//printf("case 2 cond 0\n");
			*lowPeak2 = syll[1];
			*dist += 1;
			//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
			//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
			Push(x, *dist - 1, word);
		}
		else
		{
			if (fabs(*peak - *lowPeak2) > 15 && *dist >= 13) {
				//printf("case 2 con 1\n");

				*lowPeak1 = syll[1];
				*dist += 1;
				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
				Push(x, *dist - 1, word);
				start = PerformanceCounter();
				Pa_AbortStream(stream);
				write_to_syll(d_word, def_name, ext, path, dist, word, model, sum_normal, fbank);
				double dftDuration3 = (double)(PerformanceCounter() - start) * 1000.0 / (double)Frequency.QuadPart;
				if (dftDuration3 > 0.1)
					printf("WRITE_TO" ": %f\n", dftDuration3);
				Pa_StartStream(stream);
				///*free(word);
				//word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);
				//word = (float *)realloc(word, sizeof(float) * FRAMES_PER_BUFFER);
				*dist = 0;
				*cond_flag = 0;
				succeed = 0;
			}
			else if (fabs(*peak - *lowPeak2) > 12 && *dist <= 18 || *dist > 150) {
				printf("EXCEPTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");
				*dist = 0;
				*cond_flag = 0;
			}
			else
			{
				//printf("case 2 con 2\n");
				*peak = syll[1];
				*cond_flag = 3;
				*dist += 1;
				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
				Push(x, *dist - 1, word);
			}
		}
		break;
	case 3:
		if (*peak < syll[1]) {
			//printf("case 3 cond 1\n");
			*peak = syll[1];
		}
		else
		{
			//printf("case 3 cond 2\n");
			*lowPeak2 = syll[1];
			*cond_flag = 2;
		}
		*dist += 1;
		//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
		//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
		Push(x, *dist - 1, word);
		break;
	default:
		break;
	}
	//free(temp_data);
	free(db);
	return succeed;
}

void write_to_syll(int *d_word, char *def_name, char *ext, char*path, int *dist, float *word, struct svm_model *model, SAMPLE *sum_normal, filter_bank fbank) {
	/*LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	start = PerformanceCounter();
	*/
	SIGNAL a = setSignal2(word, (*dist)*FRAMES_PER_BUFFER);

	/*double dftDuration3 = (double)(PerformanceCounter() - start) * 1000.0 / (double)Frequency.QuadPart;
	if (dftDuration3 > 0.1)
		printf("WRITE_TO" ": %f\n", dftDuration3);*/
	int temp = predict_test_one_time(a, path, 0, model, sum_normal, fbank);
	if (temp == 1) {
		printf("tu\n");
		sent_buff[*d_word] = 1;
	}
	else if (temp == 2) {
		printf("mo\n");
		sent_buff[*d_word] = 2;
	}
	else if (temp == 3) {
		printf("cua\n");
		sent_buff[*d_word] = 3;
	}
	else if (temp == 4)
	{
		printf("truoc\n");
		sent_buff[*d_word] = 4;
	}
	else if (temp == 5)
	{
		printf("ra\n");
		sent_buff[*d_word] = 5;
	}
	else {
		printf("non-key\n");
		sent_buff[*d_word] = 6;
	}
	*d_word += 1;
}

void real_time_predict(struct svm_model *model, SAMPLE *sum_normal, char *def_path, char *sent_path) {
	filter_bank fbank = filterbank(26, 512);
	sent_buff = (int*)malloc(sizeof(int) * 7);
	int order = 2;
	float *A = (float *)malloc(sizeof(float) * order);
	float *d1 = (float *)malloc(sizeof(float) * order);
	float *d2 = (float *)malloc(sizeof(float) * order);
	float *d3 = (float *)malloc(sizeof(float) * order);
	float *d4 = (float *)malloc(sizeof(float) * order);
	float *x = (float *)malloc(sizeof(float) * QUEUE_SIZE);
	float *w0 = (float *)calloc(order, sizeof(float));
	float *w1 = (float *)calloc(order, sizeof(float));
	float *w2 = (float *)calloc(order, sizeof(float));
	float *w3 = (float *)calloc(order, sizeof(float));
	float *w4 = (float *)calloc(order, sizeof(float));
	LARGE_INTEGER Frequency;

	float *queue = (float *)malloc(sizeof(float) * QUEUE_SIZE);
	float *word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);
	char *wtemp = (char*)malloc(sizeof(char) * 8);
	int trim_ms = 0;
	int offset = FRAMES_PER_BUFFER;
	int flag = 1;
	int dem = 0;
	int time = 1;
	int cond_flag = 0;
	int p_word = 0;
	float peak;
	float syll[2];
	int dist = 0;
	float lowPeak1;
	float lowPeak2;
	int d_word = 0;
	int temp = 1;
	char *def_name = "syllabic";
	char *ext = ".txt";
	///////////////////////////
	///////////////////////////
	int get_data_time = 0;
	PaError err = paNoError;
	if ((err = Pa_Initialize())) goto done;
	const PaDeviceInfo *info = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
	//AudioData data = initAudioData(16000, 1, paFloat32);
	AudioSnippet sampleBlock =
	{
		.snippet = NULL,
		.size = FRAMES_PER_BUFFER * sizeof(float)
	};
	PaStream *stream = NULL;
	sampleBlock.snippet = (float *)malloc(sampleBlock.size);
	PaStreamParameters inputParameters =
	{
		.device = Pa_GetDefaultInputDevice(),
		.channelCount = 1,
		.sampleFormat = paFloat32,
		.suggestedLatency = info->defaultHighInputLatency,
		.hostApiSpecificStreamInfo = NULL
	};

	if (err = Pa_OpenStream(&stream, &inputParameters, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, NULL, NULL)) goto done;
	if (err = Pa_StartStream(stream)) goto done;

	QueryPerformanceFrequency(&Frequency);
	int demtemp = 0;
	int timer = 0;
	for (int i = 0;;) {
		//PRINT_TIME_SAMPLE_START(start);
		err = Pa_ReadStream(stream, sampleBlock.snippet, FRAMES_PER_BUFFER);
		//PRINT_TIME_SAMPLE_STOP("SAMPLE: ",10);

		if (err) goto done;
		else {

			if (trim_ms < QUEUE_SIZE) {
				for (int j = trim_ms, k = 0; j < trim_ms + offset; ++j) {
					queue[j] = sampleBlock.snippet[k];
					k++;
				}
			}
			else {
				for (int j = FRAMES_PER_BUFFER; j < QUEUE_SIZE; ++j) {
					queue[j - FRAMES_PER_BUFFER] = queue[j];
				}
				for (int j = 0; j < FRAMES_PER_BUFFER; ++j) {
					queue[QUEUE_SIZE - FRAMES_PER_BUFFER + j] = sampleBlock.snippet[j];
				}
			}

			if (trim_ms < QUEUE_SIZE) {
				trim_ms += offset;
				if (trim_ms < QUEUE_SIZE) {
					continue;
				}
				else {
					PRINT_TIME_PROCESS_START(start);
					silence_detect(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, def_name, ext, def_path, A, d1, d2, d3, d4,
						w0, w1, w2, w3, w4, x, model, sum_normal, fbank, stream);
					PRINT_TIME_PROCESS_STOP(start, "Total1", 1);
					if (d_word == 1) {
						p_word = d_word;
						timer = 1;
					}
				}
			}
			else
			{
				PRINT_TIME_PROCESS_START(start);
				temp = silence_detect(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, def_name, ext, def_path, A, d1, d2,
					d3, d4, w0, w1, w2, w3, w4, x, model, sum_normal, fbank, stream);
				PRINT_TIME_PROCESS_STOP(start, "Total2", 1);
				if (d_word == 1) {
					p_word = d_word;
					timer = 1;
				}
				if (check_word(d_word, p_word) && tdem < 100) {
					p_word = d_word;
					timer = 1;
					tdem = 0;
				}
				if (timer) {
					tdem++;
					if (tdem > 100) {
						check_sentence_formation(sent_path, ext, d_word, wtemp);
						d_word = p_word = 0;
						tdem = 0;
						timer = 0;
						demtemp = 0;
					}
				}
			}
		}
	}
done:
	svm_free_and_destroy_model(model);
	free(sampleBlock.snippet);
	free(queue);
	free(word);
	free(A);
	free(d1);
	free(d2);
	free(d3);
	free(d4);
	free(w0);
	free(w1);
	free(w2);
	free(w3);
	free(w4);
	free(x);
	Pa_Terminate();

}


int check_word(int word, int pword) {
	if (word != pword)
		return 1;
	return 0;
}

inline long long PerformanceCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}




//void real_time_predict2(struct svm_model *model, SAMPLE *sum_normal, char *def_path, char *sent_path) {
//	PaStreamParameters  inputParameters,
//		outputParameters;
//	PaStream*           stream;
//	PaError             err = paNoError;
//	paTestData          data;
//	int                 i;
//	int                 totalFrames;
//	int                 numSamples;
//	int                 numBytes;
//	SAMPLE              max, val;
//	double              average;
//	filter_bank fbank = filterbank(26, 512);
//	sent_buff = (int*)malloc(sizeof(int) * 7);
//	int order = 2;
//	float *A = (float *)malloc(sizeof(float) * order);
//	float *d1 = (float *)malloc(sizeof(float) * order);
//	float *d2 = (float *)malloc(sizeof(float) * order);
//	float *d3 = (float *)malloc(sizeof(float) * order);
//	float *d4 = (float *)malloc(sizeof(float) * order);
//	float *x = (float *)malloc(sizeof(float) * QUEUE_SIZE);
//	float *w0 = (float *)calloc(order, sizeof(float));
//	float *w1 = (float *)calloc(order, sizeof(float));
//	float *w2 = (float *)calloc(order, sizeof(float));
//	float *w3 = (float *)calloc(order, sizeof(float));
//	float *w4 = (float *)calloc(order, sizeof(float));
//	LARGE_INTEGER Frequency;
//
//	float *queue = (float *)malloc(sizeof(float) * QUEUE_SIZE);
//	float *word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);
//	char *wtemp = (char*)malloc(sizeof(char) * 8);
//	int trim_ms = 0;
//	int offset = FRAMES_PER_BUFFER;
//	int flag = 1;
//	int dem = 0;
//	int time = 1;
//	int cond_flag = 0;
//	int p_word = 0;
//	float peak;
//	float syll[2];
//	int dist = 0;
//	float lowPeak1;
//	float lowPeak2;
//	int d_word = 0;
//	int temp = 1;
//	char *def_name = "syllabic";
//	char *ext = ".txt";
//
//
//	data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
//	data.frameIndex = 0;
//	numSamples = totalFrames * NUM_CHANNELS;
//	numBytes = numSamples * sizeof(SAMPLE);
//	data.recordedSamples = (SAMPLE *)malloc(numBytes); /* From now on, recordedSamples is initialised. */
//	if (data.recordedSamples == NULL)
//	{
//		printf("Could not allocate record array.\n");
//		goto done;
//	}
//	for (i = 0; i<numSamples; i++)
//		data.recordedSamples[i] = 0;
//
//	err = Pa_Initialize();
//	if (err != paNoError) goto done;
//
//	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
//	if (inputParameters.device == paNoDevice) {
//		fprintf(stderr, "Error: No default input device.\n");
//		goto done;
//	}
//	inputParameters.channelCount = 1;                    /* stereo input */
//	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
//	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
//	inputParameters.hostApiSpecificStreamInfo = NULL;
//
//	/* Record some audio. -------------------------------------------- */
//	err = Pa_OpenStream(
//		&stream,
//		&inputParameters,
//		NULL,                  /* &outputParameters, */
//		SAMPLE_RATE,
//		FRAMES_PER_BUFFER,
//		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
//		recordCallback,
//		&data);
//	if (err != paNoError) goto done;
//
//	err = Pa_StartStream(stream);
//	if (err != paNoError) goto done;
//	printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);
//
//	QueryPerformanceFrequency(&Frequency);
//	int demtemp = 0;
//	int timer = 0;
//
//	while ((err = Pa_IsStreamActive(stream)) == 1)
//	{
//		if (trim_ms < QUEUE_SIZE) {
//			for (int j = trim_ms, k = 0; j < trim_ms + offset; ++j) {
//				queue[j] = data.recordedSamples[k];
//				k++;
//			}
//		}
//		else {
//			for (int j = FRAMES_PER_BUFFER; j < QUEUE_SIZE; ++j) {
//				queue[j - FRAMES_PER_BUFFER] = queue[j];
//			}
//			for (int j = 0; j < FRAMES_PER_BUFFER; ++j) {
//				queue[QUEUE_SIZE - FRAMES_PER_BUFFER + j] = data.recordedSamples[j];
//			}
//		}
//
//		if (trim_ms < QUEUE_SIZE) {
//			trim_ms += offset;
//			if (trim_ms < QUEUE_SIZE) {
//				continue;
//			}
//			else {
//				PRINT_TIME_PROCESS_START(start);
//				silence_detect(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, def_name, ext, def_path, A, d1, d2, d3, d4,
//					w0, w1, w2, w3, w4, x, model, sum_normal, fbank,stream);
//				PRINT_TIME_PROCESS_STOP(start, "Total1", 1);
//				if (d_word == 1) {
//					p_word = d_word;
//					timer = 1;
//				}
//			}
//		}
//		else
//		{
//			PRINT_TIME_PROCESS_START(start);
//			temp = silence_detect(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, def_name, ext, def_path, A, d1, d2,
//				d3, d4, w0, w1, w2, w3, w4, x, model, sum_normal, fbank,stream);
//			PRINT_TIME_PROCESS_STOP(start, "Total2", 1);
//			if (d_word == 1) {
//				p_word = d_word;
//				timer = 1;
//			}
//			if (check_word(d_word, p_word) && tdem < 100) {
//				p_word = d_word;
//				timer = 1;
//				tdem = 0;
//			}
//			if (timer) {
//				tdem++;
//				if (tdem > 100) {
//					check_sentence_formation(sent_path, ext, d_word, wtemp);
//					d_word = p_word = 0;
//					tdem = 0;
//					timer = 0;
//					demtemp = 0;
//				}
//			}
//		}
//	}
//	if (err < 0) goto done;
//
//	err = Pa_CloseStream(stream);
//	if (err != paNoError) goto done;
//
//done:
//	Pa_Terminate();
//	if (data.recordedSamples)       /* Sure it is NULL or valid. */
//		free(data.recordedSamples);
//	if (err != paNoError)
//	{
//		fprintf(stderr, "An error occured while using the portaudio stream\n");
//		fprintf(stderr, "Error number: %d\n", err);
//		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
//		err = 1;          /* Always return 0 or 1, but no other return codes. */
//	}
//	scanf("%d", &temp);
//	svm_free_and_destroy_model(model);
//	free(data.recordedSamples);
//	free(queue);
//	free(word);
//	free(A);
//	free(d1);
//	free(d2);
//	free(d3);
//	free(d4);
//	free(w0);
//	free(w1);
//	free(w2);
//	free(w3);
//	free(w4);
//	free(x);
//}
//
///* This routine is run in a separate thread to write data from the ring buffer into a file (during Recording) */
//static int threadFunctionWriteToRawFile(void* ptr)
//{
//	paTestData* pData = (paTestData*)ptr;
//
//	/* Mark thread started */
//	pData->threadSyncFlag = 0;
//
//	while (1)
//	{
//		ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferReadAvailable(&pData->ringBuffer);
//		if ((elementsInBuffer >= pData->ringBuffer.bufferSize / NUM_WRITES_PER_BUFFER) ||
//			pData->threadSyncFlag)
//		{
//			void* ptr[2] = { 0 };
//			ring_buffer_size_t sizes[2] = { 0 };
//
//			/* By using PaUtil_GetRingBufferReadRegions, we can read directly from the ring buffer */
//			ring_buffer_size_t elementsRead = PaUtil_GetRingBufferReadRegions(&pData->ringBuffer, elementsInBuffer, ptr + 0, sizes + 0, ptr + 1, sizes + 1);
//			if (elementsRead > 0)
//			{
//				int i;
//				PaUtil_AdvanceRingBufferReadIndex(&pData->ringBuffer, elementsRead);
//			}
//
//			if (pData->threadSyncFlag)
//			{
//				break;
//			}
//		}
//
//		/* Sleep a little while... */
//	}
//
//	pData->threadSyncFlag = 0;
//
//	return 0;
//}
//
///* This routine is run in a separate thread to read data from file into the ring buffer (during Playback). When the file
//has reached EOF, a flag is set so that the play PA callback can return paComplete */
//static int threadFunctionReadFromRawFile(void* ptr)
//{
//	paTestData* pData = (paTestData*)ptr;
//
//	while (1)
//	{
//		ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferWriteAvailable(&pData->ringBuffer);
//
//		if (elementsInBuffer >= pData->ringBuffer.bufferSize / NUM_WRITES_PER_BUFFER)
//		{
//			void* ptr[2] = { 0 };
//			ring_buffer_size_t sizes[2] = { 0 };
//
//			/* By using PaUtil_GetRingBufferWriteRegions, we can write directly into the ring buffer */
//			PaUtil_GetRingBufferWriteRegions(&pData->ringBuffer, elementsInBuffer, ptr + 0, sizes + 0, ptr + 1, sizes + 1);
//
//			if (!feof(pData->file))
//			{
//				ring_buffer_size_t itemsReadFromFile = 0;
//				int i;
//				for (i = 0; i < 2 && ptr[i] != NULL; ++i)
//				{
//					itemsReadFromFile += (ring_buffer_size_t)fread(ptr[i], pData->ringBuffer.elementSizeBytes, sizes[i], pData->file);
//				}
//				PaUtil_AdvanceRingBufferWriteIndex(&pData->ringBuffer, itemsReadFromFile);
//
//				/* Mark thread started here, that way we "prime" the ring buffer before playback */
//				pData->threadSyncFlag = 0;
//			}
//			else
//			{
//				/* No more data to read */
//				pData->threadSyncFlag = 1;
//				break;
//			}
//		}
//
//		/* Sleep a little while... */
//		Pa_Sleep(20);
//	}
//
//	return 0;
//}
//
//
///* Start up a new thread in the given function, at the moment only Windows, but should be very easy to extend
//to posix type OSs (Linux/Mac) */
//static PaError startThread(paTestData* pData, ThreadFunctionType fn)
//{
//#ifdef _WIN64
//	typedef unsigned(__stdcall* WinThreadFunctionType)(void*);
//	pData->threadHandle = (void*)_beginthreadex(NULL, 0, (WinThreadFunctionType)fn, pData, CREATE_SUSPENDED, NULL);
//	if (pData->threadHandle == NULL) return paUnanticipatedHostError;
//
//	/* Set file thread to a little higher prio than normal */
//	SetThreadPriority(pData->threadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
//
//	/* Start it up */
//	pData->threadSyncFlag = 1;
//	ResumeThread(pData->threadHandle);
//
//#endif
//
//	/* Wait for thread to startup */
//	while (pData->threadSyncFlag) {
//		Pa_Sleep(5);
//	}
//
//	return paNoError;
//}
//
//static int stopThread(paTestData* pData)
//{
//	pData->threadSyncFlag = 1;
//	/* Wait for thread to stop */
//	while (pData->threadSyncFlag) {
//		Pa_Sleep(5);
//	}
//#ifdef _WIN64
//	CloseHandle(pData->threadHandle);
//	pData->threadHandle = 0;
//#endif
//
//	return paNoError;
//}
//
//
///* This routine will be called by the PortAudio engine when audio is needed.
//** It may be called at interrupt level on some machines so don't do anything
//** that could mess up the system like calling malloc() or free().
//*/
//static int recordCallback(const void *inputBuffer, void *outputBuffer,
//	unsigned long framesPerBuffer,
//	const PaStreamCallbackTimeInfo* timeInfo,
//	PaStreamCallbackFlags statusFlags,
//	void *userData)
//{
//	paTestData *data = (paTestData*)userData;
//	ring_buffer_size_t elementsWriteable = PaUtil_GetRingBufferWriteAvailable(&data->ringBuffer);
//	ring_buffer_size_t elementsToWrite = rbs_min(elementsWriteable, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
//	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
//
//	(void)outputBuffer; /* Prevent unused variable warnings. */
//	(void)timeInfo;
//	(void)statusFlags;
//	(void)userData;
//
//	data->frameIndex += PaUtil_WriteRingBuffer(&data->ringBuffer, rptr, elementsToWrite);
//
//	return paContinue;
//}
//
///* This routine will be called by the PortAudio engine when audio is needed.
//** It may be called at interrupt level on some machines so don't do anything
//** that could mess up the system like calling malloc() or free().
//*/
//static int playCallback(const void *inputBuffer, void *outputBuffer,
//	unsigned long framesPerBuffer,
//	const PaStreamCallbackTimeInfo* timeInfo,
//	PaStreamCallbackFlags statusFlags,
//	void *userData)
//{
//	paTestData *data = (paTestData*)userData;
//	ring_buffer_size_t elementsToPlay = PaUtil_GetRingBufferReadAvailable(&data->ringBuffer);
//	ring_buffer_size_t elementsToRead = rbs_min(elementsToPlay, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
//	SAMPLE* wptr = (SAMPLE*)outputBuffer;
//
//	(void)inputBuffer; /* Prevent unused variable warnings. */
//	(void)timeInfo;
//	(void)statusFlags;
//	(void)userData;
//
//	data->frameIndex += PaUtil_ReadRingBuffer(&data->ringBuffer, wptr, elementsToRead);
//
//	return data->threadSyncFlag ? paComplete : paContinue;
//}
//
//static unsigned NextPowerOf2(unsigned val)
//{
//	val--;
//	val = (val >> 1) | val;
//	val = (val >> 2) | val;
//	val = (val >> 4) | val;
//	val = (val >> 8) | val;
//	val = (val >> 16) | val;
//	return ++val;
//}
//
//void real_time_predict3(struct svm_model *model, SAMPLE *sum_normal, char *def_path, char *sent_path) {
//	filter_bank fbank = filterbank(26, 512);
//	sent_buff = (int*)malloc(sizeof(int) * 7);
//	int order = 2;
//	float *A = (float *)malloc(sizeof(float) * order);
//	float *d1 = (float *)malloc(sizeof(float) * order);
//	float *d2 = (float *)malloc(sizeof(float) * order);
//	float *d3 = (float *)malloc(sizeof(float) * order);
//	float *d4 = (float *)malloc(sizeof(float) * order);
//	float *x = (float *)malloc(sizeof(float) * QUEUE_SIZE);
//	float *w0 = (float *)calloc(order, sizeof(float));
//	float *w1 = (float *)calloc(order, sizeof(float));
//	float *w2 = (float *)calloc(order, sizeof(float));
//	float *w3 = (float *)calloc(order, sizeof(float));
//	float *w4 = (float *)calloc(order, sizeof(float));
//	LARGE_INTEGER Frequency;
//	float *queue = (float *)malloc(sizeof(float) * QUEUE_SIZE);
//	float *word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);
//	char *wtemp = (char*)malloc(sizeof(char) * 8);
//	int trim_ms = 0;
//	int offset = FRAMES_PER_BUFFER;
//	int flag = 1;
//	int dem = 0;
//	int time = 1;
//	int cond_flag = 0;
//	int p_word = 0;
//	float peak;
//	float syll[2];
//	int dist = 0;
//	float lowPeak1;
//	float lowPeak2;
//	int d_word = 0;
//	int temp = 1;
//	char *def_name = "syllabic";
//	char *ext = ".txt";
//
//
//	PaStreamParameters  inputParameters,
//		outputParameters;
//	PaStream*           stream;
//	PaError             err = paNoError;
//	paTestData          data = { 0 };
//	unsigned            delayCntr;
//	unsigned            numSamples;
//	unsigned            numBytes;
//
//	printf("patest_record.c\n"); fflush(stdout);
//
//	/* We set the ring buffer size to about 500 ms */
//	numSamples = NextPowerOf2((unsigned)(SAMPLE_RATE * 0.01 * NUM_CHANNELS));
//	numBytes = numSamples * sizeof(SAMPLE);
//	data.ringBufferData = (SAMPLE *)PaUtil_AllocateMemory(numBytes);
//	if (data.ringBufferData == NULL)
//	{
//		printf("Could not allocate ring buffer data.\n");
//		goto done;
//	}
//
//	if (PaUtil_InitializeRingBuffer(&data.ringBuffer, sizeof(SAMPLE), numSamples, data.ringBufferData) < 0)
//	{
//		printf("Failed to initialize ring buffer. Size is not power of 2 ??\n");
//		goto done;
//	}
//
//	err = Pa_Initialize();
//	if (err != paNoError) goto done;
//
//	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
//	if (inputParameters.device == paNoDevice) {
//		fprintf(stderr, "Error: No default input device.\n");
//		goto done;
//	}
//	inputParameters.channelCount = 1;                    /* stereo input */
//	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
//	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
//	inputParameters.hostApiSpecificStreamInfo = NULL;
//
//	/* Record some audio. -------------------------------------------- */
//	err = Pa_OpenStream(
//		&stream,
//		&inputParameters,
//		NULL,                  /* &outputParameters, */
//		SAMPLE_RATE,
//		FRAMES_PER_BUFFER,
//		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
//		recordCallback,
//		&data);
//	if (err != paNoError) goto done;
//
//	/* Open the raw audio 'cache' file... */
//	data.file = fopen(FILE_NAME, "wb");
//	if (data.file == 0) goto done;
//
//	/* Start the file writing thread */
//	err = startThread(&data, threadFunctionWriteToRawFile);
//	if (err != paNoError) goto done;
//
//	err = Pa_StartStream(stream);
//	if (err != paNoError) goto done;
//	printf("\n=== Now recording to '" FILE_NAME "' for %d seconds!! Please speak into the microphone. ===\n", NUM_SECONDS); fflush(stdout);
//
//	/* Note that the RECORDING part is limited with TIME, not size of the file and/or buffer, so you can
//	increase NUM_SECONDS until you run out of disk */
//	delayCntr = 0;
//	while (1)
//	{
//		printf("index = %d\n", data.frameIndex); fflush(stdout);
//		Pa_Sleep(10);
//	}
//	if (err < 0) goto done;
//
//	err = Pa_CloseStream(stream);
//	if (err != paNoError) goto done;
//
//	/* Stop the thread */
//	err = stopThread(&data);
//	if (err != paNoError) goto done;
//
//	/* Close file */
//	fclose(data.file);
//	data.file = 0;
//
//
//done:
//	Pa_Terminate();
//	if (data.ringBufferData)       /* Sure it is NULL or valid. */
//		PaUtil_FreeMemory(data.ringBufferData);
//	if (err != paNoError)
//	{
//		fprintf(stderr, "An error occured while using the portaudio stream\n");
//		fprintf(stderr, "Error number: %d\n", err);
//		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
//		err = 1;          /* Always return 0 or 1, but no other return codes. */
//	}
//	svm_free_and_destroy_model(model);
//	free(queue);
//	free(word);
//	free(A);
//	free(d1);
//	free(d2);
//	free(d3);
//	free(d4);
//	free(w0);
//	free(w1);
//	free(w2);
//	free(w3);
//	free(w4);
//	free(x);
//}