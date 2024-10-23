#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#define Height 6000
#define Width 4000
typedef unsigned char  BYTE;

int main() {
    BITMAPINFOHEADER head;//信息头
    BITMAPFILEHEADER bfhead;//文件头
    RGBQUAD* BMPColorTable;//调色板

    //读取待修改文件的BMP数据
    FILE* fp1 = fopen("test.bmp", "rb");
    if (fp1 == 0)
    {
        printf("测试文件打开失败!");
        return -1;
    }
    

    fread(&head, 40, 1, fp1);
    fseek(fp1, 1078, 0);//跳过BMP文件头

    BYTE* ptr;
    BYTE** matrix = new BYTE * [Height];

    for (int i = 0; i != Height; i++)
    {
        matrix[i] = new BYTE[Width];
    }

    ptr = (BYTE*)malloc(Width * Height * sizeof(BYTE));
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            fread(ptr, 1, 1, fp1);
            matrix[i][j] = *ptr;
            ptr++;
        }
    }
    fclose(fp1);

    int hist[256] = {0};
    float fp_hist[256] = {0};
    int eq_hist[256] = {0};
    float eq_hist_t[256] = {0};
    int size = Height * Width;


    //计算直方图
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            BYTE GrayScale = matrix[i][j];
            hist[GrayScale]++;
        }
    }

    //计算直方图密度
    for (int i = 0; i < 256; i++)
    {
        fp_hist[i] = (float)hist[i] / (float)size;
    }

    //计算累计直方图分布
    for (int i = 0; i < 256; i++)
    {
        if (i == 0)
        {
            eq_hist_t[i] = fp_hist[i];
        }
        else
        {
            eq_hist_t[i] = eq_hist_t[i - 1] + fp_hist[i];
        }
    }

    //确定映射关系
    for (int i = 0; i < 256; i++)
    {
        eq_hist[i] = (int)(eq_hist_t[i] * 255.0 + 0.5);
    }
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            BYTE GrayScale = matrix[i][j];
            matrix[i][j] = eq_hist[GrayScale];
        }
    }

    //生成结果图像
    FILE* fp2 = fopen("result.bmp", "wb");
    if (fp2 == 0)
    {
        printf("结果文件打开失败!");
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

    BMPColorTable = new RGBQUAD[256];
    for (int i = 0; i < 256; i++) {
        BMPColorTable[i].rgbRed = i;
        BMPColorTable[i].rgbGreen = i;
        BMPColorTable[i].rgbBlue = i; //调色板里的B、G、R分量都相等，且等于索引值
    }
    fwrite(BMPColorTable, sizeof(RGBQUAD), 256, fp2); //将颜色表写入"result.bmp";


    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            fwrite(&matrix[i][j], 1, 1, fp2);
        }
    }
    printf("均衡化成功\n");
    fclose(fp2);

    // 释放内存
    for (int i = 0; i < Height; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}