//Jpegformat.h

#ifndef __JPEGFORMAT__H__
#define __JPEGFORMAT__H__

//�ļ���ʼ,��ʼ���Ϊ0xFFD8
const static WORD SOITAG = 0xD8FF;

//�ļ�����,�������Ϊ0xFFD9
const static WORD EOITAG = 0xD9FF;

//JFIF APP0�νṹ
#pragma pack(push,1)
typedef struct tagJPEGAPP0
{
 WORD segmentTag;  //APP0�α�ǣ�����ΪFFE0
 WORD length;    //�γ��ȣ�һ��Ϊ16�����û������ͼ
 CHAR id[5];     //�ļ���� "JFIF" + "\0"
 WORD ver;      //�ļ��汾��һ��Ϊ0101��0102
 BYTE densityUnit; //�ܶȵ�λ��0=�޵�λ 1=����/Ӣ�� 2=����/����
 WORD densityX;   //X�᷽���ܶ�,ͨ��д1
 WORD densityY;   //Y�᷽���ܶ�,ͨ��д1
 BYTE thp;     //����ͼˮƽ������,д0
 BYTE tvp;     //����ͼ��ֱ������,д0
}JPEGAPP0;// = {0xE0FF,16,'J','F','I','F',0,0x0101,0,1,1,0,0};
#pragma pack(pop)

//JFIF APPN�νṹ
#pragma pack(push,1)
typedef struct tagJPEGAPPN
{
 WORD segmentTag;  //APPn�α�ǣ���FFE0 - FFEF n=0-F
 WORD length;    //�γ���   
}JPEGAPPN;
#pragma pack(pop)  

//JFIF DQT�νṹ(8 bits ������)
#pragma pack(push,1)
typedef struct tagJPEGDQT_8BITS
{
 WORD segmentTag;  //DQT�α�ǣ�����Ϊ0xFFDB
 WORD length;    //�γ���,������0x4300
 BYTE tableInfo;  //��������Ϣ
 BYTE table[64];  //������(8 bits)
}JPEGDQT_8BITS;
#pragma pack(pop)

//JFIF DQT�νṹ(8 bits ������)
#pragma pack(push,1)
typedef struct tagJPEGDQT_16BITS
{
 WORD segmentTag;  //DQT�α�ǣ�����Ϊ0xFFDB
 WORD length;    //�γ��ȣ�������0x8300
 BYTE tableInfo;  //��������Ϣ
 WORD table[64];   //������(16 bits)
}JPEGDQT_16BITS;
#pragma pack(pop)

//JFIF SOF0�νṹ(���)�����໹��SOF1-SOFF
#pragma pack(push,1)
typedef struct tagJPEGSOF0_24BITS
{
 WORD segmentTag;  //SOF�α�ǣ�����Ϊ0xFFC0
 WORD length;    //�γ��ȣ����ͼΪ17���Ҷ�ͼΪ11
 BYTE precision;  //���ȣ�ÿ���źŷ������õ�λ��������ϵͳΪ0x08
 WORD height;    //ͼ��߶�
 WORD width;     //ͼ����
 BYTE sigNum;   //�ź����������JPEGӦ��Ϊ3���Ҷ�Ϊ1
 BYTE YID;     //�źű�ţ�����Y
 BYTE HVY;     //������ʽ��0-3λ�Ǵ�ֱ������4-7λ��ˮƽ����
 BYTE QTY;     //��Ӧ�������
 BYTE UID;     //�źű�ţ�ɫ��U
 BYTE HVU;     //������ʽ��0-3λ�Ǵ�ֱ������4-7λ��ˮƽ����
 BYTE QTU;     //��Ӧ�������
 BYTE VID;     //�źű�ţ�ɫ��V
 BYTE HVV;     //������ʽ��0-3λ�Ǵ�ֱ������4-7λ��ˮƽ����
 BYTE QTV;     //��Ӧ�������
}JPEGSOF0_24BITS;// = {0xC0FF,0x0011,8,0,0,3,1,0x11,0,2,0x11,1,3,0x11,1};
#pragma pack(pop)

