#include "syll_fragmentation.h"

void check_sentence_formation(char *path, char *ext, int sent_len, char *wtemp, int num_of_sents) {
	char *label = (char *)malloc(sizeof(char) * 2);
	const char *default_ext = ".txt";
	size_t len_path_tmp = strlen(path) + strlen(label) + 5;
	char *temp = (char*)malloc(sizeof(char) * len_path_tmp);
	FILE *fsent;
	int num, dtemp, succeed = 0;

	for (int i = 0; i < num_of_sents; i++) {
		sprintf(label, "%d", i + 1);
		strcpy(temp, path);
		strcat(temp, "s_");
		strcat(temp, label);
		strcat(temp, default_ext);

		fsent = fopen(temp, "r");

		if (fsent == NULL) {
			printf("FILE DOESN'T EXIST!!");
			return;
		}
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
				sent_buff[i] = 0;
			}
			fscanf(fsent, "%s", wtemp);
			printf("%s", wtemp);
#ifdef WIND
			startThread2(wtemp, play_sound);
#elif DUNIX
			pthread_t thread1;
			int  iret1, iret2;

			/* Create independent threads each of which will execute function */
			iret1 = pthread_create(&thread1, NULL, print_message_function, wtemp);
			if (iret1)
			{
				fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
				exit(EXIT_FAILURE);
			}
#endif
			fclose(fsent);
			return;
		}
		fclose(fsent);
	}
	if (succeed == 1) {
		printf("INVALID SENTENCE!! ");	//return true
		//startThread2(".\\sentences\\invalid.wav", play_sound);
	}

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
			if (index * 160 < (MAX_WORD_BUFFER_RECORD * FRAMES_PER_BUFFER)) {
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

void Push2(float *data, int index, float *word, SAMPLE *final_feats, hyper_vector fbank, hyper_vector temp_feats) {
	int dem = 480;
	if (index == 0) {
		for (int i = 0; i < 160; ++i) {
			word[i] = data[dem];
			dem++;
		}
	}
	else if (index < 2)
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
	else if (index == 2) {
		for (int i = index * 160; i < (index + 1) * 160; i++) {
			word[i] = data[dem];
			dem++;
		}
		for (int i = 0; i < 400; ++i) {
			temp_feats.data[i] = word[i];
		}
		add_to_final(final_feats, get_feature_vector_from_signal2(temp_feats, fbank), index - 2);
	}
	else if (index > 2) {
		for (int i = 160; i < 480; ++i) {
			word[i - 160] = word[i];
			temp_feats.data[i - 160] = word[i - 160];
		}
		for (int i = 320; i < 480; i++) {
			word[i] = data[dem];
			if (i < 400) {
				temp_feats.data[i] = word[i];
			}
			dem++;
		}
		add_to_final(final_feats, get_feature_vector_from_signal2(temp_feats, fbank), index - 2);
		/*for (int i = index; i < index + 13; ++i) {
			printf("%f ", final_feats[i]);
		}
		printf("\n");*/
	}
}

void add_to_final(SAMPLE *final_feats, hyper_vector temp, int num_feats) {
	int dem = 0;
	for (int i = num_feats * RAW_FEAT_SIZE; i < (num_feats + 1)*RAW_FEAT_SIZE; i++) {
		final_feats[i] = temp.data[dem];
		//printf("%f ", final_feats.data[i]);
		dem++;
	}
	//printf("\n");
}

int silence_detect(float *data, size_t length, int *time, int *cond_flag, int *dist, float *word, float *peak, float *syll, float *lowPeak1, float *lowPeak2,
	int *d_word, char *def_name, char *ext, char *path, float *A, float *d1, float *d2, float *d3, float *d4, float *w0, float *w1, float *w2, float *w3,
	float *w4, float *x, struct svm_model *model, SAMPLE *sum_normal, hyper_vector fbank, PaStream *stream, char **words, SAMPLE *final_feats, hyper_vector temp_feats) {
	int chunk_size = 160;
	float sum = 0;
	int trim_ms = 0;
	int dem = 0;
	int succeed = 1;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	//start = PerformanceCounter();

	x = butterworth_bandpass_v2(2, data, length, 16000, 4000, 500, A, d1, d2, d3, d4, w0, w1, w2, w3, w4, x);
	//double dftDuration3 = (double)(PerformanceCounter() - start) * 1000.0 / (double)Frequency.QuadPart;
	//if (dftDuration3 > 0.1)
	//	printf("WRITE_TO" ": %f\n", dftDuration3);

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
				Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);

				*time = 0;
			}
			else
			{
				//Push(x, *dist - 1, word);
				Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);
				*time = 0;
			}
		}
		else if (*lowPeak1 >= syll[1]) {
			//printf("case 0, cond 1\n");
			*lowPeak1 = syll[1];
		}
		break;
	case 1:
		if (*peak < syll[1]) {
			//printf("case1 cond 0.0\n");
			*peak = syll[1];
			*dist += 1;
			//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
			//word = realloc_same_add(data, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
			//Push(x, *dist - 1, word);
			Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);

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
				//Push(x, *dist - 1, word);
				Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);

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
			//Push(x, *dist - 1, word);
			Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);

		}
		else
		{
			if (fabs(*peak - *lowPeak2) > 15 && *dist >= 13) {
				//printf("case 2 con 1\n");

				*lowPeak1 = syll[1];
				*dist += 1;
				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
				//Push(x, *dist - 1, word);
				//start = PerformanceCounter();

				Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);
				//double dftDuration3 = (double)(PerformanceCounter() - start) * 1000.0 / (double)Frequency.QuadPart;

				//if (dftDuration3 > 0.1)
				//	printf("WRITE_TO" ": %f\n", dftDuration3);
				//for (int i = *dist - 1; i < *dist - 1 + 13; ++i) {
				//	printf("%f ", final_feats[i]);
				//}
				//printf("\n");
				write_to_syll2(d_word, def_name, ext, path, dist, final_feats, model, sum_normal, fbank, words);

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
				//Push(x, *dist - 1, word);
				Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);

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
		//Push(x, *dist - 1, word);
		Push2(x, *dist - 1, word, final_feats, fbank, temp_feats);

		break;
	default:
		break;
	}
	//free(temp_data);
	free(db);
	return succeed;
}

