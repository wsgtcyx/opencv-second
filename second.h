
#include <io.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace cv;
using namespace std;
/** @function main */
void getFiles(string path, vector<string>& files);
double calculate_scores2(Mat Image, Mat grad);
double calculate_scores(Mat Image, Mat grad);