#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

// 函数声明
void gradientSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height);

int main() {
    FILE* inputFile, * outputFile;
    BMPHeader header;
    BMPInfoHeader infoHeader;

    // 打开输入文件
    inputFile = fopen("input.bmp", "rb");
    if (inputFile == NULL) {
        printf("打开文件失败！\n");
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
    BYTE* sharpenedInfo = (BYTE*)malloc(infoHeader.imageSize);

    // 读取像素数据
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // 进行梯度锐化滤波
    gradientSharpen(imageInfo, sharpenedInfo, infoHeader.width, infoHeader.height);

    // 创建输出文件
    outputFile = fopen("output.bmp", "wb");
    if (outputFile == NULL) {
        printf("创建结果文件失败！\n");
        fclose(inputFile);
        free(imageInfo);
        free(sharpenedInfo);
        return -1;
    }

    // 写入文件头和信息头
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    // 写入锐化后的像素数据
    fseek(outputFile, header.dataOffset, SEEK_SET);
    fwrite(sharpenedInfo, 1, infoHeader.imageSize, outputFile);
    printf("梯度锐化处理完成！\n");

    // 关闭文件并且释放空间
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(sharpenedInfo);

    return 0;
}

// 实现梯度锐化滤波
void gradientSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height) {
    //梯度模板
    int sobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int sobelY[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sumRX = 0, sumGX = 0, sumBX = 0;
            int sumRY = 0, sumGY = 0, sumBY = 0;
            // 计算X方向和Y方向的梯度
            for (int k = -1; k <= 1; k++) {
                for (int m = -1; m <= 1; m++) {
                    int ni = i + k;
                    int nj = j + m;
                    int index = (ni * width + nj) * 3; // 每个像素占3个字节
                    sumRX += imageInfo[index] * sobelX[k + 1][m + 1];
                    sumGX += imageInfo[index + 1] * sobelX[k + 1][m + 1];
                    sumBX += imageInfo[index + 2] * sobelX[k + 1][m + 1];
                    sumRY += imageInfo[index] * sobelY[k + 1][m + 1];
                    sumGY += imageInfo[index + 1] * sobelY[k + 1][m + 1];
                    sumBY += imageInfo[index + 2] * sobelY[k + 1][m + 1];
                }
            }
            // 计算梯度的模值
            double gradientR = sqrt((double)(sumRX * sumRX + sumRY * sumRY));
            double gradientG = sqrt((double)(sumGX * sumGX + sumGY * sumGY));
            double gradientB = sqrt((double)(sumBX * sumBX + sumBY * sumBY));
            // 确保梯度模值在0~255之间
            int sharpR = (int)(gradientR < 0 ? 0 : (gradientR > 255 ? 255 : gradientR));
            int sharpG = (int)(gradientG < 0 ? 0 : (gradientG > 255 ? 255 : gradientG));
            int sharpB = (int)(gradientB < 0 ? 0 : (gradientB > 255 ? 255 : gradientB));
            // 存储锐化后的像素值
            int index = (i * width + j) * 3;
            sharpenedInfo[index] = sharpR;
            sharpenedInfo[index + 1] = sharpG;
            sharpenedInfo[index + 2] = sharpB;
        }
    }
}
