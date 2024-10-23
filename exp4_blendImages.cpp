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
void laplacianSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height);
void gradientSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height);
void blendImages(const BYTE* laplacianInfo, const BYTE* gradientInfo, BYTE* blendedInfo, int width, int height);

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
    BYTE* laplacianInfo = (BYTE*)malloc(infoHeader.imageSize);
    BYTE* gradientInfo = (BYTE*)malloc(infoHeader.imageSize);
    BYTE* blendedInfo = (BYTE*)malloc(infoHeader.imageSize);

    // ��ȡ��������
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // ����������˹���˲�
    laplacianSharpen(imageInfo, laplacianInfo, infoHeader.width, infoHeader.height);

    // �����ݶ����˲�
    gradientSharpen(imageInfo, gradientInfo, infoHeader.width, infoHeader.height);

    // ��������˹�񻯺��ݶ��񻯵Ľ�����
    blendImages(laplacianInfo, gradientInfo, blendedInfo, infoHeader.width, infoHeader.height);

    // ��������ļ�
    outputFile = fopen("output.bmp", "wb");
    if (outputFile == NULL) {
        printf("��������ļ�ʧ�ܣ�\n");
        fclose(inputFile);
        free(imageInfo);
        free(laplacianInfo);
        free(gradientInfo);
        free(blendedInfo);
        return -1;
    }

    // д���ļ�ͷ����Ϣͷ
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    // д���Ϻ����������
    fseek(outputFile, header.dataOffset, SEEK_SET);
    fwrite(blendedInfo, 1, infoHeader.imageSize, outputFile);
    printf("��Ͽռ��˲�������ɣ�\n");

    // �ر��ļ������ͷſռ�
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(laplacianInfo);
    free(gradientInfo);
    free(blendedInfo);

    return 0;
}

// ʵ��������˹���˲�
void laplacianSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height) {
    //������˹ģ��
    int laplacianKernel[3][3] = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}
    };

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sumR = 0, sumG = 0, sumB = 0;
            // Ӧ��������˹ģ��
            for (int k = -1; k <= 1; k++) {
                for (int m = -1; m <= 1; m++) {
                    int ni = i + k;
                    int nj = j + m;
                    int index = (ni * width + nj) * 3; // ÿ������ռ3���ֽ�
                    sumR += imageInfo[index] * laplacianKernel[k + 1][m + 1];
                    sumG += imageInfo[index + 1] * laplacianKernel[k + 1][m + 1];
                    sumB += imageInfo[index + 2] * laplacianKernel[k + 1][m + 1];
                }
            }
            // ȷ������ֵ��0~255֮��
            sumR = sumR < 0 ? 0 : (sumR > 255 ? 255 : sumR);
            sumG = sumG < 0 ? 0 : (sumG > 255 ? 255 : sumG);
            sumB = sumB < 0 ? 0 : (sumB > 255 ? 255 : sumB);
            // �洢�񻯺������ֵ
            int index = (i * width + j) * 3;
            sharpenedInfo[index] = sumR;
            sharpenedInfo[index + 1] = sumG;
            sharpenedInfo[index + 2] = sumB;
        }
    }
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

// ʵ��ͼ����
void blendImages(const BYTE* laplacianInfo, const BYTE* gradientInfo, BYTE* blendedInfo, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = (i * width + j) * 3;
            // ���������˹�񻯺��ݶ��񻯵Ľ��
            blendedInfo[index] = (laplacianInfo[index] + gradientInfo[index]) / 2;
            blendedInfo[index + 1] = (laplacianInfo[index + 1] + gradientInfo[index + 1]) / 2;
            blendedInfo[index + 2] = (laplacianInfo[index + 2] + gradientInfo[index + 2]) / 2;
        }
    }
}