//JFIF SOF0�νṹ(�Ҷ�)�����໹��SOF1-SOFF
#pragma pack(push,1)
typedef struct tagJPEGSOF0_8BITS
{
 WORD segmentTag;  //SOF�α�ǣ�����Ϊ0xFFC0
 WORD length;    //�γ��ȣ����ͼΪ17���Ҷ�ͼΪ11
 BYTE precision;  //���ȣ�ÿ���źŷ������õ�λ��������ϵͳΪ0x08
 WORD height;    //ͼ��߶�
 WORD width;     //ͼ����
 BYTE sigNum;   //�ź����������JPEGӦ��Ϊ3���Ҷ�Ϊ1
 BYTE YID;     //�źű�ţ�����Y
 BYTE HVY;     //������ʽ��0-3λ�Ǵ�ֱ������4-7λ��ˮƽ����
 BYTE QTY;     //��Ӧ������� 
}JPEGSOF0_8BITS;// = {0xC0FF,0x000B,8,0,0,1,1,0x11,0};
#pragma pack(pop)

//JFIF DHT�νṹ
#pragma pack(push,1)
typedef struct tagJPEGDHT
{
 WORD segmentTag;  //DHT�α�ǣ�����Ϊ0xFFC4
 WORD length;    //�γ���
 BYTE tableInfo;  //����Ϣ������ϵͳ�� bit0-3 ΪHuffman���������bit4 Ϊ0ָDC��Huffman�� Ϊ1ָAC��Huffman��bit5-7����������Ϊ0
 BYTE huffCode[16];//1-16λ��Huffman���ֵ��������ֱ���������[1-16]��
 //BYTE* huffVal;  //���δ�Ÿ����ֶ�Ӧ��ֵ
}JPEGDHT;
#pragma pack(pop)

// JFIF SOS�νṹ����ʣ�
#pragma pack(push,1)
typedef struct tagJPEGSOS_24BITS
{
 WORD segmentTag;  //SOS�α�ǣ�����Ϊ0xFFDA
 WORD length;    //�γ��ȣ�������12
 BYTE sigNum;   //�źŷ����������ͼΪ0x03,�Ҷ�ͼΪ0x01
 BYTE YID;     //����Y�ź�ID,������1
 BYTE HTY;     //Huffman��ţ�bit0-3ΪDC�źŵı�bit4-7ΪAC�źŵı�
 BYTE UID;     //����Y�ź�ID,������2
 BYTE HTU;
 BYTE VID;     //����Y�ź�ID,������3
 BYTE HTV;
 BYTE Ss;     //����ϵͳ��Ϊ0
 BYTE Se;     //����ϵͳ��Ϊ63
 BYTE Bf;     //����ϵͳ��Ϊ0
}JPEGSOS_24BITS;// = {0xDAFF,0x000C,3,1,0,2,0x11,3,0x11,0,0x3F,0};
#pragma pack(pop)

// JFIF SOS�νṹ���Ҷȣ�
#pragma pack(push,1)
typedef struct tagJPEGSOS_8BITS
{
 WORD segmentTag;  //SOS�α�ǣ�����Ϊ0xFFDA
 WORD length;    //�γ��ȣ�������8
 BYTE sigNum;   //�źŷ����������ͼΪ0x03,�Ҷ�ͼΪ0x01
 BYTE YID;     //����Y�ź�ID,������1
 BYTE HTY;     //Huffman��ţ�bit0-3ΪDC�źŵı�bit4-7ΪAC�źŵı�  
 BYTE Ss;     //����ϵͳ��Ϊ0
 BYTE Se;     //����ϵͳ��Ϊ63
 BYTE Bf;     //����ϵͳ��Ϊ0
}JPEGSOS_8BITS;// = {0xDAFF,0x0008,1,1,0,0,0x3F,0};
#pragma pack(pop) 

// JFIF COM�νṹ
#pragma pack(push,1)
typedef struct tagJPEGCOM
{
 WORD segmentTag;  //COM�α�ǣ�����Ϊ0xFFFE
 WORD length;    //ע�ͳ���
}JPEGCOM;
#pragma pack(pop) 

#endif //__JPEGFORMAT__H__