void write_to_syll(int *d_word, char *def_name, char *ext, char*path, int *dist, float *word, struct svm_model *model, SAMPLE *sum_normal
	, hyper_vector fbank, char **words) {
	/*LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	start = PerformanceCounter();
	*/
	SIGNAL a = setSignal2(word, (*dist)*FRAMES_PER_BUFFER);

	/*double dftDuration3 = (double)(PerformanceCounter() - start) * 1000.0 / (double)Frequency.QuadPart;
	if (dftDuration3 > 0.1)
		printf("WRITE_TO" ": %f\n", dftDuration3);*/
	int temp = predict_test_one_time(a, path, 0, model, sum_normal, fbank);

	printf("%s\n", words[temp - 1]);
	sent_buff[*d_word] = temp;
	*d_word += 1;
}

void write_to_syll2(int *d_word, char *def_name, char *ext, char*path, int *dist, SAMPLE *final_feats, struct svm_model *model, SAMPLE *sum_normal
	, hyper_vector fbank, char **words) {
	/*LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	start = PerformanceCounter();
	*/
	//SIGNAL a = setSignal2(word, (*dist)*FRAMES_PER_BUFFER);

	/*double dftDuration3 = (double)(PerformanceCounter() - start) * 1000.0 / (double)Frequency.QuadPart;
	if (dftDuration3 > 0.1)
	printf("WRITE_TO" ": %f\n", dftDuration3);*/

	//for (int i = 0; i < *dist-3; i++) {
	//	for (int j = 0; j <  RAW_FEAT_SIZE; j++) {
	//		printf("%f ", final_feats[i*RAW_FEAT_SIZE +j]);
	//	}
	//	printf("\n");
	//}
	hyper_vector compact_final_feats = cov(setHVector2(final_feats, RAW_FEAT_SIZE, *dist - 2, 1));

	int temp = predict_test_one_time(compact_final_feats, path, 0, model, sum_normal, fbank);

	printf("%s\n", words[temp - 1]);
	sent_buff[*d_word] = temp;
	*d_word += 1;
}

