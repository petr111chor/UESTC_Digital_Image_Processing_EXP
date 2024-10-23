#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<windows.h>

int main()
{
	int Height;
	int Width;
	int BitCounter;
	unsigned char* matrix1;
	unsigned char* matrix2;
	RGBQUAD* BMPColorTable;//调色板
	BITMAPINFOHEADER head;//信息头
	BITMAPFILEHEADER bfhead;//文件头

	//读取待修改文件的BMP数据
	FILE* fp1 = fopen("test.bmp", "rb");
	if (fp1 == 0)
	{
		printf("文件打开失败!");
		return -1;
	}
	fseek(fp1, sizeof(BITMAPFILEHEADER), 0);//跳过BMP文件头
	fread(&head, 40, 1, fp1);
	Height = head.biHeight;
	Width = head.biWidth;
	BitCounter = head.biBitCount;
	fseek(fp1, sizeof(RGBQUAD), 1);
	int LineBytef1 = (Width * BitCounter / 8 + 3) / 4 * 4;//取4倍数，使每一行字节数均为4倍数
	matrix1 = new unsigned char[Height * LineBytef1];
	fread(matrix1, Height * LineBytef1, 1, fp1);
	fclose(fp1);

	FILE* fp2 = fopen("result.bmp", "wb");
	if (fp2 == 0)
	{
		printf("文件打开失败!");
		return -1;
	}

	int LineBytef2 = (Width * 8 / 8 + 3) / 4 * 4;

	//新建的"result.bmp"的文件头与信息头需进行修改，否则无法正常打开
	//修改"result,bmp"的文件头,修改内容分别为bfSize和bfOffBits
	bfhead.bfType = 0x4D42;
	bfhead.bfSize = 14 + 40 + 256 * sizeof(RGBQUAD) + LineBytef2 * Height;//修改文件大小
	bfhead.bfReserved1 = 0;
	bfhead.bfReserved2 = 0;
	bfhead.bfOffBits = 14 + 40 + 256 * sizeof(RGBQUAD);//修改偏移字节数
	fwrite(&bfhead, 14, 1, fp2);    //将修改后的文件头存入"result.bmp"

	//修改"result.bmp"的信息头，将biBitCount修改为8，将biSizeImage修改为LineBytef2 * Height
	BITMAPINFOHEADER head1;
	head1.biBitCount = 8;    //将每像素的位数改为8
	head1.biSizeImage = LineBytef2 * Height;//修改位图数据的大小
	head1.biClrImportant = 0;
	head1.biCompression = 0;
	head1.biClrUsed = 0;
	head1.biHeight = Height;
	head1.biWidth = Width;
	head1.biPlanes = 1;
	head1.biSize = 40;
	head1.biXPelsPerMeter = 0;
	head1.biYPelsPerMeter = 0;
	fwrite(&head1, 40, 1, fp2);  //将修改后的信息头存入"result.bmp"

	//通过BMP调色板对文件的灰度进行修改
	BMPColorTable = new RGBQUAD[256];
	for (int i = 0; i < 256; i++) {
		BMPColorTable[i].rgbRed = i;
		BMPColorTable[i].rgbGreen = i;
		BMPColorTable[i].rgbBlue = i; //调色板里的B、G、R分量都相等，且等于索引值
	}
	fwrite(BMPColorTable, sizeof(RGBQUAD), 256, fp2); //将颜色表写入"result.bmp";

	//写入BMP数据
	matrix2 = new unsigned char[LineBytef2 * Height];
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			unsigned char* pb1, * pb2;
			pb1 = matrix1 + j * 3 + i * LineBytef1;
			int y = *(pb1) * 0.33 + *(pb1 + 1) * 0.33 + *(pb1 + 2) * 0.33; //修改灰度值
			pb2 = matrix2 + i * LineBytef2 + j;
			*pb2 = y;
		}
	}
	fwrite(matrix2, LineBytef2 * Height, 1, fp2);
	fclose(fp2);
	printf("修改成功!\n");
	return 0;
}