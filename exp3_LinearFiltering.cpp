#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
typedef unsigned char BYTE;

#pragma pack(1)

// BMP文件头结构
typedef struct {
    char signature[2];
    unsigned int fileSize;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int dataOffset;
} BMPHeader;

// BMP信息头结构
typedef struct {
    unsigned int headerSize;
    int width;
    int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    int xPixelsPerMeter;
    int yPixelsPerMeter;
    unsigned int colorsUsed;
    unsigned int importantColors;
} BMPInfoHeader;

#pragma pack()

// 平滑滤波函数声明
void smoothFilter(const BYTE* imageData, BYTE* smoothedData, int width, int height);

int main() {
    FILE* inputFile;
    FILE* outputFile;
    BMPHeader header;
    BMPInfoHeader infoHeader;

    // 打开输入文件
    inputFile = fopen("input.bmp", "rb");
    if (inputFile == NULL) {
        printf("图片打开失败！\n");
        return -1;
    }

    // 读取文件头
    fread(&header, sizeof(BMPHeader), 1, inputFile);

    // 读取信息头
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, inputFile);

    // 确保是24位色深的BMP文件
    if (infoHeader.bitsPerPixel != 24) {
        printf("仅支持24位色深的bmp文件！\n");
        fclose(inputFile);
        return -1;
    }

    // 分配内存来保存像素数据
    BYTE* imageInfo = (BYTE*)malloc(infoHeader.imageSize);
    BYTE* smoothedInfo = (BYTE*)malloc(infoHeader.imageSize);

    // 读取像素数据
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // 进行均值滤波
    smoothFilter(imageInfo, smoothedInfo, infoHeader.width, infoHeader.height);

    // 创建输出文件
    outputFile = fopen("output.bmp", "wb");
    if (outputFile == NULL) {
        printf("创建结果图像失败！\n");
        fclose(inputFile);
        free(imageInfo);
        free(smoothedInfo);
        return -1;
    }

    // 写入文件头和信息头
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    // 写入平滑后的像素数据
    fseek(outputFile, header.dataOffset, SEEK_SET);
    fwrite(smoothedInfo, 1, infoHeader.imageSize, outputFile);
    printf("平滑滤波成功！\n");

    // 关闭文件与释放空间
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(smoothedInfo);
    return 0;
}

// 实现均值滤波
void smoothFilter(const BYTE* imageInfo, BYTE* smoothedInfo, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sum = 0, sumR = 0, sumG = 0, sumB = 0;
            // 遍历周围像素
            for (int k = -1; k <= 1; k++) {
                for (int m = -1; m <= 1; m++) {
                    int ni = i + k;
                    int nj = j + m;
                    // 边界检查
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                        int index = (ni * width + nj) * 3; // 每个像素占3个字节
                        sumR += imageInfo[index];
                        sumG += imageInfo[index + 1];
                        sumB += imageInfo[index + 2];
                        sum++;
                    }
                }
            }
            // 计算平均值并存储
            int index = (i * width + j) * 3;
            smoothedInfo[index] = sumR / sum;
            smoothedInfo[index + 1] = sumG / sum;
            smoothedInfo[index + 2] = sumB / sum;
        }
    }
}
