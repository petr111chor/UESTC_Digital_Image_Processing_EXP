#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
typedef unsigned char BYTE;

#pragma pack(1)

// BMP�ļ�ͷ�ṹ
typedef struct {
    char signature[2];
    unsigned int fileSize;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int dataOffset;
} BMPHeader;

// BMP��Ϣͷ�ṹ
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

// ��ֵ�˲���������
void medianFilter(const BYTE* imageInfo, BYTE* filteredInfo, int width, int height);

int main() {
    FILE* inputFile;
    FILE* outputFile;
    BMPHeader header;
    BMPInfoHeader infoHeader;

    // �������ļ�
    inputFile = fopen("input.bmp", "rb");
    if (inputFile == NULL) {
        printf("���ļ�ʧ�ܣ�\n");
        return -1;
    }

    // ��ȡ�ļ�ͷ
    fread(&header, sizeof(BMPHeader), 1, inputFile);

    // ��ȡ��Ϣͷ
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, inputFile);

    // ȷ����24λɫ���BMP�ļ�
    if (infoHeader.bitsPerPixel != 24) {
        printf("��֧��24λɫ���bmp�ļ���\n");
        fclose(inputFile);
        return -1;
    }

    // �����ڴ���������������
    BYTE* imageInfo = (BYTE*)malloc(infoHeader.imageSize);
    BYTE* filteredInfo = (BYTE*)malloc(infoHeader.imageSize);

    // ��ȡ��������
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // ������ֵ�˲�
    medianFilter(imageInfo, filteredInfo, infoHeader.width, infoHeader.height);

    // ��������ļ�
    outputFile = fopen("output.bmp", "wb");
    if (outputFile == NULL) {
        printf("���ɽ���ļ�ʧ�ܣ�\n");
        fclose(inputFile);
        free(imageInfo);
        free(filteredInfo);
        return -1;
    }

    // д���ļ�ͷ����Ϣͷ
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    // д���˲������������
    fseek(outputFile, header.dataOffset, SEEK_SET);
    fwrite(filteredInfo, 1, infoHeader.imageSize, outputFile);
    printf("��ֵ�˲��ɹ���\n");

    // �ر��ļ������ͷſռ�
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(filteredInfo);

    return 0;
}

// ʵ����ֵ�˲�
void medianFilter(const BYTE* imageInfo, BYTE* filteredInfo, int width, int height) {
    int model[9]; // �洢3x3��������ֵ

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = (i * width + j) * 3; // ��ǰ���ص�����

            // �ռ�3x3��������ֵ
            int count = 0;
            for (int k = i - 1; k <= i + 1; k++) {
                for (int m = j - 1; m <= j + 1; m++) {
                    if (k >= 0 && k < height && m >= 0 && m < width) {
                        int neighborIndex = (k * width + m) * 3;
                        model[count++] = imageInfo[neighborIndex];
                    }
                }
            }

            // ����������ֵ��������
            for (int k = 0; k < count - 1; k++) {
                for (int m = 0; m < count - k - 1; m++) {
                    if (model[m] > model[m + 1]) {
                        int temp = model[m];
                        model[m] = model[m + 1];
                        model[m + 1] = temp;
                    }
                }
            }

            // ����ֵ������ǰ����
            filteredInfo[index] = model[count / 2];
            filteredInfo[index + 1] = model[count / 2];
            filteredInfo[index + 2] = model[count / 2];
        }
    }
}
