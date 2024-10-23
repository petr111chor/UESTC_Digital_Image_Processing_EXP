#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

typedef unsigned char  BYTE;
#define WIDTHBYTE(bits) ((bits+31)/32*4)

// �ļ�ͷ
typedef struct {
	unsigned short   fileType;
	unsigned short   fileSizeLow;
	unsigned short   fileSizeHigh;
	unsigned short   reserved1;
	unsigned short   reserved2;
	unsigned short   dataOffsetLow;
	unsigned short   dataOffsetHigh;
}BMPHeader;

// ��Ϣͷ
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

// ˫���Բ�ֵ�Ŵ�������
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

	// �������ļ�
	inputFile = fopen("input.bmp", "rb");
	if (inputFile == NULL)
	{
		printf("ͼƬ��ʧ�ܣ�\n");
		return -1;
	}

	// ��ȡ�ļ�ͷ
	fread(&originHeader, sizeof(BMPHeader), 1, inputFile);

	// ��ȡ��Ϣͷ
	fread(&originInfoHeader, sizeof(BMPInfoHeader), 1, inputFile);

	// ȷ����24λɫ���BMP�ļ�
	if (originInfoHeader.bitsPerPixel != 24) {
		printf("��֧��24λɫ���bmp�ļ���\n");
		fclose(inputFile);
		return -1;
	}

	// ����ͼ������
	int originWidth = originInfoHeader.width;
	int originHeight = originInfoHeader.height;
	int originByte = WIDTHBYTE(originWidth * originInfoHeader.bitsPerPixel);// Դͼ�����ֽ���
	int originAllByte = originHeight * originByte;// Դͼ�����ֽ���

	// �����ڴ���������������
	BYTE* imageInfo = (BYTE*)malloc(originAllByte);
	memset(imageInfo, 0, originAllByte);
	fread(imageInfo, 1, originAllByte, inputFile);

	// ���ͼ������
	int newWidth = originWidth * 3;  // ������Ŵ�����
	int newHeight = originHeight * 3;
	int newByte = WIDTHBYTE(newWidth * originInfoHeader.bitsPerPixel);  // �Ŵ��ͼ�����ֽ���
	int newAllByte = newHeight * newByte;

	// �����ڴ���������������
	BYTE* outputInfo = (BYTE*)malloc(newAllByte);
	memset(outputInfo, 0, newAllByte);

	// �Ŵ���
	double rateWidth = (double)originWidth / newWidth;
	double rateHeight = (double)originHeight / newHeight;

	// ʵ��˫���Բ�ֵ
	for (int i = 0; i < newHeight; i++)
		for (int j = 0; j < newWidth; j++)
		{
			double originX = j * rateWidth;
			double originY = i * rateHeight;// �Ŵ��ͼ�����ص��Ӧԭͼ��λ��
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


	//  �������ͼ����ļ�ͷ����Ϣͷ
	BMPHeader outputHeader;
	BMPInfoHeader outputInfoHeader;
	outputHeader = originHeader;
	outputInfoHeader = originInfoHeader;
	outputInfoHeader.width = newWidth;
	outputInfoHeader.height = newHeight;
	outputInfoHeader.imageSize = newAllByte;

	// �������ͼ��
	outputFile = fopen("output.bmp", "wb");
	if (outputFile == NULL)
	{
		printf("�������ͼ��ʧ�ܣ�\n");
		fclose(inputFile);
		free(imageInfo);
		return -1;
	}

	// д���ļ�ͷ����Ϣͷ
	fwrite(&outputHeader, 1, sizeof(BMPHeader), outputFile);
	fwrite(&outputInfoHeader, 1, sizeof(BMPInfoHeader), outputFile);

	// д��˫���Բ�ֵ�Ŵ���ͼ������
	fwrite(outputInfo, 1, newAllByte, outputFile);
	printf("˫���Բ�ֵ�Ŵ�ɹ���\n");

	// �ر��ļ����ͷſռ�
	fclose(inputFile);
	fclose(outputFile);
	free(imageInfo);
	free(outputInfo);


	return 0;
}

void BilinearInterpolation(double originX, double originY, int newX, int newY, int originWidth,
	int originHeight, BYTE* originData, BYTE* newData, int originByte, int newByte)
{
	// ȡ��Ӧԭͼ����������������
	int x = originX;
	int y = originY;
	// ȡ��Ӧԭͼ��������С������
	double dx = originX - x;
	double dy = originY - y;

	// ���¡����¡����ϡ����ϵ��ĸ����ص�
	int A = y * originByte + x * 3;
	int B = y * originByte + (x + 1) * 3;
	int C = (y + 1) * originByte + x * 3;
	int D = (y + 1) * originByte + (x + 1) * 3;


	// ��ԭͼ���RGBͨ�������޸�

	int output = newY * newByte + newX * 3; 
	// Rͨ��
	newData[output] =
		originData[A] * (1 - dx) * (1 - dy) +
		originData[B] * dx * (1 - dy) +
		originData[C] * (1 - dx) * dy +
		originData[D] * dx * dy;

	// Gͨ��
	newData[output + 1] =
		originData[A + 1] * (1 - dx) * (1 - dy) +
		originData[B + 1] * dx * (1 - dy) +
		originData[C + 1] * (1 - dx) * dy +
		originData[D + 1] * dx * dy;

	// Bͨ��
	newData[output + 2] =
		originData[A + 2] * (1 - dx) * (1 - dy) +
		originData[B + 2] * dx * (1 - dy) +
		originData[C + 2] * (1 - dx) * dy +
		originData[D + 2] * dx * dy;

    // �߽紦��
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