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

// ��������
void laplacianSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height);

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
    BYTE* sharpenedInfo = (BYTE*)malloc(infoHeader.imageSize);

    // ��ȡ��������
    fseek(inputFile, header.dataOffset, SEEK_SET);
    fread(imageInfo, 1, infoHeader.imageSize, inputFile);

    // ����������˹���˲�
    laplacianSharpen(imageInfo, sharpenedInfo, infoHeader.width, infoHeader.height);

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
    printf("������˹�񻯴�����ɣ�\n");

    // �ر��ļ������ͷſռ�
    fclose(inputFile);
    fclose(outputFile);
    free(imageInfo);
    free(sharpenedInfo);

    return 0;
}

// ʵ��������˹���˲�
void laplacianSharpen(const BYTE* imageInfo, BYTE* sharpenedInfo, int width, int height) {
    //������˹ģ��
    int laplacianKernel[3][3] = {
        {1, 1, 1},
        {1, -8, 1},
        {1, 1, 1}
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
            sharpenedInfo[index] = imageInfo[index]-sumR;
            sharpenedInfo[index + 1] = imageInfo[index+1]-sumG;
            sharpenedInfo[index + 2] = imageInfo[index+2]-sumB;
        }
    }
}