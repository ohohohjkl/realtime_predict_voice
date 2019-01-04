//
//
//void Push(float *data, int index, float *word) {
//	int dem = 480;
//	if (index == 0) {
//		for (int i = 0; i < 160; ++i) {
//			word[i] = data[dem];
//			dem++;
//		}
//	}
//	else
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
//	dem++;
//}

//int silence_detect(float *data, size_t length, int *time, int *cond_flag, int *dist, float *word, float *peak, float *syll, float *lowPeak1, float *lowPeak2,
//	int *d_word, char *def_name, char *ext, char *path, float *A, float *d1, float *d2, float *d3, float *d4, float *w0, float *w1, float *w2, float *w3,
//	float *w4, float *x, struct svm_model *model, SAMPLE *sum_normal, hyper_vector fbank, PaStream *stream, char **words) {
//	x = butterworth_bandpass_v2(2, data, length, 16000, 4000, 500, A, d1, d2, d3, d4, w0, w1, w2, w3, w4, x);
//	int chunk_size = 160;
//	float sum = 0;
//	int trim_ms = 0;
//	int dem = 0;
//	int succeed = 1;
//	LARGE_INTEGER Frequency;
//	QueryPerformanceFrequency(&Frequency);
//
//	float *db = (float *)malloc(sizeof(float) * 7);
//	while (trim_ms < length)
//	{
//		sum = 0;
//		for (int i = trim_ms; i < trim_ms + chunk_size; i++) {
//			sum += x[i] * x[i];
//		}
//		sum = sqrt(sum / chunk_size);
//		sum = 20 * log10(sum);
//		db[dem] = sum;
//		dem++;
//		trim_ms += chunk_size;
//	}
//
//	if (*time) {
//		syll[0] = (db[0] + db[1] + db[2] + db[3] + db[4] + db[5]) / 6;
//		syll[1] = (db[1] + db[2] + db[3] + db[4] + db[5] + db[6]) / 6;
//		*lowPeak1 = syll[0];
//		*lowPeak2 = 0;
//	}
//	else
//	{
//		syll[1] = (db[1] + db[2] + db[3] + db[4] + db[5] + db[6]) / 6;
//	}
//	switch (*cond_flag)
//	{
//	case 0:
//		if (*lowPeak1 < syll[1]) {
//			//printf("case 0, cond 0\n");
//			*peak = syll[1];
//			*cond_flag = 1;
//			(*dist) += 1;
//			//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
//			if (!(*time)) {
//				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
//				Push(x, *dist - 1, word);
//				*time = 0;
//			}
//			else
//			{
//				Push(x, *dist - 1, word);
//				*time = 0;
//			}
//		}
//		else if (*lowPeak1 >= syll[1]) {
//			//printf("case 0, cond 1\n");
//			*lowPeak1 = syll[1];
//		}
//		break;
//	case 1:
//		if (*peak < syll[1]) {
//			//printf("case1 cond 0.0\n");
//			*peak = syll[1];
//			*dist += 1;
//			//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
//			//word = realloc_same_add(data, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
//			Push(x, *dist - 1, word);
//		}
//		else
//		{
//			if (fabs(*peak - *lowPeak1) > 15) {
//				//printf("case 1 cond 0\n");
//				*lowPeak2 = syll[1];
//				*cond_flag = 2;
//				*dist += 1;
//				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
//				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
//				Push(x, *dist - 1, word);
//			}
//			else
//			{
//				//printf("case 1 cond 1\n");
//				*cond_flag = 0;
//				*dist = 0;
//				*lowPeak1 = syll[1];
//				/*free(word);
//				word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);*/
//				//word = (float *)realloc(word, sizeof(float) * FRAMES_PER_BUFFER);
//			}
//		}
//		break;
//	case 2:
//		if (*lowPeak2 > syll[1]) {
//			//printf("case 2 cond 0\n");
//			*lowPeak2 = syll[1];
//			*dist += 1;
//			//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
//			//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
//			Push(x, *dist - 1, word);
//		}
//		else
//		{
//			if (fabs(*peak - *lowPeak2) > 15 && *dist >= 13) {
//				//printf("case 2 con 1\n");
//
//				*lowPeak1 = syll[1];
//				*dist += 1;
//				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
//				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
//				Push(x, *dist - 1, word);
//				//start = PerformanceCounter();
//				//Pa_StopStream(stream);
//				write_to_syll(d_word, def_name, ext, path, dist, word, model, sum_normal, fbank, words);
//				//Pa_StartStream(stream);
//				//double dftDuration3 = (double)(PerformanceCounter() - start) * 1000.0 / (double)Frequency.QuadPart;
//				//if (dftDuration3 > 0.1)
//				//	printf("WRITE_TO" ": %f\n", dftDuration3);
//				///*free(word);
//				//word = (float *)malloc(sizeof(float) * FRAMES_PER_BUFFER * MAX_WORD_BUFFER);
//				//word = (float *)realloc(word, sizeof(float) * FRAMES_PER_BUFFER);
//				*dist = 0;
//				*cond_flag = 0;
//				succeed = 0;
//			}
//			else if (fabs(*peak - *lowPeak2) > 12 && *dist <= 18 || *dist > 150) {
//				printf("EXCEPTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");
//				*dist = 0;
//				*cond_flag = 0;
//			}
//			else
//			{
//				//printf("case 2 con 2\n");
//				*peak = syll[1];
//				*cond_flag = 3;
//				*dist += 1;
//				//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
//				//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
//				Push(x, *dist - 1, word);
//			}
//		}
//		break;
//	case 3:
//		if (*peak < syll[1]) {
//			//printf("case 3 cond 1\n");
//			*peak = syll[1];
//		}
//		else
//		{
//			//printf("case 3 cond 2\n");
//			*lowPeak2 = syll[1];
//			*cond_flag = 2;
//		}
//		*dist += 1;
//		//word = (float *)realloc(word, sizeof(float) * (*dist) * FRAMES_PER_BUFFER);
//		//word = realloc_same_add(word, (*dist - 1) * FRAMES_PER_BUFFER, (*dist) * FRAMES_PER_BUFFER);
//		Push(x, *dist - 1, word);
//		break;
//	default:
//		break;
//	}
//	//free(temp_data);
//	free(db);
//	return succeed;
//}