void real_time_predict(struct svm_model *model, SAMPLE *sum_normal, char *def_path, char *sent_path, int num_of_sents, char **words) {
	//creat Mel Filter banks
	filter_bank fbank = filterbank(26, 512);
	hyper_vector transpose_param = setHVector2(fbank.data, fbank.filt_len, fbank.nfilt, 1);		//26x257
	hyper_vector tmp = transpose(transpose_param);
	// initialize feature hyper_vector
	hyper_vector final_feats = setEHVector(RAW_FEAT_SIZE, MAX_FEATS_BUFFER, 1);

	hyper_vector temp_feats;
	temp_feats.row = 1;
	temp_feats.col = 400;
	temp_feats.dim = 1;
	temp_feats.data = (SAMPLE*)malloc(sizeof(SAMPLE) * 400);
	//

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
					silence_detect(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, def_name, ext, def_path, A, d1, d2, d3, d4,
						w0, w1, w2, w3, w4, x, model, sum_normal, tmp, stream, words, final_feats.data, temp_feats);
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
					d3, d4, w0, w1, w2, w3, w4, x, model, sum_normal, tmp, stream, words, final_feats.data, temp_feats);
				PRINT_TIME_PROCESS_STOP(start, "Total2", 0.5);
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
						check_sentence_formation(sent_path, ext, d_word, wtemp, num_of_sents);
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


/* This routine is run in a separate thread to read data from file into the ring buffer (during Playback). When the file
has reached EOF, a flag is set so that the play PA callback can return paComplete */
static int threadFunctionReadFromRawFile(void* ptr)
{
	paTestData* pData = (paTestData*)ptr;

	while (1)
	{
		ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferWriteAvailable(&pData->ringBuffer);

		if (elementsInBuffer >= pData->ringBuffer.bufferSize / NUM_WRITES_PER_BUFFER)
		{
			void* ptr[2] = { 0 };
			ring_buffer_size_t sizes[2] = { 0 };

			/* By using PaUtil_GetRingBufferWriteRegions, we can write directly into the ring buffer */
			PaUtil_GetRingBufferWriteRegions(&pData->ringBuffer, elementsInBuffer, ptr + 0, sizes + 0, ptr + 1, sizes + 1);

			if (!feof(pData->file))
			{
				ring_buffer_size_t itemsReadFromFile = 0;
				int i;
				for (i = 0; i < 2 && ptr[i] != NULL; ++i)
				{
					itemsReadFromFile += (ring_buffer_size_t)fread(ptr[i], pData->ringBuffer.elementSizeBytes, sizes[i], pData->file);
				}
				PaUtil_AdvanceRingBufferWriteIndex(&pData->ringBuffer, itemsReadFromFile);

				/* Mark thread started here, that way we "prime" the ring buffer before playback */
				pData->threadSyncFlag = 0;
			}
			else
			{
				/* No more data to read */
				pData->threadSyncFlag = 1;
				break;
			}
		}

		/* Sleep a little while... */
		Pa_Sleep(20);
	}

	return 0;
}


///* Start up a new thread in the given function, at the moment only Windows, but should be very easy to extend
//to posix type OSs (Linux/Mac) */
//static PaError startThread(paTestData* pData, float *queue, char *sent_path, char *wtemp, int *demtemp, int *trim_ms, int offset, int *p_word, int *timer, int *time, int *cond_flag, int *dist, float *word, float *peak, float *syll, float *lowPeak1, float *lowPeak2,
//	int *d_word, char *def_name, char *ext, char *def_path, float *A, float *d1, float *d2, float *d3, float *d4, float *w0, float *w1, float *w2, float *w3, float *w4, float *x, struct svm_model *model, SAMPLE *sum_normal, filter_bank fbank, LARGE_INTEGER Frequency, PaStream* stream, ThreadFunctionType fn)
//{
//#ifdef _WIN64
//	int a, b, c, d, e;
//	typedef unsigned(__stdcall* WinThreadFunctionType)(void*);
//	pData->threadHandle = (void*)_beginthreadex(NULL, 0, (WinThreadFunctionType)fn, pData, a, b, c, d, e, CREATE_SUSPENDED, NULL);
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

////static int stopThread(paTestData* pData)
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


