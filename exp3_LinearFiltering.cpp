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

// ƽ���˲���������
void smoothFilter(const BYTE* imageData, BYTE* smoothedData, int width, int height);

int main() {
    FILE* inputFile;
    FILE* outputFile;
    BMPHeader header;
    BMPInfoHeader infoHeader;

    // �������ļ�
    inputFile = fopen("input.bmp", "rb");
    if (inputFile == NULL) {
        printf("ͼƬ��ʧ�ܣ�\n");
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
    BYTE* smoothedInfo = (BYTE*)malloc(infoHeader.imageSize);

    // ��ȡ��������
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // ���о�ֵ�˲�
    smoothFilter(imageInfo, smoothedInfo, infoHeader.width, infoHeader.height);

    // ��������ļ�
    outputFile = fopen("output.bmp", "wb");
    if (outputFile == NULL) {
        printf("�������ͼ��ʧ�ܣ�\n");
        fclose(inputFile);
        free(imageInfo);
        free(smoothedInfo);
        return -1;
    }

    // д���ļ�ͷ����Ϣͷ
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    // д��ƽ�������������
    fseek(outputFile, header.dataOffset, SEEK_SET);
    fwrite(smoothedInfo, 1, infoHeader.imageSize, outputFile);
    printf("ƽ���˲��ɹ���\n");

    // �ر��ļ����ͷſռ�
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(smoothedInfo);
    return 0;
}

// ʵ�־�ֵ�˲�
void smoothFilter(const BYTE* imageInfo, BYTE* smoothedInfo, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sum = 0, sumR = 0, sumG = 0, sumB = 0;
            // ������Χ����
            for (int k = -1; k <= 1; k++) {
                for (int m = -1; m <= 1; m++) {
                    int ni = i + k;
                    int nj = j + m;
                    // �߽���
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                        int index = (ni * width + nj) * 3; // ÿ������ռ3���ֽ�
                        sumR += imageInfo[index];
                        sumG += imageInfo[index + 1];
                        sumB += imageInfo[index + 2];
                        sum++;
                    }
                }
            }
            // ����ƽ��ֵ���洢
            int index = (i * width + j) * 3;
            smoothedInfo[index] = sumR / sum;
            smoothedInfo[index + 1] = sumG / sum;
            smoothedInfo[index + 2] = sumB / sum;
        }
    }
}