//hyper_vector get_feature_vector_from_signal(hyper_vector a, hyper_vector fbank)
//{
//	/*______________________get_pre_emphasized_signal_________________________________________________*/
//	/*______________________get_silence_free_signal___________________________________________________*/
//	/*______________________get_Frames________________________________________________________________*/
//	LARGE_INTEGER Frequency;
//	QueryPerformanceFrequency(&Frequency);
//
//	//hyper_vector frames = getFrames(a);			
//	//free(a.raw_signal);
//	/*______________________compute_DFT_and_Power_spectrum____________________________________________*/
//
//	hyper_vector power_spec = fft(a, 512);
//	/*______________________get_filterbanks___________________________________________________________*/
//	/*______________________apply_filterBanks_________________________________________________________*/
//	//hyper_vector transpose_param = setHVector2(fbank.data, fbank.filt_len, fbank.nfilt, 1);		//26x257
//
//	//hyper_vector tmp = transpose(transpose_param);
//	//free(transpose_param.data);
//	//long long start3 = PerformanceCounter();
//	//#ifdef USE_MULTI_THREAD
//	//	hyper_vector apply = multiply_multithread(power_spec, tmp);
//	//#else
//	hyper_vector apply = multiply(power_spec, fbank);
//	//#endif // USE_MULTI_THREAD
//
//	//NUMFRAMESx257 * 257x26
//	/*double dftDuration3 = (double)(PerformanceCounter() - start3) * 1000.0 / (double)Frequency.QuadPart;
//
//	if (dftDuration3 > 0.1)
//	printf("Matrix processing" ": %f\n", dftDuration3);
//	*/
//	//free(tmp.data);
//
//	/*______________________get_more_compact_output_by_performing_DCT_conversion_______________________*/
//	//long long start3 = PerformanceCounter();
//
//	hyper_vector test = DCT2(apply, 13);
//	/*double dftDuration3 = (double)(PerformanceCounter() - start3) * 1000.0 / (double)Frequency.QuadPart;
//	if (dftDuration3 > 0.1)
//	printf("Matrix processing" ": %f\n", dftDuration3);
//	*/
//	free(apply.data);
//	/*______________________append_frame_energy_into_mfcc_vectors______________________________________*/
//	append_energy(test, power_spec);
//	free(power_spec.data);
//	/*______________________final_feature_vector_size_1x91_____________________________________________*/
//	//long long start3 = PerformanceCounter();
//
//	hyper_vector final_feats = cov(test);
//	//double dftDuration3 = (double)(PerformanceCounter() - start3) * 1000.0 / (double)Frequency.QuadPart;
//
//	/*if (dftDuration3 > 0.1)
//	printf("Matrix processing" ": %f\n", dftDuration3);
//	*/
//	free(test.data);
//	return final_feats;
//}


