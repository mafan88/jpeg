//jenc.h
/*
 ����һ���򵥵�jpeg�������֧��1��1��1������baseline��ɫjpeg������ֻ����24bit��BMP�ļ�
 ����ṹֻ����˵����������̣��������ر���Ż���Ч�ʽ�Ϊһ�㡣
*/

#ifndef __JENC__
#define __JENC__

#include <string>
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include "jpegformat.h"
#include "jpeg.h"

using namespace std;

class JEnc
{
public:
 // bmFile:�����ļ�
 // jpgFile:����ļ�
 // Q:����
 void Invoke(string bmFile, string jpgFile, long Q);

private:

 FILE* pOutFile;
 int buffWidth;
 int buffHeight;
 int imgWidth;
 int imgHeight;


 // ��ȡBMP�ļ������������Ϣ
 BMBUFINFO GetBMBuffSize(FILE* pFile);
 
 // ��ȡͼ������
 void GetBMData(FILE* pFile, BYTE* pBuff, BMBUFINFO buffInfo);
 

 // ת��ɫ�ʿռ�BGR-YUV,111����
 void BGR2YUV111(BYTE* pBuf, BYTE* pYBuff, BYTE* pUBuff, BYTE* pVBuff);
 

 //********************************************************************
 // ��������:DivBuff 
 // ����޶�����:2003.5.3 
 //
 // ����˵��:
 // lpBuf:���뻺��,����������Ҳ�洢������
 // width:����X���򳤶�
 // height:����Y���򳤶�
 // xLen:X�����и��
 // yLen:Y�����и��
 //********************************************************************
 void DivBuff(BYTE* pBuf,UINT width,UINT height,UINT xLen,UINT yLen);
  

 //********************************************************************
 // ��������:SetQuantTable 
 //
 // ����˵��:����������������������
 //
 // ����˵��:
 // std_QT:��׼������
 // QT:���������
 // Q:��������
 //********************************************************************
 // ����������������������
 void SetQuantTable(const BYTE* std_QT,BYTE* QT, int Q);
 

 //Ϊfloat AA&N IDCT�㷨��ʼ��������
 void InitQTForAANDCT();
 

 //д�ļ���ʼ���
 void WriteSOI(void);

 //дAPP0��
 void WriteAPP0(void);


 //д��DQT��
 void WriteDQT(void);
 

 //д��SOF��
 void WriteSOF(void);
 

 //д��DHT��
 void WriteDHT(void);
 

 //д��SOS��
 void WriteSOS(void);
 
 //д���ļ��������
 void WriteEOI(void);
 


 // ����8λ�͵�8λ����
 USHORT Intel2Moto(USHORT val);
 

 //д1�ֽڵ��ļ�
 void WriteByte(BYTE val);
 

 // ���ɱ�׼Huffman��
 void BuildSTDHuffTab(BYTE* nrcodes,BYTE* stdTab,HUFFCODE* huffCode);
 

 // ����DU(���ݵ�Ԫ);
 void ProcessDU(FLOAT* lpBuf,FLOAT* quantTab,HUFFCODE* dcHuffTab,HUFFCODE* acHuffTab,SHORT* DC);
 

 //********************************************************************
 // ��������:ProcessData 
 //
 // ����˵��:����ͼ������FDCT-QUANT-HUFFMAN
 //
 // ����˵��:
 // lpYBuf:����Y�ź����뻺��
 // lpUBuf:ɫ��U�ź����뻺��
 // lpVBuf:ɫ��V�ź����뻺��
 //********************************************************************
 void ProcessData(BYTE* lpYBuf,BYTE* lpUBuf,BYTE* lpVBuf);
 

 // 8x8�ĸ�����ɢ���ұ任
 void FDCT(FLOAT* lpBuff);
 

 //********************************************************************
 // ��������:WriteBits 
 //
 // ����˵��:д���������
 //
 // ����˵��:
 // value:AC/DC�źŵ����
 //********************************************************************
 void WriteBits(HUFFCODE huffCode);
 
 void WriteBits(SYM2 sym);
 

 //********************************************************************
 // ��������:WriteBitsStream 
 //
 // ����˵��:д���������
 //
 // ����˵��:
 // value:��Ҫд���ֵ
 // codeLen:�����Ƴ���
 //********************************************************************
 void WriteBitsStream(USHORT value,BYTE codeLen);
 

 //********************************************************************
 // ��������:RLEComp 
 //
 // ����˵��:ʹ��RLE�㷨��ACѹ��,������������1,0,0,0,3,0,5 
 //     ���Ϊ(0,1);(3,3);(1,5);,��λ��ʾ��λ����ǰ0�ĸ���
 //          ��λ��4bits��ʾ,0�ĸ���������ʾ��Χ�����Ϊ(15,0);
 //          �����0��������һ�������б�ʾ.
 //
 // ����˵��:
 // lpbuf:���뻺��,8x8�任�źŻ���
 // lpOutBuf:�������,�ṹ����,�ṹ��Ϣ��ͷ�ļ�
 // resultLen:������峤��,���������ŵ�����
 //********************************************************************
 void RLEComp(SHORT* lpbuf,ACSYM* lpOutBuf,BYTE &resultLen);
 

 //********************************************************************
 // ��������:BuildSym2 
 //
 // ����˵��:���źŵ����VLI����,���ر��볤�Ⱥ��ź�����ķ���
 //
 // ����˵��:
 // value:AC/DC�źŵ����
 //********************************************************************
 SYM2 BuildSym2(SHORT value);
 


 //���ط��ŵĳ���
 BYTE ComputeVLI(SHORT val);
 

 //********************************************************************
 // ��������:BuildVLITable 
 //
 // ����˵��:����VLI��
 //
 // ����˵��:
 //********************************************************************
 void BuildVLITable(void);
 
};

#endif // __JENC__
