#include "lib_io.h"
#include "lib_time.h"
#include "stdio.h"
#include "predict.h"

extern int read_file(char ** const buff, const unsigned int spec, const char * const filename);
void print_time(const char * const head);
extern void release_buff(char ** const buff, const int valid_item_num);
extern void write_result(const char * const buff, const char * const filename);

int main(int argc, char *argv[])
{
	//print_time("Begin");
	char *data[MAX_DATA_NUM];   //50000
	char *info[MAX_INFO_NUM];   //50
	int data_line_num;   //数据行数
	int info_line_num;   //输入行数

	char *data_file = argv[1];

	data_line_num = read_file(data, MAX_DATA_NUM, data_file);

	printf("data file line num is :%d \n", data_line_num);
	if (data_line_num == 0)
	{
		printf("Please input valid data file.\n");
		return -1;
	}

	char *input_file = argv[2];

	info_line_num = read_file(info, MAX_INFO_NUM, input_file);

	printf("input file line num is :%d \n", info_line_num);
	if (info_line_num == 0)
	{
		printf("Please input valid info file.\n");
		return -1;
	}

	char *output_file = argv[3];
	predict_server(info, data, data_line_num, output_file);

	release_buff(info, info_line_num);
	release_buff(data, data_line_num);

	//print_time("End");

	return 0;
}

