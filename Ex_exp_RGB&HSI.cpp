#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef unsigned char BYTE;

#pragma pack(1)
// 文件头
typedef struct {
	unsigned short bfType;
	unsigned int   bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;
} BITMAPFILEHEADER;


// 信息头
typedef struct {
	unsigned int   biSize;
	int            biWidth;
	int            biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int   biCompression;
	unsigned int   biSizeImage;
	int            biXPelsPerMeter;
	int            biYPelsPerMeter;
	unsigned int   biClrUsed;
	unsigned int   biClrImportant;
} BITMAPINFOHEADER;


// 调色板
typedef struct {
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;


BYTE getMin(BYTE R, BYTE G, BYTE B);

int main()
{
	FILE* file = fopen("test.bmp", "rb");

	// 打开输入文件
	if (file == NULL)
	{
		printf("文件打开失败！\n");
		return -1; 
	}
		

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	// 读取文件头
	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);

	// 读取信息头
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

	int width = infoHeader.biWidth;
	int height = infoHeader.biHeight;
	int bitCount = infoHeader.biBitCount;
	int bytePerLine = (bitCount * width / 8 + 3) / 4 * 4;// 行字节数

	RGBQUAD* pColorTable;
	pColorTable = (RGBQUAD*)malloc(256 * sizeof(RGBQUAD));
	if (pColorTable==NULL) {
		printf("调色板内存分配失败！\n");
		fclose(file);
		return -1;
	}

	// 读取颜色表
	fread(pColorTable, sizeof(RGBQUAD), 256, file);

	BYTE* imageInfo;
	imageInfo = (BYTE*)malloc(bytePerLine * height);
	if (imageInfo==NULL) {
		printf("文件信息内存分配失败！\n");
		free(pColorTable);
		fclose(file);
		return -1;
	}

	// 读取文件像素信息
	fread(imageInfo, bytePerLine * height, 1, file);
	fclose(file);


	// RGB与HSI通道的信息数组
	BYTE* channelR = (BYTE*)malloc(bytePerLine * height);
	BYTE* channelG = (BYTE*)malloc(bytePerLine * height);
	BYTE* channelB = (BYTE*)malloc(bytePerLine * height);
	BYTE* channelH = (BYTE*)malloc(bytePerLine * height);
	BYTE* channelS = (BYTE*)malloc(bytePerLine * height);
	BYTE* channelI = (BYTE*)malloc(bytePerLine * height);

	// 遍历输入图像以获取RGB与HSI通道的像素信息
	for (int i = 0; i < height; i++) 
	{
		unsigned char RGB[3] = {};
		for (int j = 0; j < bytePerLine; j++) 
		{
			int startPossion = i * bytePerLine;
			int channel = j % 3;
			RGB[channel] = *(imageInfo + j + startPossion);

			if (channel == 0) // G通道
			{
				*(channelR + j + startPossion) = *(imageInfo + j + startPossion + 1);
				*(channelG + j + startPossion) = *(imageInfo + j + startPossion);
				*(channelB + j + startPossion) = *(imageInfo + j + startPossion + 2);
			} 
			else if (channel == 1) // R通道
			{
				*(channelR + j + startPossion) = *(imageInfo + j + startPossion);
				*(channelG + j + startPossion) = *(imageInfo + j + startPossion - 1);
				*(channelB + j + startPossion) = *(imageInfo + j + startPossion + 1);
			}
			else if (channel == 2) // B通道
			{
				*(channelR + j + startPossion) = *(imageInfo + j + startPossion - 1);
				*(channelG + j + startPossion) = *(imageInfo + j + startPossion - 2);
				*(channelB + j + startPossion) = *(imageInfo + j + startPossion);

				BYTE R = *(imageInfo + j + startPossion - 1);
				BYTE G = *(imageInfo + j + startPossion - 2);
				BYTE B = *(imageInfo + j + startPossion);

				// 色调
				double upper = (R - G) - (R - B);
				double lower = 2 * sqrt(pow((R - G), 2) + (R - B) * (G - B));
				float angle = acos(upper / lower);
				if (G < B)
					*(channelH + j + startPossion) = (BYTE)((360 - angle) / 360 * 255);
				else
					*(channelH + j + startPossion) = (BYTE)(angle / 360 * 255);
				    *(channelH + j + startPossion - 1) = *(channelH + j + startPossion);
					*(channelH + j + startPossion - 2) = *(channelH + j + startPossion - 1);

				// 饱和度
				if (R == G && G == B)
				{
					*(channelS + j + startPossion) = 0;
				}
				else 
				{
					BYTE min = getMin(R,G,B);
					float sum = R + G + B;
					float a = min * 3.0 / sum;
					float b = 1 - a;
					BYTE S = (BYTE)(b * 255);
					*(channelS + j + startPossion) = S;
				}
				*(channelS + j + startPossion - 1) = *(channelS + j + startPossion);
				*(channelS + j + startPossion - 2) = *(channelS + j + startPossion - 1);

				// 亮度
				*(channelI + j + startPossion) = (BYTE)((*(imageInfo + j + startPossion - 1) + *(imageInfo + j + startPossion) + *(imageInfo + j + startPossion - 2)) / 3);
				*(channelI + j + startPossion - 1) = *(channelI + j + startPossion);
				*(channelI + j + startPossion - 2) = *(channelI + j + startPossion - 1);
			}
		}
	}

	// 生成结果文件
	FILE* resultFiles[6];
	const char* channels[6] = { "result_R.bmp", "result_G.bmp", "result_B.bmp", "result_H.bmp", "result_S.bmp", "result_I.bmp" };

	for (int i = 0; i < 6; i++) {
		resultFiles[i] = fopen(channels[i], "wb");
		if (!resultFiles[i]) {
			printf("生成文件失败！\n");
			for (int j = 0; j < i; j++)
				fclose(resultFiles[j]);
			free(channelR);
			free(channelG);
			free(channelB);
			free(channelH);
			free(channelS);
			free(channelI);
			free(imageInfo);
			free(pColorTable);
			return 1;
		}

		fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, resultFiles[i]);
		fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, resultFiles[i]);
		fwrite(pColorTable, sizeof(RGBQUAD), 256, resultFiles[i]);

		switch (i) {
		case 0:
			fwrite(channelR, bytePerLine * height, 1, resultFiles[i]);
			break;		   
		case 1:			   
			fwrite(channelG, bytePerLine * height, 1, resultFiles[i]);
			break;		   
		case 2:			   
			fwrite(channelB, bytePerLine * height, 1, resultFiles[i]);
			break;		   
		case 3:			   
			fwrite(channelH, bytePerLine * height, 1, resultFiles[i]);
			break;		   
		case 4:			   
			fwrite(channelS, bytePerLine * height, 1, resultFiles[i]);
			break;		   
		case 5:			   
			fwrite(channelI, bytePerLine * height, 1, resultFiles[i]);
			break;
		}

		fclose(resultFiles[i]);
	}

	// 释放内存
	free(channelR);
	free(channelG);
	free(channelB);
	free(channelH);
	free(channelS);
	free(channelI);
	free(imageInfo);
	free(pColorTable);

	return 0;
}

BYTE getMin(BYTE R, BYTE G, BYTE B) {
	BYTE min = (R < G) ? ((R < B) ? R : B) : ((G < B) ? G : B);
	return min;
}