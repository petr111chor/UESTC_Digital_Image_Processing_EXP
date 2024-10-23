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

// 函数声明
void laplacianSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height);

int main() {
    FILE* inputFile;
    FILE* outputFile;
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

    // 进行拉普拉斯锐化滤波
    laplacianSharpen(imageInfo, sharpenedInfo, infoHeader.width, infoHeader.height);

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
    printf("拉普拉斯锐化处理完成！\n");

    // 关闭文件并且释放空间
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(sharpenedInfo);

    return 0;
}

// 实现拉普拉斯锐化滤波
void laplacianSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height) {
    //拉普拉斯模板
    int laplacianKernel[3][3] = {
        {1, 1, 1},
        {1, -8, 1},
        {1, 1, 1}
    };

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sumR = 0, sumG = 0, sumB = 0;
            // 应用拉普拉斯模板
            for (int k = -1; k <= 1; k++) {
                for (int m = -1; m <= 1; m++) {
                    int ni = i + k;
                    int nj = j + m;
                    int index = (ni * width + nj) * 3; // 每个像素占3个字节
                    sumR += imageInfo[index] * laplacianKernel[k + 1][m + 1];
                    sumG += imageInfo[index + 1] * laplacianKernel[k + 1][m + 1];
                    sumB += imageInfo[index + 2] * laplacianKernel[k + 1][m + 1];
                }
            }
            // 确保像素值在0~255之间
            sumR = sumR < 0 ? 0 : (sumR > 255 ? 255 : sumR);
            sumG = sumG < 0 ? 0 : (sumG > 255 ? 255 : sumG);
            sumB = sumB < 0 ? 0 : (sumB > 255 ? 255 : sumB);
            // 存储锐化后的像素值
            int index = (i * width + j) * 3;
            sharpenedInfo[index] = imageInfo[index]-sumR;
            sharpenedInfo[index + 1] = imageInfo[index+1]-sumG;
            sharpenedInfo[index + 2] = imageInfo[index+2]-sumB;
        }
    }
}