/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	paTestData *data = (paTestData*)userData;
	ring_buffer_size_t elementsWriteable = PaUtil_GetRingBufferWriteAvailable(&data->ringBuffer);
	ring_buffer_size_t elementsToWrite = rbs_min(elementsWriteable, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;

	(void)outputBuffer; /* Prevent unused variable warnings. */
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	data->frameIndex += PaUtil_WriteRingBuffer(&data->ringBuffer, rptr, elementsToWrite);

	return paContinue;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	paTestData *data = (paTestData*)userData;
	ring_buffer_size_t elementsToPlay = PaUtil_GetRingBufferReadAvailable(&data->ringBuffer);
	ring_buffer_size_t elementsToRead = rbs_min(elementsToPlay, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
	SAMPLE* wptr = (SAMPLE*)outputBuffer;

	(void)inputBuffer; /* Prevent unused variable warnings. */
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	data->frameIndex += PaUtil_ReadRingBuffer(&data->ringBuffer, wptr, elementsToRead);

	return data->threadSyncFlag ? paComplete : paContinue;
}

static unsigned NextPowerOf2(unsigned val)
{
	val--;
	val = (val >> 1) | val;
	val = (val >> 2) | val;
	val = (val >> 4) | val;
	val = (val >> 8) | val;
	val = (val >> 16) | val;
	return ++val;
}
static int play_sound(char* path)
{
#ifdef WIND
	PlaySound(path, NULL, SND_SYNC);

	stopThread2(threadHandle);
#elif DUNIX
	system(path);
#endif

}

typedef void(*ThreadFunctionType2)(char*);

void *print_message_function(void *ptr);

/* Start up a new thread in the given function, at the moment only Windows, but should be very easy to extend
to posix type OSs (Linux/Mac) */
static int startThread2(char* path, ThreadFunctionType2 fn)
{
#ifdef _WIN32
	typedef unsigned(__stdcall* WinThreadFunctionType2)(void*);
	threadHandle = (void*)_beginthreadex(NULL, 0, (WinThreadFunctionType2)fn, path, CREATE_SUSPENDED, NULL);
	if (threadHandle == NULL) return 1;

	/* Set file thread to a little higher prio than normal */
	SetThreadPriority(threadHandle, THREAD_PRIORITY_ABOVE_NORMAL);

	/* Start it up */
	ResumeThread(threadHandle);

#endif
}

static void stopThread2(void* threadHandle)
{
#ifdef _WIN32
	CloseHandle(threadHandle);
#endif
}

SIGNAL real_time_record() {
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

	float *queue = (float *)malloc(sizeof(float) * QUEUE_SIZE);
	float *word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER_RECORD);
	int trim_ms = 0;
	int offset = FRAMES_PER_BUFFER;
	int flag = 1;
	int dem = 0;
	int succeed = 0;
	int time = 1;
	int cond_flag = 0;
	float peak;
	float syll[2];
	int dist = 0;
	float lowPeak1;
	float lowPeak2;
	int d_word = 0;
	char def_name[3];
	SIGNAL result;
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

	for (int i = 0;;) {
		err = Pa_ReadStream(stream, sampleBlock.snippet, FRAMES_PER_BUFFER);
		if (err) goto done;
		else {
			/*for (int u = 0; u < FRAMES_PER_BUFFER; ++u) {
				printf("%d : %f\n", u, sampleBlock.snippet[u]);
			}
			getchar();*/
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
					silence_detect_record(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, A, d1, d2, d3, d4,
						w0, w1, w2, w3, w4, x);
				}
			}
			else
			{
				succeed = silence_detect_record(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, A, d1, d2,
					d3, d4, w0, w1, w2, w3, w4, x);
				
				if (succeed == 1) {
					result = setSignal2(word, dist * FRAMES_PER_BUFFER);
					goto done;
				}
				/*for (int j = FRAMES_PER_BUFFER; j < QUEUE_SIZE; ++j) {
				queue[j - FRAMES_PER_BUFFER] = queue[j];
				}*/
			}
		}
	}

done:
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
	return result;
}

