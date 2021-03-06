//Jpeg.h
#ifndef __JPEG__H__
#define __JPEG__H__

typedef struct tagBMBUFINFO
{
    UINT imgWidth;
    UINT imgHeight;
	UINT buffWidth;
	UINT buffHeight;
    WORD BitCount;
    BYTE padSize;    
}BMBUFINFO;

// DCT转换尺寸
static const BYTE DCTSIZE = 8;
// DCT转换块长度
static const BYTE DCTBLOCKSIZE = 64;  

//Huffman码结构
typedef struct tagHUFFCODE
{
 WORD code;  // huffman 码字
 BYTE length;// 编码长度
 WORD val;   // 码字对应的值
}HUFFCODE;
//AC信号中间符号结构
typedef struct tagACSYM
{
 BYTE zeroLen;  //0行程
 BYTE codeLen;  //幅度编码长度
 SHORT amplitude;//振幅
}ACSYM;

//DC/AC 中间符号2描述结构
typedef struct tagSYM2
{
 SHORT amplitude;//振幅
 BYTE codeLen;  //振幅长度(二进制形式的振幅数据的位数)
}SYM2;

//下面的extern 变量在jenc.cpp中定义
// 存放VLI表
extern BYTE VLI_TAB[4096];
extern BYTE* pVLITAB;                        //VLI_TAB的别名,使下标在-2048-2048

// 存放2个量化表
extern BYTE YQT[DCTBLOCKSIZE]; 
extern BYTE UVQT[DCTBLOCKSIZE]; 
// 存放2个FDCT变换要求格式的量化表
extern FLOAT YQT_DCT[DCTBLOCKSIZE];
extern FLOAT UVQT_DCT[DCTBLOCKSIZE];
//存放4个Huffman表
extern HUFFCODE STD_DC_Y_HT[12];
extern HUFFCODE STD_DC_UV_HT[12];
extern HUFFCODE STD_AC_Y_HT[256];
extern HUFFCODE STD_AC_UV_HT[256];


static BYTE bytenew=0; // The byte that will be written in the JPG file
static CHAR bytepos=7; //bit position in the byte we write (bytenew)
//should be<=7 and >=0
static USHORT mask[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

static const DOUBLE aanScaleFactor[8] = {1.0, 1.387039845, 1.306562965, 1.175875602,
1.0, 0.785694958, 0.541196100, 0.275899379};

//量化后DC范围在-2^11 - 2^11 - 1之间,量化后AC范围在-2^10 - 2^10 - 1之间
static const INT AC_MAX_QUANTED = 1023;   //量化后AC的最大值
static const INT AC_MIN_QUANTED = -1024;   //量化后AC的最小值
static const INT DC_MAX_QUANTED = 2047;   //量化后DC的最大值
static const INT DC_MIN_QUANTED = -2048;   //量化后DC的最小值


//标准亮度信号量化模板
const static BYTE std_Y_QT[64] = 
{
 16, 11, 10, 16, 24, 40, 51, 61,
  12, 12, 14, 19, 26, 58, 60, 55,
  14, 13, 16, 24, 40, 57, 69, 56,
  14, 17, 22, 29, 51, 87, 80, 62,
  18, 22, 37, 56, 68, 109,103,77,
  24, 35, 55, 64, 81, 104,113,92,
  49, 64, 78, 87, 103,121,120,101,
  72, 92, 95, 98, 112,100,103,99
};

//标准色差信号量化模板
const static BYTE std_UV_QT[64] = 
{
 17, 18, 24, 47, 99, 99, 99, 99,
  18, 21, 26, 66, 99, 99, 99, 99,
  24, 26, 56, 99, 99, 99, 99, 99,
  47, 66, 99 ,99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99
};

//正向 8x8 Z变换表
const static BYTE FZBT[64] =
{
 0, 1, 5, 6, 14,15,27,28,
  2, 4, 7, 13,16,26,29,42,
  3, 8, 12,17,25,30,41,43,
  9, 11,18,24,31,40,44,53,
  10,19,23,32,39,45,52,54,
  20,22,33,38,46,51,55,60,
  21,34,37,47,50,56,59,61,
  35,36,48,49,57,58,62,63 
};

//色彩空间系数常量,依次是411,111,211采样的系数,211采样的2种方式的系数相同
static const FLOAT COLORSPACECOEF[4][3] = {{1,0.25,0.25},{1,1,1},{1,0.5,0.5},{1,0.5,0.5}};
//MCU中各型号分量出现的比率
static const BYTE MCUIndex[4][3] = {{4,1,1},{1,1,1},{2,1,1},{2,1,1}};

// 标准Huffman表 (cf. JPEG standard section K.3) 
static BYTE STD_DC_Y_NRCODES[17]={0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
static BYTE STD_DC_Y_VALUES[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static BYTE STD_DC_UV_NRCODES[17]={0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
static BYTE STD_DC_UV_VALUES[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static BYTE STD_AC_Y_NRCODES[17]={0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0X7D };
static BYTE STD_AC_Y_VALUES[162]= {
 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
  0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
  0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
  0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
  0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
  0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
  0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
  0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
  0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
  0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
  0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
  0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
  0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
  0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
  0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
  0xf9, 0xfa };

 static BYTE STD_AC_UV_NRCODES[17]={0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0X77};
 static BYTE STD_AC_UV_VALUES[162]={
  0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
   0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
   0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
   0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
   0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
   0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
   0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
   0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
   0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
   0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
   0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
   0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
   0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
   0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
   0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
   0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
   0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
   0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
   0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
   0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
   0xf9, 0xfa };  

#endif //__JPEG__H__
