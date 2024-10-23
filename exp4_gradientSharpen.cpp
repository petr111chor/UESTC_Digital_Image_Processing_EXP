#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

// ��������
void gradientSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height);

int main() {
    FILE* inputFile, * outputFile;
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
    BYTE* sharpenedInfo = (BYTE*)malloc(infoHeader.imageSize);

    // ��ȡ��������
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // �����ݶ����˲�
    gradientSharpen(imageInfo, sharpenedInfo, infoHeader.width, infoHeader.height);

    // ��������ļ�
    outputFile = fopen("output.bmp", "wb");
    if (outputFile == NULL) {
        printf("��������ļ�ʧ�ܣ�\n");
        fclose(inputFile);
        free(imageInfo);
        free(sharpenedInfo);
        return -1;
    }

    // д���ļ�ͷ����Ϣͷ
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    // д���񻯺����������
    fseek(outputFile, header.dataOffset, SEEK_SET);
    fwrite(sharpenedInfo, 1, infoHeader.imageSize, outputFile);
    printf("�ݶ��񻯴�����ɣ�\n");

    // �ر��ļ������ͷſռ�
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(sharpenedInfo);

    return 0;
}

// ʵ���ݶ����˲�
void gradientSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height) {
    //�ݶ�ģ��
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
            // ����X�����Y������ݶ�
            for (int k = -1; k <= 1; k++) {
                for (int m = -1; m <= 1; m++) {
                    int ni = i + k;
                    int nj = j + m;
                    int index = (ni * width + nj) * 3; // ÿ������ռ3���ֽ�
                    sumRX += imageInfo[index] * sobelX[k + 1][m + 1];
                    sumGX += imageInfo[index + 1] * sobelX[k + 1][m + 1];
                    sumBX += imageInfo[index + 2] * sobelX[k + 1][m + 1];
                    sumRY += imageInfo[index] * sobelY[k + 1][m + 1];
                    sumGY += imageInfo[index + 1] * sobelY[k + 1][m + 1];
                    sumBY += imageInfo[index + 2] * sobelY[k + 1][m + 1];
                }
            }
            // �����ݶȵ�ģֵ
            double gradientR = sqrt((double)(sumRX * sumRX + sumRY * sumRY));
            double gradientG = sqrt((double)(sumGX * sumGX + sumGY * sumGY));
            double gradientB = sqrt((double)(sumBX * sumBX + sumBY * sumBY));
            // ȷ���ݶ�ģֵ��0~255֮��
            int sharpR = (int)(gradientR < 0 ? 0 : (gradientR > 255 ? 255 : gradientR));
            int sharpG = (int)(gradientG < 0 ? 0 : (gradientG > 255 ? 255 : gradientG));
            int sharpB = (int)(gradientB < 0 ? 0 : (gradientB > 255 ? 255 : gradientB));
            // �洢�񻯺������ֵ
            int index = (i * width + j) * 3;
            sharpenedInfo[index] = sharpR;
            sharpenedInfo[index + 1] = sharpG;
            sharpenedInfo[index + 2] = sharpB;
        }
    }
}
