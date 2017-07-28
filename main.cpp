
#include "second.h"

int thresh_for_sobel_detect_contour = 30;


int main(int argc, char** argv)
{

	char * filePath = "D:\\internship\\test2";
	vector<string> files;

	getFiles(filePath, files);
	ofstream record;
	record.open("D:\\internship\\test2\\test.txt");

	char str[70];
	int size = files.size();
	for (int i = 0; i < size; i++)
	{
		Mat src = imread(files[i].c_str(),0);
		if (!src.data)
			continue;
		cout << " " << endl;
		cout << files[i].c_str() << endl;
		record << files[i].c_str() << endl;
		Mat grad = Mat::zeros(src.size(), CV_8UC1);;
		namedWindow("source", CV_WINDOW_AUTOSIZE);

		GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
		imshow("source", src);
		
		double result = calculate_scores(src, grad);
		
		cout << result << endl << endl;
		record << result << endl << endl;

	}
	record.close();
	waitKey(0);
	return 0;

}

double calculate_scores2(Mat Image, Mat grad)
{
	int nl = Image.rows, nc = Image.cols;
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	/// Generate grad_x and grad_y
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	/// Gradient X
	Sobel(Image, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	/// Gradient Y
	Sobel(Image, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);
	/// Total Gradient (approximate)
	//addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
	//namedWindow("sobel", CV_WINDOW_AUTOSIZE);
	//imshow("sobel", grad);
	double count = 0;
	double gradx_value, grady_value;
	for (int j = 0; j < nl; j++)//y
	{
		uchar* data = grad.ptr<uchar>(j);
		uchar* data1 = abs_grad_x.ptr<uchar>(j);
		uchar* data2 = abs_grad_y.ptr<uchar>(j);
		for (int i = 0; i < nc; i++)//x
		{
			gradx_value = (int)data1[i]*1.0/ 4;
			grady_value = (int)data2[i]*1.0 / 4;
			data[i] = (int)sqrt(pow(gradx_value, 2) + pow(grady_value, 2));
			if ((int)data[i]>thresh_for_sobel_detect_contour)
				count += pow(data[i], 2);
		}
	}
	double result = count*1.0 / (nl*nc);
	cout << endl;
	return result;

}

double calculate_scores(Mat Image, Mat grad)
{
	int nl = Image.rows, nc = Image.cols;
	int scale = 1;
	int delta = 0;
	int ddepth = -1;
	/// Generate grad_x and grad_y
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	/// Gradient X
	Sobel(Image, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	/// Gradient Y
	Sobel(Image, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);
	/// Total Gradient (approximate)
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
	namedWindow("sobel", CV_WINDOW_AUTOSIZE);
	imshow("sobel", grad);
	double count = 0;
	for (int j = 0; j < nl; j++)//y
	{
		uchar* data = grad.ptr<uchar>(j);
		for (int i = 0; i < nc; i++)//x
		{
			if ((int)data[i]>thresh_for_sobel_detect_contour)
			{
				count += (int)data[i] * (int)data[i];
			}
		}
	}
	double result = count*1.0 / (nl*nc);
	//double result = count;
	cout << endl;
	return result;

}



void getFiles(string path, vector<string>& files)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}