#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "svm2.h"
#ifdef __cplusplus
}
#endif // __cplusplus


/*Prototypes
------------------------------------------
------------------------------------------*/


void normalize_test(char *filename, int row, int col);
void OPTION();
char **load_words_sent(char *sent_path, int *num_of_sents);

int main(int argc, char **argv)
{
	int current_max_index = 0;
	int is_record = find_args(argc, argv, "-record");
	int is_create_database = find_args(argc, argv, "-createdb");
	int is_normalize_db = find_args(argc, argv, "-normlizedb");
	int is_train = find_args(argc, argv, "-training");
	int is_predict = find_args(argc, argv, "-predicting");
	if (is_record) {
		int current_index;
		char *path = argv[2];
		size_t len_path_folder = strlen(path);
		char *path_config = (char *)malloc(sizeof(char) *(len_path_folder + 10));
		strcpy(path_config, path);
		strcat(path_config, "config.txt");
		record_audio_to_database2(path, &current_index);
		FILE *config;
		if (!cfileexists(path_config)) {
			FILE *config_first_write = fopen(path_config, "w");
			current_max_index = current_index;
			fprintf(config_first_write, "%d", current_index);
			fclose(config_first_write);
		}
		else {
			config = fopen(path_config, "r");
			fscanf(config, "%d", &current_max_index);
			if (current_index > current_max_index) {
				fclose(config);
				FILE *config_write = fopen(path_config, "w");
				current_max_index = current_index;
				fprintf(config_write, "%d", current_max_index);
				fclose(config_write);
			}
			else {
				fclose(config);
			}
		}
	}
	if (is_create_database) {
		char *path = argv[2];

		if (path == NULL) {
			fprintf(stderr, "Path no exist!");
			exit(1);
		}
		size_t len_path_folder = strlen(path);
		char *path_config = (char *)malloc(sizeof(char) *(len_path_folder + 10));
		strcpy(path_config, path);
		strcat(path_config, "config.txt");
		FILE *config = fopen(path_config, "r");
		if (config == NULL) {
			fprintf(stderr, "Config file no exist!");
			exit(1);
		}
		fscanf(config, "%d", &current_max_index);
		fclose(config);
		filter_bank fbank = filterbank(26, 512);
		hyper_vector transpose_param = setHVector2(fbank.data, fbank.filt_len, fbank.nfilt, 1);		//26x257
		hyper_vector tmp = transpose(transpose_param);
		create_database(path, current_max_index, tmp);
	}
	if (is_normalize_db) {
		char *path = argv[2];
		if (path == NULL) {
			fprintf(stderr, "Path no exist!");
			exit(1);
		}
		size_t len_path_folder = strlen(path);
		char *path_config = (char *)malloc(sizeof(char) *(len_path_folder + 10));
		char *path_db = (char *)malloc(sizeof(char) * (len_path_folder + 6));
		char *path_info = (char *)malloc(sizeof(char) * (len_path_folder + 8));
		char *path_meaning = (char *)malloc(sizeof(char) * (len_path_folder + 8));
		char *path_nor = (char *)malloc(sizeof(char) * (len_path_folder + 14));
		char *path_sum = (char *)malloc(sizeof(char) * (len_path_folder + 7));
		strcpy(path_config, path);
		strcat(path_config, "config.txt");
		strcpy(path_db, path);
		strcat(path_db, "db.txt");
		strcpy(path_info, path);
		strcat(path_info, "info.txt");
		strcpy(path_meaning, path);
		strcat(path_meaning, "mean.txt");
		strcpy(path_nor, path);
		strcat(path_nor, "normalized.txt");
		strcpy(path_sum, path);
		strcat(path_sum, "sum.txt");
		FILE *config = fopen(path_config, "r");
		if (config == NULL) {
			fprintf(stderr, "Config file no exist!");
			exit(1);
		}
		fscanf(config, "%d", &current_max_index);
		normalize_db(path_nor, path_meaning, path_db, path_info, path_sum, current_max_index);
	}
	if (is_train) {
		char *path = argv[argc - 1];
		training_normalize(path, argc, argv);
	}
	if (is_predict) {
		char *sent_path = argv[argc - 1];
		char *path = argv[argc - 2];
		const char *model_path_def = "normalized.model";
		const char *sum_path_def = "sum.txt";
		size_t len_path = strlen(path);
		char *model_path = (char *)malloc(sizeof(char) * (len_path + 16));
		char *sum_path = (char *)malloc(sizeof(char) * (len_path + 7));
		strcpy(model_path, path);
		strcat(model_path, model_path_def);
		strcpy(sum_path, path);
		strcat(sum_path, sum_path_def);
		struct svm_model *model;
		if ((model = svm_load_model(model_path)) == 0) {
			fprintf(stderr, "cant load model file \n");
			exit(1);
		}

		SAMPLE *sum_normal = (SAMPLE*)malloc(sizeof(SAMPLE) * 91);
		mfcc_load_normalized_sum(sum_normal, sum_path);

		int num_of_sents;
		char **words;
		words = load_words_sent(sent_path, &num_of_sents);

		real_time_predict(model, sum_normal, path, sent_path, num_of_sents, words);
	}
	OPTION();
	getch();
}

