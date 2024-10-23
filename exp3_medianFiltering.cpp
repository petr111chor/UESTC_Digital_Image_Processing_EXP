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

// 中值滤波函数声明
void medianFilter(const BYTE* imageInfo, BYTE* filteredInfo, int width, int height);

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
    BYTE* filteredInfo = (BYTE*)malloc(infoHeader.imageSize);

    // 读取像素数据
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // 进行中值滤波
    medianFilter(imageInfo, filteredInfo, infoHeader.width, infoHeader.height);

    // 创建输出文件
    outputFile = fopen("output.bmp", "wb");
    if (outputFile == NULL) {
        printf("生成结果文件失败！\n");
        fclose(inputFile);
        free(imageInfo);
        free(filteredInfo);
        return -1;
    }

    // 写入文件头和信息头
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    // 写入滤波后的像素数据
    fseek(outputFile, header.dataOffset, SEEK_SET);
    fwrite(filteredInfo, 1, infoHeader.imageSize, outputFile);
    printf("中值滤波成功！\n");

    // 关闭文件并且释放空间
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(filteredInfo);

    return 0;
}

// 实现中值滤波
void medianFilter(const BYTE* imageInfo, BYTE* filteredInfo, int width, int height) {
    int model[9]; // 存储3x3邻域像素值

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = (i * width + j) * 3; // 当前像素的索引

            // 收集3x3邻域像素值
            int count = 0;
            for (int k = i - 1; k <= i + 1; k++) {
                for (int m = j - 1; m <= j + 1; m++) {
                    if (k >= 0 && k < height && m >= 0 && m < width) {
                        int neighborIndex = (k * width + m) * 3;
                        model[count++] = imageInfo[neighborIndex];
                    }
                }
            }

            // 对邻域像素值进行排序
            for (int k = 0; k < count - 1; k++) {
                for (int m = 0; m < count - k - 1; m++) {
                    if (model[m] > model[m + 1]) {
                        int temp = model[m];
                        model[m] = model[m + 1];
                        model[m + 1] = temp;
                    }
                }
            }

            // 将中值赋给当前像素
            filteredInfo[index] = model[count / 2];
            filteredInfo[index + 1] = model[count / 2];
            filteredInfo[index + 2] = model[count / 2];
        }
    }
}
