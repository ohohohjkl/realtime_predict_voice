



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