int silence_detect_record(float *data, size_t length, int *time, int *cond_flag, int *dist, float *word, float *peak, float *syll, float *lowPeak1, float *lowPeak2,
	int *d_word, float *A, float *d1, float *d2, float *d3, float *d4, float *w0, float *w1, float *w2, float *w3, float *w4, float *x) {
	//float *temp_data = butterworth_bandpass(8, data, length, 16000, 4000, 500);
	x = butterworth_bandpass_v2(2, data, length, 16000, 4000, 500, A, d1, d2, d3, d4, w0, w1, w2, w3, w4, x);
	int chunk_size = 160;
	float sum = 0;
	int trim_ms = 0;
	int dem = 0;
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
		}
		break;
	case 1:
		if (*peak < syll[1]) {
			//printf("case1 cond 0.0\n");
			*peak = syll[1];
			*dist += 1;
			Push(x, *dist - 1, word);
		}
		else
		{
			if (fabs(*peak - *lowPeak1) > 15) {
				//printf("case 1 cond 0\n");
				*lowPeak2 = syll[1];
				*cond_flag = 2;
				*dist += 1;
				Push(x, *dist - 1, word);
			}
			else
			{
				//printf("case 1 cond 1\n");
				*cond_flag = 0;
				*dist = 0;
				*lowPeak1 = syll[1];
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
			if (fabs(*peak - *lowPeak2) > 15 && *dist >= 14) {
				//printf("case 2 con 1\n");
				*lowPeak1 = syll[1];
				*dist += 1;
				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
				Push(x, *dist - 1, word);

				free(db);
				return 1;
				/*free(word);
				word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);*/
				//word = (float *)realloc(word, sizeof(float) * FRAMES_PER_BUFFER);
				/**dist = 0;
				*cond_flag = 0;*/
			}
			else if (fabs(*peak - *lowPeak2) > 10 && *dist <= 18 || *dist > 150) {
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
	return 0;
}


//void push_to_record(float *data, int index, float *word, SAMPLE *final_feats, hyper_vector fbank, hyper_vector temp_feats) {
//	int dem = 480;
//	if (index == 0) {
//		for (int i = 0; i < 160; ++i) {
//			word[i] = data[dem];
//			dem++;
//		}
//	}
//	else if (index < 2)
//	{
//		for (int i = index * 160; i < (index + 1) * 160; i++) {
//			if (index * 160 < (MAX_WORD_BUFFER * FRAMES_PER_BUFFER)) {
//				word[i] = data[dem];
//				dem++;
//			}
//			else
//			{
//				break;
//			}
//		}
//	}
//	else if (index == 2) {
//		for (int i = index * 160; i < (index + 1) * 160; i++) {
//			word[i] = data[dem];
//			dem++;
//		}
//		for (int i = 0; i < 400; ++i) {
//			temp_feats.data[i] = word[i];
//		}
//		add_to_final(final_feats, get_feature_vector_from_signal2(temp_feats, fbank), index - 3);
//	}
//	else if (index > 3) {
//		for (int i = 160; i < 480; ++i) {
//			word[i - 160] = word[i];
//			temp_feats.data[i - 160] = word[i - 160];
//		}
//		for (int i = 320; i < 480; i++) {
//			word[i] = data[dem];
//			if (i < 400) {
//				temp_feats.data[i] = word[i];
//			}
//			dem++;
//		}
//		add_to_final(final_feats, get_feature_vector_from_signal2(temp_feats, fbank), index - 3);
//		//for (int i = index; i < index + 13; ++i) {
//		//	printf("%f ", final_feats[i]);
//		//}
//		//printf("\n");
//	}
//}

void record_audio_to_database2(char *path, int *current_index)
{
	int size;
	SIGNAL audio_singal = real_time_record();
	int number_of_sample = audio_singal.signal_length;
	char *keyword = (char *)malloc(sizeof(char) * 5);
	char *numerical_order = (char *)malloc(sizeof(char) * 5);
	char *y_n = (char *)malloc(sizeof(char) * 5);

	printf("choose index of keyword \n");
	scanf("%s", keyword);
	*current_index = atoi(keyword);
	printf("choose the number order of text file to save audio signal \n");
	scanf("%s", numerical_order);
	char *name = get_name_of_new_file(path, keyword, numerical_order);
	FILE *fp = fopen(name, "w");
	fprintf(fp, "%d\n", number_of_sample);

	for (int i = 0; i < number_of_sample; ++i) {
		fprintf(fp, "%f\n", audio_singal.raw_signal[i]);
	}
	fclose(fp);
	free(keyword);
	free(numerical_order);
	free(audio_singal.raw_signal);
	printf("Do you wanna continue to record? (y/n) \n");
	scanf("%s", y_n);
	check_continue(y_n, path, current_index);
}