char **load_words_sent(char *sent_path, int *num_of_sents) {
	const char* num_sent_def = "num_sents.txt";
	size_t len_path_sent = strlen(sent_path);
	char *num_sent = (char *)malloc(sizeof(char) * (len_path_sent + 8));

	/////////////num_sent///////////
	strcpy(num_sent, sent_path);
	strcat(num_sent, num_sent_def);
	FILE* fnumt = fopen(num_sent, "r");
	fscanf(fnumt, "%d", num_of_sents);
	fclose(fnumt);
	////////////words list///////////

	const char* lwords = "words.txt";
	size_t len_path_word = strlen(lwords);
	char *word_path = (char *)malloc(sizeof(char) * (len_path_sent + 8));
	strcpy(word_path, sent_path);
	strcat(word_path, lwords);

	FILE* fword = fopen(word_path, "r");
	int num_words;
	fscanf(fword, "%d", &num_words);
	char **words = (int**)malloc(num_words * sizeof(char*));

	for (int i = 0; i < num_words; i++) {
		words[i] = (char*)malloc(5 * sizeof(char));
		fscanf(fword, "%s", words[i]);
		printf("%s\n", words[i]);
	}

	fclose(fword);
	return words;
}

//////////////////////////////////
//////////////////////////////////


void normalize_test(char *filename, int row, int col) {
	int dem = 0, i = 0, j = 0;
	float * raw_training = (float*)malloc(sizeof(float) * (row + 1) * col);
	int label;

	FILE *f = fopen(filename, "r");

	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	for (i = 0; i < row; i++) {
		dem = 0;
		fscanf(f, "%d", &label);
		for (j = 0; j < col; j++) {
			fscanf(f, "%f", &raw_training[i * col + j]);
		}
	}
	fclose(f);
	FILE *fdb = fopen("db2.txt", "r");
	if (fdb == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	for (i = 0; i < 10; i++) {
		fscanf(fdb, "%d", &label);
		for (j = 0; j < col; j++) {
			fscanf(fdb, "%f", &raw_training[(row)* col + j]);
			printf("%f ", raw_training[(row)* col + j]);
		}
		Get_normalize(label, raw_training, row, col);
	}
	fclose(fdb);
	free(raw_training);
}

void OPTION() {
	int current_max_index = 0;
	char *path = "D:\\Visual_Studio_Workspace\\data\\scaleData\\";
	size_t len_path = strlen(path);
	const char *model_path_def = "normalized.model";
	char *model_path = (char *)malloc(sizeof(char) * (len_path + 16));
	strcpy(model_path, path);
	strcat(model_path, model_path_def);
	struct svm_model *model;
	size_t len_path_folder = strlen(path);
	char *path_config = (char *)malloc(sizeof(char) *(len_path_folder + 10));
	char *path_db = (char *)malloc(sizeof(char) * (len_path_folder + 6));
	char *path_info = (char *)malloc(sizeof(char) * (len_path_folder + 8));
	char *path_meaning = (char *)malloc(sizeof(char) * (len_path_folder + 8));
	char *path_nor = (char *)malloc(sizeof(char) * (len_path_folder + 14));
	char *path_sum = (char *)malloc(sizeof(char) * (len_path_folder + 7));
	strcpy(path_config, path);
	strcat(path_config, "config.txt");
	strcpy(path_db, path);
	strcat(path_db, "db.txt");
	strcpy(path_info, path);
	strcat(path_info, "info.txt");
	strcpy(path_meaning, path);
	strcat(path_meaning, "mean.txt");
	strcpy(path_nor, path);
	strcat(path_nor, "normalized.txt");
	strcpy(path_sum, path);
	strcat(path_sum, "sum.txt");


	printf("ALTERNATIVE.\n");
	printf("1. Create DB.\n");
	printf("2. Predict.\n");
	printf("3. Normalize.\n");
	int option;
	scanf("%d", &option);
	switch (option) {
	case 1:
		FILE *config = fopen(path_config, "r");
		if (config == NULL) {
			fprintf(stderr, "Config file no exist!");
			exit(1);
		}
		fscanf(config, "%d", &current_max_index);
		fclose(config);
		filter_bank fbank = filterbank(26, 512);
		hyper_vector transpose_param = setHVector2(fbank.data, fbank.filt_len, fbank.nfilt, 1);		//26x257
		hyper_vector tmp = transpose(transpose_param);
		create_database(path, current_max_index, tmp);
		break;
	case 2:
	{
		char *sent_path = "D:\\Work_place_game\\sentences\\";
		const char *model_path_def = "normalized.model";
		const char *sum_path_def = "sum.txt";
		size_t len_path = strlen(path);

		char *model_path = (char *)malloc(sizeof(char) * (len_path + 16));
		char *sum_path = (char *)malloc(sizeof(char) * (len_path + 7));

		strcpy(model_path, path);
		strcat(model_path, model_path_def);

		strcpy(sum_path, path);
		strcat(sum_path, sum_path_def);



		struct svm_model *model;
		if ((model = svm_load_model(model_path)) == 0) {
			fprintf(stderr, "cant load model file \n");
			exit(1);
		}

		SAMPLE *sum_normal = (SAMPLE*)malloc(sizeof(SAMPLE) * 91);
		filter_bank fbanks = filterbank(26, 512);
		mfcc_load_normalized_sum(sum_normal, sum_path);

		int num_of_sents;
		char **words;
		words = load_words_sent(sent_path, &num_of_sents);
		real_time_predict(model, sum_normal, path, sent_path, num_of_sents, words);
		break;
	}
	case 3: {
		FILE *config = fopen(path_config, "r");
		if (config == NULL) {
			fprintf(stderr, "Config file no exist!");
			exit(1);
		}
		fscanf(config, "%d", &current_max_index);
		normalize_db(path_nor, path_meaning, path_db, path_info, path_sum, current_max_index);
		break;
	}
	case 4: {
		char *path = "D:\\Visual_Studio_Workspace\\sentences\\mo_cua_truoc_ra.mp3";
		system(path);
		break;
	}
	case 5: {
		int current_index;
		char *path = "D:\\Visual_Studio_Workspace\\data\\";
		size_t len_path_folder = strlen(path);
		char *path_config = (char *)malloc(sizeof(char) *(len_path_folder + 10));
		strcpy(path_config, path);
		strcat(path_config, "config.txt");
		record_audio_to_database2(path, &current_index);
	}
	default:

		break;
	}
}