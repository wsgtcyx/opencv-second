
#include "second.h"
int No_Algorithm = 4; //there are 1,2,3,4,5 optionally.
#define DEBUG  

// Global variables  不可缺少
const int g_thresh_for_sobel_detect_contour = 10;
const int g_length =3; //the g_length of square for convolution
const int g_thresh_Di = 5 * g_length*g_length;
RECT g_stored_Rect[500000];
int g_count_Rect = 0;

#if run_switch
double stored_scores[25][5];
int g_scores_number = 0;
double g_result[25];

void calculate_and_store(uFORM_ImgParam* src_Form)
{
	double result;
	for (int i = 1; i <= 5; i++)
	{
		switch (i)
		{
			case 1:result = calculate_scores1_Cversion(src_Form); break;
			case 2:result = calculate_scores2_Cversion(src_Form); break;
			case 3:result = calculate_scores3_Cversion(src_Form); break;
			case 4:result = calculate_scores4_Cversion(src_Form); break;
			case 5:result = calculate_scores5_Cversion(src_Form); break;
		}
		stored_scores[g_scores_number][i - 1] = result;
	}
	g_scores_number++;
}
void process_data(void)
{
	//max-min scaler
	for (int j = 0; j < 5; j++)
	{
		double max_value = -50000000;
		double min_value = 100000000;
		for (int i = 0; i < g_scores_number; i++)
		{
			if (stored_scores[i][j]>max_value)
				max_value = stored_scores[i][j];
			if (stored_scores[i][j] < min_value)
				min_value = stored_scores[i][j];
		}
		for (int i = 0; i < g_scores_number; i++)
		{
			stored_scores[i][j] = stored_scores[i][j] - min_value;
			stored_scores[i][j] = stored_scores[i][j] / (max_value - min_value);
		}
		printf("algorithm No: %d max_value:%lf, min_value:%lf\n\n", j+1, max_value, min_value);
	}
	for (int i = 0; i < g_scores_number; i++)
	{
		double temp_result = 0;
		for (int j = 0; j < 5; j++)
			temp_result += stored_scores[i][j];
		g_result[i] = temp_result / 5;	
	}
}

void transform_Form(Mat* Image, uFORM_ImgParam* Form_Image)
{
	Form_Image->pSrc = (BYTE *)malloc(Image->rows *Image->cols* sizeof(BYTE));
	Form_Image->pSrc = Image->data;
	Form_Image->nHeight = Image->rows;
	Form_Image->nWidth = Image->cols;
	return;
}
void release_mode(void)
{
	time_t start, end_time;
	char imageName[100];
	double result;
	gets(imageName);
	Mat src = imread(imageName, 0);
	if (!src.data)
	{
		printf("picture data error");
		return;
	}
	global_initialize();
	uFORM_ImgParam src_Form;
	transform_Form(&src, &src_Form);

	start = clock();

	switch (No_Algorithm)
	{
	case 1:result = calculate_scores1_Cversion(&src_Form); break;
	case 2:result = calculate_scores2_Cversion(&src_Form); break;
	case 3:result = calculate_scores3_Cversion(&src_Form); break;
	case 4:result = calculate_scores4_Cversion(&src_Form); break;
	case 5:result = calculate_scores5_Cversion(&src_Form); break;
	}

	end_time = clock();
	printf("the running time is : %f ms\n", double(end_time - start) / CLOCKS_PER_SEC * 1000);

	printf("Score is:%lf\n\n", result);
}
void debug_mode(void)
{
	time_t start, end_time;
	char imageName[100];
	char readfile[] = "file_list.txt";
	char writefile[] = "D:\\internship\\test2\\test.txt";
	Mat src;

	FILE* fp_Write;
	FILE* fp_Read;
	remove(readfile);
	system("DIR /b/o >> file_list.txt");

	fp_Write = fopen(writefile, "w+");
	if ((fp_Read = fopen(readfile, "r")) == NULL)
	{
		printf("error because of readfile\n");
		getchar();
		return;
	}

	while (!feof(fp_Read))
	{
		fgets(imageName, 100, fp_Read);
		imageName[strlen(imageName) - 1] = NULL;


		global_initialize();
		src = imread(imageName, 0);
		if (!src.data)
			continue;
		uFORM_ImgParam src_Form;
		transform_Form(&src, &src_Form);

		//fprintf(fp_Write, "%s\n", imageName);
		printf("%s\n", imageName);

		start = clock();
		calculate_and_store(&src_Form);
		end_time = clock();
		printf("the running time is : %f ms\n", double(end_time - start) / CLOCKS_PER_SEC * 1000);
		

	}
	process_data();
	
	for (int i = 0; i < g_scores_number; i++)
	{
		fprintf(fp_Write, "%d\n", i+5);
		fprintf(fp_Write, "%lf\n\n", g_result[i]);
		printf("sample No:%d scores:%lf\n\n",i+5, g_result[i]);
	}


	fclose(fp_Read);
	fclose(fp_Write);

	system("python Demo.py");
}
int main(int argc, char** argv)
{
#ifdef DEBUG
	//DEBUG接口说明
	//程序的工作目录需设置为待处理图片所在文件夹，将会自动生成一个包含所有图片文件名的txt文件file_list.txt 
	//writefile 为输出文件txt， 输出每张图片的文件名，图片清晰度评价函数值。
	debug_mode();
#else//release version
	//release接口说明
	//程序的工作目录需设置为待处理图片所在文件夹,需要用户手动输入图片文件名
	//输出图片清晰度评价函数值
	release_mode();
#endif//DEBUG
	getchar();
	return 0;

}
#endif //run_swich
void copy_Form(uFORM_ImgParam* src, uFORM_ImgParam* dst)
{
	dst->pSrc = (BYTE *)malloc(src->nHeight *src->nWidth* sizeof(BYTE));
	dst->nHeight = src->nHeight;
	dst->nWidth = src->nWidth;
}

