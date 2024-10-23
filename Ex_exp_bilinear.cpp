#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

typedef unsigned char  BYTE;
#define WIDTHBYTE(bits) ((bits+31)/32*4)

// 文件头
typedef struct {
	unsigned short   fileType;
	unsigned short   fileSizeLow;
	unsigned short   fileSizeHigh;
	unsigned short   reserved1;
	unsigned short   reserved2;
	unsigned short   dataOffsetLow;
	unsigned short   dataOffsetHigh;
}BMPHeader;

// 信息头
typedef struct {
	unsigned long  headerSize;
	long  width;
	long  height;
	unsigned short   planes;
	unsigned short   bitsPerPixel;
	unsigned long   compression;
	unsigned long   imageSize;
	long  xPixelsPerMeter;
	long  yPixelsPerMeter;
	unsigned long   colorsUsed;
	unsigned long   importantColors;
} BMPInfoHeader;

// 双线性差值放大函数声明
void BilinearInterpolation(
	double originX, 
	double originY, 
	int newX, 
	int newY, 
	int originWidth, 
	int originHeight, 
	BYTE* originData,
	BYTE* newData, 
	int originByte, 
	int newByte);


int main()
{
	FILE* inputFile;
	FILE* outputFile;

	BMPHeader originHeader;
	BMPInfoHeader originInfoHeader;

	// 打开输入文件
	inputFile = fopen("input.bmp", "rb");
	if (inputFile == NULL)
	{
		printf("图片打开失败！\n");
		return -1;
	}

	// 读取文件头
	fread(&originHeader, sizeof(BMPHeader), 1, inputFile);

	// 读取信息头
	fread(&originInfoHeader, sizeof(BMPInfoHeader), 1, inputFile);

	// 确保是24位色深的BMP文件
	if (originInfoHeader.bitsPerPixel != 24) {
		printf("仅支持24位色深的bmp文件！\n");
		fclose(inputFile);
		return -1;
	}

	// 输入图像数据
	int originWidth = originInfoHeader.width;
	int originHeight = originInfoHeader.height;
	int originByte = WIDTHBYTE(originWidth * originInfoHeader.bitsPerPixel);// 源图像行字节数
	int originAllByte = originHeight * originByte;// 源图像总字节数

	// 分配内存来保存像素数据
	BYTE* imageInfo = (BYTE*)malloc(originAllByte);
	memset(imageInfo, 0, originAllByte);
	fread(imageInfo, 1, originAllByte, inputFile);

	// 输出图像数据
	int newWidth = originWidth * 3;  // 长宽均放大三倍
	int newHeight = originHeight * 3;
	int newByte = WIDTHBYTE(newWidth * originInfoHeader.bitsPerPixel);  // 放大后图像行字节数
	int newAllByte = newHeight * newByte;

	// 分配内存来保存像素数据
	BYTE* outputInfo = (BYTE*)malloc(newAllByte);
	memset(outputInfo, 0, newAllByte);

	// 放大倍率
	double rateWidth = (double)originWidth / newWidth;
	double rateHeight = (double)originHeight / newHeight;

	// 实现双线性差值
	for (int i = 0; i < newHeight; i++)
		for (int j = 0; j < newWidth; j++)
		{
			double originX = j * rateWidth;
			double originY = i * rateHeight;// 放大后图像像素点对应原图像位置
			BilinearInterpolation(
				originX,
				originY,
				j,
				i,
				originWidth,
				originHeight,
				imageInfo,
				outputInfo,
				originByte,
				newByte);
		}


	//  更新输出图像的文件头与信息头
	BMPHeader outputHeader;
	BMPInfoHeader outputInfoHeader;
	outputHeader = originHeader;
	outputInfoHeader = originInfoHeader;
	outputInfoHeader.width = newWidth;
	outputInfoHeader.height = newHeight;
	outputInfoHeader.imageSize = newAllByte;

	// 生成输出图像
	outputFile = fopen("output.bmp", "wb");
	if (outputFile == NULL)
	{
		printf("创建结果图像失败！\n");
		fclose(inputFile);
		free(imageInfo);
		return -1;
	}

	// 写入文件头与信息头
	fwrite(&outputHeader, 1, sizeof(BMPHeader), outputFile);
	fwrite(&outputInfoHeader, 1, sizeof(BMPInfoHeader), outputFile);

	// 写入双线性差值放大后的图像数据
	fwrite(outputInfo, 1, newAllByte, outputFile);
	printf("双线性差值放大成功！\n");

	// 关闭文件与释放空间
	fclose(inputFile);
	fclose(outputFile);
	free(imageInfo);
	free(outputInfo);


	return 0;
}

void BilinearInterpolation(double originX, double originY, int newX, int newY, int originWidth,
	int originHeight, BYTE* originData, BYTE* newData, int originByte, int newByte)
{
	// 取对应原图像坐标点的整数部分
	int x = originX;
	int y = originY;
	// 取对应原图像坐标点的小数部分
	double dx = originX - x;
	double dy = originY - y;

	// 左下、右下、左上、右上的四个像素点
	int A = y * originByte + x * 3;
	int B = y * originByte + (x + 1) * 3;
	int C = (y + 1) * originByte + x * 3;
	int D = (y + 1) * originByte + (x + 1) * 3;


	// 对原图像的RGB通道进行修改

	int output = newY * newByte + newX * 3; 
	// R通道
	newData[output] =
		originData[A] * (1 - dx) * (1 - dy) +
		originData[B] * dx * (1 - dy) +
		originData[C] * (1 - dx) * dy +
		originData[D] * dx * dy;

	// G通道
	newData[output + 1] =
		originData[A + 1] * (1 - dx) * (1 - dy) +
		originData[B + 1] * dx * (1 - dy) +
		originData[C + 1] * (1 - dx) * dy +
		originData[D + 1] * dx * dy;

	// B通道
	newData[output + 2] =
		originData[A + 2] * (1 - dx) * (1 - dy) +
		originData[B + 2] * dx * (1 - dy) +
		originData[C + 2] * (1 - dx) * dy +
		originData[D + 2] * dx * dy;

    // 边界处理
    if (x == (originWidth - 1))
    {
    	B = A;
    	D = C;
    }

    if (y == (originHeight - 1))
    {
    	C = A;
    	D = B;
    }
	return;
}