//SAMPLE * get_audio_signal_from_source(int *size)
//{
//	PaStreamParameters inputParameters, outputParameters;
//	PaStream *stream;
//	PaError err;
//	SAMPLE *recordedSamples;
//	int i;
//	int totalFrames;
//	int numSamples;
//	size_t numBytes;
//	SAMPLE max, average, val;
//
//	printf("patest_read_record.c\n"); fflush(stdout);
//
//	totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
//	numSamples = totalFrames * NUM_CHANNELS;
//
//	*size = numSamples;
//	numBytes = numSamples * sizeof(SAMPLE);
//	recordedSamples = (SAMPLE *)malloc(numBytes);
//	if (!recordedSamples)
//	{
//		printf("Could not allocate record array.\n");
//		exit(1);
//	}
//	for (i = 0; i < numSamples; i++) *(recordedSamples + i) = 0;
//
//	err = Pa_Initialize();
//	if (err != paNoError) goto error;
//
//	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
//	if (inputParameters.device == paNoDevice) {
//		fprintf(stderr, "Error: No default input device.\n");
//		goto error;
//	}
//	inputParameters.channelCount = NUM_CHANNELS;
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
//		NULL, /* no callback, use blocking API */
//		NULL); /* no callback, so no callback userData */
//	if (err != paNoError) goto error;
//
//	err = Pa_StartStream(stream);
//	if (err != paNoError) goto error;
//	printf("Now recording!!\n"); fflush(stdout);
//	err = Pa_ReadStream(stream, recordedSamples, totalFrames);
//	if (err != paNoError) goto error;
//
//	err = Pa_CloseStream(stream);
//	if (err != paNoError) goto error;
//	return recordedSamples;
//error:
//	Pa_Terminate();
//	fprintf(stderr, "An error occured while using the portaudio stream\n");
//	fprintf(stderr, "Error number: %d\n", err);
//	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
//	return -1;
//	return recordedSamples;
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
//	QueryPerformanceFrequency(&Frequency);
//	int demtemp = 0;
//	int timer = 0;
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
//	err = startThread(&data, queue, sent_path,wtemp, demtemp, trim_ms, offset, p_word,timer,time,cond_flag,dist, word, peak, syll,lowPeak1, lowPeak2,
//		d_word, def_name,ext, def_path,A, d1, d2, d3, d4, w0, w1, w2,w3, w4, x, model, sum_normal, fbank, Frequency, stream,threadFunctionWriteToRawFile);
//	if (err != paNoError) goto done;
//
//	err = Pa_StartStream(stream);
//	if (err != paNoError) goto done;
//	printf("\n=== Now recording to '" FILE_NAME "' for %d seconds!! Please speak into the microphone. ===\n", NUM_SECONDS); fflush(stdout);
//
//	/* Note that the RECORDING part is limited with TIME, not size of the file and/or buffer, so you can
//	increase NUM_SECONDS until you run out of disk */
//	
//	while (1)
//	{
//		
//	}
//	if (err < 0) goto done;
//	err = Pa_CloseStream(stream);
//	if (err != paNoError) goto done;
//
//	/* Stop the thread */
//	err = stopThread(&data);
//	if (err != paNoError) goto done;
//	/* Close file */
//	fclose(data.file);
//	data.file = 0;
//
//done:
//	
//	
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


///* This routine is run in a separate thread to write data from the ring buffer into a file (during Recording) */
//static int threadFunctionWriteToRawFile(void* ptr, float *queue, char *sent_path, char *wtemp,int *demtemp,int *trim_ms, int offset, int *p_word,int *timer, int *time, int *cond_flag, int *dist, float *word, float *peak, float *syll, float *lowPeak1, float *lowPeak2,
//	int *d_word, char *def_name, char *ext, char *def_path, float *A, float *d1, float *d2, float *d3, float *d4, float *w0, float *w1, float *w2, float *w3, float *w4, float *x, struct svm_model *model, SAMPLE *sum_normal, filter_bank fbank, LARGE_INTEGER Frequency, PaStream* stream)
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
//				if (trim_ms < QUEUE_SIZE) {
//					for (int j = trim_ms, k = 0; j < trim_ms + offset; ++j) {
//						queue[j] = pData->ringBufferData[k];
//						printf("%f \n", queue[j]);
//						k++;
//					}
//				}
//				else {
//					for (int j = FRAMES_PER_BUFFER; j < QUEUE_SIZE; ++j) {
//						queue[j - FRAMES_PER_BUFFER] = queue[j];
//					}
//					for (int j = 0; j < FRAMES_PER_BUFFER; ++j) {
//						queue[QUEUE_SIZE - FRAMES_PER_BUFFER + j] = pData->ringBufferData[j];
//					}
//				}
//
//				if (trim_ms < QUEUE_SIZE) {
//					trim_ms += offset;
//					if (trim_ms < QUEUE_SIZE) {
//						continue;
//					}
//					else {
//						silence_detect(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, def_name, ext, def_path, A, d1, d2, d3, d4,
//							w0, w1, w2, w3, w4, x, model, sum_normal, fbank, stream);
//						if (d_word == 1) {
//							p_word = d_word;
//							timer = 1;
//						}
//					}
//				}
//				else
//				{
//					PRINT_TIME_PROCESS_START(start);
//					silence_detect(queue, QUEUE_SIZE, &time, &cond_flag, &dist, word, &peak, syll, &lowPeak1, &lowPeak2, &d_word, def_name, ext, def_path, A, d1, d2,
//						d3, d4, w0, w1, w2, w3, w4, x, model, sum_normal, fbank, stream);
//					PRINT_TIME_PROCESS_STOP(start, "Total2", 1);
//					if (d_word == 1) {
//						p_word = d_word;
//						timer = 1;
//					}
//					if (check_word(d_word, p_word) && tdem < 100) {
//						p_word = d_word;
//						timer = 1;
//						tdem = 0;
//					}
//					if (timer) {
//						tdem++;
//						if (tdem > 100) {
//							check_sentence_formation(sent_path, ext, d_word, wtemp);
//							d_word = p_word = 0;
//							tdem = 0;
//							timer = 0;
//							demtemp = 0;
//						}
//					}
//					fflush(stdout);
//					Pa_Sleep(10);
//				}
//				
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