void global_initialize(void)
{
	g_count_Rect = 0;
	return;
}
double calculate_scores1_Cversion(uFORM_ImgParam* Form_Image)
{
	//original sobel derivatives

	int nl = Form_Image->nHeight, nc = Form_Image->nWidth;
	//uFORM_ImgParam abs_grad_x, abs_grad_y;
	uFORM_ImgParam imageX, imageY;
	BYTE *P = Form_Image->pSrc;
	copy_Form(Form_Image, &imageX);
	copy_Form(Form_Image, &imageY);
	BYTE *PX = imageX.pSrc;
	BYTE *PY = imageY.pSrc;

	int i, j;
	double count = 0;
	double gradx_value, grady_value;
	double grad_value = 0;
	double result = 0;

	for (i = 1; i<nl - 1; i++)
	{
		for (j = 1; j<nc - 1; j++)
		{
			//通过指针遍历图像上每一个像素  
			PX[i*nc + j] = abs(P[(i - 1)*nc + j + 1] + P[i*nc + j + 1] * 2 + P[(i + 1)*nc + j + 1] - P[(i - 1)*nc + j - 1] - P[i*nc + j - 1] * 2 - P[(i + 1)*nc + j - 1]);
			PY[i*nc + j] = abs(P[(i + 1)*nc + j - 1] + P[(i + 1)*nc + j] * 2 + P[(i + 1)*nc + j + 1] - P[(i - 1)*nc + j - 1] - P[(i - 1)*nc + j] * 2 - P[(i - 1)*nc + j + 1]);
		}
	}


	for (j = 1; j < nl - 1; j++)//y
	{
		for (i = 1; i < nc - 1; i++)//x
		{
			gradx_value = PX[j*nc + i];
			grady_value = PY[j*nc + i];
			grad_value = (gradx_value + grady_value)*1.0 / 2;
			if (grad_value>g_thresh_for_sobel_detect_contour)
				count += pow(grad_value, 2);
		}
	}
	free(imageX.pSrc);
	free(imageY.pSrc);
	result = count*1.0 / (nl*nc);
	return result;
}
double calculate_scores2_Cversion(uFORM_ImgParam* Form_Image)
{
	//tenengrad function version
	int nl = Form_Image->nHeight, nc = Form_Image->nWidth;
	int i, j;
	/// Gradient X
	//uFORM_ImgParam abs_grad_x, abs_grad_y;
	uFORM_ImgParam imageX, imageY;
	BYTE *P = Form_Image->pSrc;
	copy_Form(Form_Image, &imageX);
	copy_Form(Form_Image, &imageY);
	BYTE *PX = imageX.pSrc;
	BYTE *PY = imageY.pSrc;
	double count = 0;
	double gradx_value, grady_value;
	double grad_value = 0;
	double result;

	for (i = 1; i<nl - 1; i++)
	{
		for (j = 1; j<nc - 1; j++)
		{
			//通过指针遍历图像上每一个像素  
			PX[i*nc + j] = abs(P[(i - 1)*nc + j + 1] + P[i*nc + j + 1] * 2 + P[(i + 1)*nc + j + 1] - P[(i - 1)*nc + j - 1] - P[i*nc + j - 1] * 2 - P[(i + 1)*nc + j - 1]);
			PY[i*nc + j] = abs(P[(i + 1)*nc + j - 1] + P[(i + 1)*nc + j] * 2 + P[(i + 1)*nc + j + 1] - P[(i - 1)*nc + j - 1] - P[(i - 1)*nc + j] * 2 - P[(i - 1)*nc + j + 1]);
		}
	}

	for (j = 1; j < nl - 1; j++)//y
	{
		for (i = 1; i < nc - 1; i++)//x
		{
			gradx_value = PX[j*nc + i] * 1.0;
			grady_value = PY[j*nc + i] * 1.0;
			grad_value = sqrt(pow(gradx_value, 2) + pow(grady_value, 2));
			if (grad_value>g_thresh_for_sobel_detect_contour)
				count += pow(grad_value, 2);
		}
	}
	free(imageX.pSrc);
	free(imageY.pSrc);
	result = count*1.0 / (nl*nc);
	return result;
}
double calculate_scores3_Cversion(uFORM_ImgParam* Form_Image)
{
	//SMD2
	int nl = Form_Image->nHeight, nc = Form_Image->nWidth;
	int i, j;
	BYTE *P = Form_Image->pSrc;
	double count = 0;
	double result;
	int gradx_value, grady_value;
	for (j = 1; j < (nl - 1); j++)//y
	{
		for (i = 1; i < (nc - 1); i++)//x
		{
			gradx_value = P[j*nc + i] - P[j*nc + i + 1];
			grady_value = P[j*nc + i] - P[(j + 1)*nc + i];
			count += abs(gradx_value)*abs(grady_value);
		}
	}
	result = count*1.0 / ((nl - 2)*(nc - 2));
	return result;

}
double calculate_scores4_Cversion(uFORM_ImgParam* Form_Image)
{
	//利用方差评价模糊程度
	//返回值为模糊程度值的倒数

	int nl = Form_Image->nHeight, nc = Form_Image->nWidth;
	int i, j;
	//GaussianBlur(Image, Image2, Size(3, 3), 0, 0, BORDER_DEFAULT);
	BYTE *P = Form_Image->pSrc;
	double sum_D = 0;
	double Di = 0;
	double Max_Di = 0;
	double count = 0;
	double average_value = 0;
	double B;
	for (j = 1; j < (nl - g_length - 1); j = j + g_length)//y
	{
		for (i = 1; i < (nc - 1 - g_length); i = i + g_length)//x
		{
			Di = 0;
			average_value = 0;
			for (int m = 0; m < g_length; m++)//y in g_length
			{
				for (int n = 0; n < g_length; n++) //x in g_length
				{
					average_value += P[(j + m)*nc + i + n];
				}
			}
			average_value = average_value / (g_length*g_length);
			for (int m = 0; m < g_length; m++)
			{
				for (int n = 0; n < g_length; n++)
				{
					Di += pow((P[(j + m)*nc + i + n] - average_value), 2);
				}
			}
			if (Di>Max_Di)
				Max_Di = Di;
			sum_D += Di;
			count++;
		}
	}
	B = exp(-(sum_D) / count / (Max_Di + 1));
	return 1 / (B + 0.000001);
}
double cal_Diff(uFORM_ImgParam* Form_Image, uFORM_ImgParam* Blur_Image)
{
	double bfV[50000];
	double bfH[50000];
	int nl = Form_Image->nHeight, nc = Form_Image->nWidth;
	int DfV, DfH, DbV, DbH, sVV, sVH, sfV, sfH;
	int Rect_number;
	int i, j;
	for (Rect_number = 0; Rect_number < g_count_Rect; Rect_number++)
	{
		RECT Rect_temp = g_stored_Rect[Rect_number];
		sVV = 0;
		sVH = 0;
		sfV = 0;
		sfH = 0;
		BYTE *P = Form_Image->pSrc;
		BYTE *BP = Blur_Image->pSrc;
		for (j = Rect_temp.top; j < Rect_temp.bottom; j++)
		{
			for (i = Rect_temp.left; i < Rect_temp.right; i++)
			{
				DfV = abs(P[j*nc + i] - P[j*nc + i - 1]);
				DfH = abs(P[j*nc + i] - P[(j - 1)*nc + i]);
				DbV = abs(BP[j*nc + i] - BP[j*nc + i - 1]);
				DbH = abs(BP[j*nc + i] - BP[(j - 1)*nc + i]);

				sfV += DfV;
				sfH += DfH;
				sVV += DbV;
				sVH += DbH;
			}
		}
		double bfV_temp = (sfV - sVV)*1.0 / sfV;
		double bfH_temp = (sfH - sVH)*1.0 / sfH;
		bfV[Rect_number] = bfV_temp;
		bfH[Rect_number] = bfH_temp;
	}
	double average_bfV = 0, average_bfH = 0;
	for (int i = 0; i < g_count_Rect; i++)
	{
		average_bfV += bfV[i];
		average_bfH += bfH[i];
	}
	average_bfV = average_bfV*1.0 / g_count_Rect;
	average_bfH = average_bfH*1.0 / g_count_Rect;

	double result = average_bfV*average_bfH;
	return result;
}
void Filter(uFORM_ImgParam* Form_Image)
{
	//均值滤波有最好的效果
	BYTE* data = Form_Image->pSrc;
	int height = Form_Image->nHeight, width = Form_Image->nWidth;

	int i, j, index, sum;
	int templates[9] = { 1, 1, 1,
		1, 5, 1,
		1, 1, 1 };//模板的值   
	sum = height * width * sizeof(BYTE);//图像所占内存的大小

	BYTE *tmpdata = (BYTE*)malloc(sum);
	memcpy((BYTE*)tmpdata, (BYTE*)data, sum);
	for (i = 1; i < height - 1; i++)
	{
		for (j = 1; j < width - 1; j++)
		{
			index = sum = 0;
			for (int m = i - 1; m < i + 2; m++)
			{
				for (int n = j - 1; n < j + 2; n++)
				{
					sum += tmpdata[m * width + n] * templates[index++];  //处理   
				}
			}
			data[i * width + j] = sum /9;

		}
	}
	free(tmpdata);
}
double calculate_scores5_Cversion(uFORM_ImgParam* Form_Image)
{
	//Reblur really

	int nl = Form_Image->nHeight, nc = Form_Image->nWidth;

	uFORM_ImgParam Blur_Image;
	copy_Form(Form_Image, &Blur_Image);
	memcpy((BYTE*)Blur_Image.pSrc, (BYTE*)Form_Image->pSrc, nl*nc);
	Filter(&Blur_Image);

	BYTE *P = Form_Image->pSrc;
	double Di = 0;
	for (int j = 1; j < (nl - g_length - 1); j = j + g_length)//y
	{
		for (int i = 1; i < (nc - 1 - g_length); i = i + g_length)//x
		{
			Di = 0;
			//calculate the average
			double average_value = 0;
			for (int m = 0; m < g_length; m++)//y in g_length
			{
				for (int n = 0; n < g_length; n++) //x in g_length
				{
					average_value += P[(j + m)*nc + i + n];
				}
			}
			average_value = average_value / (g_length*g_length);
			//calculate Di
			for (int m = 0; m < g_length; m++)
			{
				for (int n = 0; n < g_length; n++)
				{
					Di += pow((P[(j + m)*nc + i + n] - average_value), 2);
				}
			}
			//judge the ROI by Di comparison
			if (Di>g_thresh_Di)
			{
				g_stored_Rect[g_count_Rect].left = i;
				g_stored_Rect[g_count_Rect].right = i + g_length;
				g_stored_Rect[g_count_Rect].top = j;
				g_stored_Rect[g_count_Rect].bottom = j + g_length;
				g_count_Rect++;
			}

		}
	}
	double result = cal_Diff(Form_Image, &Blur_Image);
	free(Blur_Image.pSrc);
	//return 1/(result+(1e-7));
	return -result;
}
