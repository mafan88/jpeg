//jenc.cpp
/*
 ����һ���򵥵�jpeg�������֧��1��1��1������baseline��ɫjpeg������ֻ����24bit��BMP�ļ�
 ����ṹֻ����˵����������̣��������ر���Ż���Ч�ʽ�Ϊһ�㡣jpeg��ɫ���ǲ���YCrCbģʽ
 �ģ����Դ�BMP��jpegҪ����YUV��ת����
*/
#include "jenc.h"

// ���VLI��
BYTE VLI_TAB[4096];

// ���2��������
BYTE YQT[DCTBLOCKSIZE]; 
BYTE UVQT[DCTBLOCKSIZE]; 
// ���2��FDCT�任Ҫ���ʽ��������
FLOAT YQT_DCT[DCTBLOCKSIZE];
FLOAT UVQT_DCT[DCTBLOCKSIZE];
//���4��Huffman��
HUFFCODE STD_DC_Y_HT[12];
HUFFCODE STD_DC_UV_HT[12];
HUFFCODE STD_AC_Y_HT[256];
HUFFCODE STD_AC_UV_HT[256];


 // bmFile:�����ļ�
 // jpgFile:����ļ�
 // Q:����
void JEnc::Invoke(string bmFile, string jpgFile, long Q)
 {
  FILE* pFile;            // �����ļ����

  if ((pFile = fopen(bmFile.c_str(),"rb")) == NULL)   // ���ļ�
  { 
   throw("open bmp file error.");   
  }

  // ��ȡjpeg������Ҫ��bmp���ݽṹ��jpegҪ�����ݻ������ĸߺͿ�Ϊ8��16�ı���(�Ӳ�����ʽ����)
  BMBUFINFO bmBuffInfo = GetBMBuffSize(pFile);    
  imgWidth = bmBuffInfo.imgWidth;     // ͼ���
  imgHeight = bmBuffInfo.imgHeight;    // ͼ���
  buffWidth = bmBuffInfo.buffWidth;    // �����
  buffHeight = bmBuffInfo.buffHeight;    // �����
  size_t buffSize = buffHeight * buffWidth * 3; // ���峤�ȣ���Ϊ��24bits,����*3
  BYTE* bmData = new BYTE[buffSize];    // �����ڴ�ռ�
  GetBMData(pFile, bmData, bmBuffInfo);   // ��ȡ����
  fclose(pFile);         // �ر��ļ�

  //=====================================
  // ���������Ҫ�Ļ�������RGB�ź���Ҫ��ֱ���룬������Ҫ3��������������ֻ��1��1��1������һ����
  size_t yuvBuffSize = buffWidth * buffHeight; 
  BYTE* pYBuff = new BYTE[yuvBuffSize];
  BYTE* pUBuff = new BYTE[yuvBuffSize];
  BYTE* pVBuff = new BYTE[yuvBuffSize];
  // ��RGB�ź�ת��ΪYUV�ź�
  BGR2YUV111(bmData,pYBuff,pUBuff,pVBuff);
  // ���źŷָ�Ϊ8x8�Ŀ�
  DivBuff(pYBuff, buffWidth, buffHeight, DCTSIZE, DCTSIZE );  
  DivBuff(pUBuff, buffWidth, buffHeight, DCTSIZE, DCTSIZE );  
  DivBuff(pVBuff, buffWidth, buffHeight, DCTSIZE, DCTSIZE );  

  SetQuantTable(std_Y_QT,YQT, Q);         // ����Y������
  SetQuantTable(std_UV_QT,UVQT, Q);        // ����UV������  
  InitQTForAANDCT();            // ��ʼ��AA&N��Ҫ��������
  pVLITAB=VLI_TAB + 2047;                             // ����VLI_TAB�ı���
  BuildVLITable();            // ����VLI��   

  pOutFile = fopen(jpgFile.c_str(),"wb");

  // д�����
  WriteSOI();              
  WriteAPP0();
  WriteDQT();
  WriteSOF();
  WriteDHT();
  WriteSOS();

  // ����Y/UV�źŵĽ�ֱ������huffman������ʹ�ñ�׼��huffman�������Ǽ���ó���ȱ�����ļ��Գ��������ٶȿ�
  BuildSTDHuffTab(STD_DC_Y_NRCODES,STD_DC_Y_VALUES,STD_DC_Y_HT);
  BuildSTDHuffTab(STD_AC_Y_NRCODES,STD_AC_Y_VALUES,STD_AC_Y_HT);
  BuildSTDHuffTab(STD_DC_UV_NRCODES,STD_DC_UV_VALUES,STD_DC_UV_HT);
  BuildSTDHuffTab(STD_AC_UV_NRCODES,STD_AC_UV_VALUES,STD_AC_UV_HT);

  // ����Ԫ����
  ProcessData(pYBuff,pUBuff,pVBuff);  
  WriteEOI();

  fclose(pOutFile);
  delete[] bmData;
 }

// ��ȡBMP�ļ������������Ϣ
BMBUFINFO JEnc::GetBMBuffSize(FILE* pFile)
 {
  BITMAPFILEHEADER bmHead;       //�ļ�ͷ��Ϣ�� 
  BITMAPINFOHEADER bmInfo;       //ͼ��������Ϣ��
  BMBUFINFO   bmBuffInfo;
  UINT colSize = 0;
  UINT rowSize = 0;

  fseek(pFile,0,SEEK_SET);       //����дָ��ָ���ļ�ͷ��
  fread(&bmHead,sizeof(bmHead),1,pFile);    //��ȡ�ļ�ͷ��Ϣ��
  fread(&bmInfo,sizeof(bmInfo),1,pFile);    //��ȡλͼ��Ϣ��

  // ��������������jpeg����Ҫ�󻺳����ĸߺͿ�Ϊ8��16�ı���
  if (bmInfo.biWidth % 8 == 0)
  {
   colSize = bmInfo.biWidth;
  }
  else
  {
   colSize = bmInfo.biWidth + 8 - (bmInfo.biWidth % 8);
  }

  // ������������
  if (bmInfo.biHeight % 8 == 0)
  {
   rowSize = bmInfo.biHeight;
  }
  else
  {
   rowSize = bmInfo.biHeight + 8 - (bmInfo.biHeight % 8);
  }

  bmBuffInfo.BitCount = 24;
  bmBuffInfo.buffHeight = rowSize;   // ��������
  bmBuffInfo.buffWidth = colSize;    // ��������
  bmBuffInfo.imgHeight = bmInfo.biHeight;  // ͼ���
  bmBuffInfo.imgWidth = bmInfo.biWidth;  // ͼ���

  return bmBuffInfo;
 }

 // ��ȡͼ������
 void JEnc::GetBMData(FILE* pFile, BYTE* pBuff, BMBUFINFO buffInfo)
 { 
  BITMAPFILEHEADER bmHead;       // �ļ�ͷ��Ϣ�� 
  BITMAPINFOHEADER bmInfo;       // ͼ��������Ϣ��
  size_t    dataLen  = 0;    // �ļ�����������
  long    alignBytes = 0;     // Ϊ����4�ֽ���Ҫ������ֽ��� 
  UINT    lineSize  = 0;  

  fseek(pFile,0,SEEK_SET);       // ����дָ��ָ���ļ�ͷ��
  fread(&bmHead,sizeof(bmHead),1,pFile);    // ��ȡ�ļ�ͷ��Ϣ��
  fread(&bmInfo,sizeof(bmInfo),1,pFile);    // ��ȡλͼ��Ϣ��

  //���������ֽ���
  alignBytes = (((bmInfo.biWidth * bmInfo.biBitCount) + 31) & ~31) / 8L
   - (bmInfo.biWidth * bmInfo.biBitCount) / 8L; // ����ͼ���ļ����ݶ��в����ֽ���   

  //�������ݻ���������        
  lineSize = bmInfo.biWidth * 3;      
  // ��Ϊbmp�ļ������ǵ��õ����Դ����һ�п�ʼ��
  for (int i = bmInfo.biHeight - 1; i >= 0; --i)
  {   
   fread(&pBuff[buffInfo.buffWidth * i * 3],lineSize,1,pFile);   
   fseek(pFile,alignBytes,SEEK_CUR);             // ���������ֽ�            
  }  
 }

 // ת��ɫ�ʿռ�BGR-YUV,111����
 void JEnc::BGR2YUV111(BYTE* pBuf, BYTE* pYBuff, BYTE* pUBuff, BYTE* pVBuff)
 {
  DOUBLE tmpY   = 0;         //��ʱ����
  DOUBLE tmpU   = 0;
  DOUBLE tmpV   = 0;
  BYTE tmpB   = 0;       
  BYTE tmpG   = 0;
  BYTE tmpR   = 0;
  UINT i    = 0;
  size_t elemNum = _msize(pBuf) / 3;  //���峤��

  for (i = 0; i < elemNum; i++)
  {
   tmpB = pBuf[i * 3];
   tmpG = pBuf[i * 3 + 1];
   tmpR = pBuf[i * 3 + 2];
   tmpY = 0.299 * tmpR + 0.587 * tmpG + 0.114 * tmpB;
   tmpU = -0.1687 * tmpR - 0.3313 * tmpG + 0.5 * tmpB + 128;
   tmpV = 0.5 * tmpR - 0.4187 * tmpG - 0.0813 * tmpB + 128;
   //if(tmpY > 255){tmpY = 255;}     //�������
   //if(tmpU > 255){tmpU = 255;}
   //if(tmpV > 255){tmpV = 255;}
   //if(tmpY < 0){tmpY = 0;}  
   //if(tmpU < 0){tmpU = 0;}  
   //if(tmpV < 0){tmpV = 0;}
   pYBuff[i] = tmpY;           //�������뻺��
   pUBuff[i] = tmpU;
   pVBuff[i] = tmpV;
  }
 }

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
 void JEnc::DivBuff(BYTE* pBuf,UINT width,UINT height,UINT xLen,UINT yLen)
 {
  UINT xBufs   = width / xLen;             //X�᷽�����и�����
  UINT yBufs   = height / yLen;            //Y�᷽�����и�����
  UINT tmpBufLen  = xBufs * xLen * yLen;           //������ʱ����������
  BYTE* tmpBuf  = new BYTE[tmpBufLen];           //������ʱ����
  UINT i    = 0;               //��ʱ����
  UINT j    = 0;
  UINT k    = 0; 
  UINT n    = 0;
  UINT bufOffset  = 0;               //�иʼ��ƫ����

  for (i = 0; i < yBufs; ++i)               //ѭ��Y�����и�����
  {
   n = 0;                   //��λ��ʱ������ƫ����
   for (j = 0; j < xBufs; ++j)              //ѭ��X�����и�����  
   {   
    bufOffset = yLen * xLen * i * xBufs + j * xLen;        //���㵥Ԫ�źſ������ƫ����  
    for (k = 0; k < yLen; ++k)             //ѭ���������
    {
     memcpy(&tmpBuf[n],&pBuf[bufOffset],xLen);        //����һ�е���ʱ����
     n += xLen;                //������ʱ������ƫ����
     bufOffset += width;              //�������뻺����ƫ����
    }
   }
   memcpy(&pBuf[i * tmpBufLen],tmpBuf,tmpBufLen);         //������ʱ�������ݵ����뻺��
  } 
  delete[] tmpBuf;                 //ɾ����ʱ����
 } 

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
 void JEnc::SetQuantTable(const BYTE* std_QT,BYTE* QT, int Q)
 {
  INT tmpVal = 0;                  //��ʱ����
  DWORD i    = 0; 

  if (Q < 1) Q = 1;               //��������ϵ��
  if (Q > 100) Q = 100;

  //������ӳ�� 1->5000, 10->500, 25->200, 50->100, 75->50, 100->0
  if (Q < 50)
  {
   Q = 5000 / Q;
  }
  else
  {
   Q = 200 - Q * 2;
  }

  for (i = 0; i < DCTBLOCKSIZE; ++i)
  {
   tmpVal = (std_QT[i] * Q + 50L) / 100L;

   if (tmpVal < 1)                 //��ֵ��Χ�޶�
   {
    tmpVal = 1L;
   }
   if (tmpVal > 255)
   {
    tmpVal = 255L;
   }
   QT[FZBT[i]] = static_cast<BYTE>(tmpVal);
  } 
 }

 //Ϊfloat AA&N IDCT�㷨��ʼ��������
 void JEnc::InitQTForAANDCT()
 {
  UINT i = 0;           //��ʱ����
  UINT j = 0;
  UINT k = 0; 

  for (i = 0; i < DCTSIZE; i++)  //��ʼ�������ź�������
  {
   for (j = 0; j < DCTSIZE; j++)
   {
    YQT_DCT[k] = (FLOAT) (1.0 / ((DOUBLE) YQT[FZBT[k]] *
     aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
    ++k;
   }
  } 

  k = 0;
  for (i = 0; i < DCTSIZE; i++)  //��ʼ��ɫ���ź�������
  {
   for (j = 0; j < DCTSIZE; j++)
   {
    UVQT_DCT[k] = (FLOAT) (1.0 / ((DOUBLE) UVQT[FZBT[k]] *
     aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
    ++k;
   }
  } 
 }

 //д�ļ���ʼ���
 void JEnc::WriteSOI(void)
 { 
  fwrite(&SOITAG,sizeof(SOITAG),1,this->pOutFile);
 }
 //дAPP0��
 void JEnc::WriteAPP0(void)
 {
  JPEGAPP0 APP0;
  APP0.segmentTag  = 0xE0FF;
  APP0.length    = 0x1000;
  APP0.id[0]    = 'J';
  APP0.id[1]    = 'F';
  APP0.id[2]    = 'I';
  APP0.id[3]    = 'F';
  APP0.id[4]    = 0;
  APP0.ver     = 0x0101;
  APP0.densityUnit = 0x00;
  APP0.densityX   = 0x0100;
  APP0.densityY   = 0x0100;
  APP0.thp     = 0x00;
  APP0.tvp     = 0x00;
  fwrite(&APP0,sizeof(APP0),1,this->pOutFile);
 }

 //д��DQT��
 void JEnc::WriteDQT(void)
 {
  UINT i = 0;
  JPEGDQT_8BITS DQT_Y;
  DQT_Y.segmentTag = 0xDBFF;
  DQT_Y.length   = 0x4300;
  DQT_Y.tableInfo  = 0x00;
  for (i = 0; i < DCTBLOCKSIZE; i++)
  {
   DQT_Y.table[i] = YQT[i];
  }    
  fwrite(&DQT_Y,sizeof(DQT_Y),1,this->pOutFile);

  DQT_Y.tableInfo  = 0x01;
  for (i = 0; i < DCTBLOCKSIZE; i++)
  {
   DQT_Y.table[i] = UVQT[i];
  }
  fwrite(&DQT_Y,sizeof(DQT_Y),1,this->pOutFile); 
 }

 //д��SOF��
 void JEnc::WriteSOF(void)
 {
  JPEGSOF0_24BITS SOF;
  SOF.segmentTag = 0xC0FF;
  SOF.length   = 0x1100;
  SOF.precision  = 0x08;
  SOF.height   = Intel2Moto(USHORT(this->imgHeight));
  SOF.width    = Intel2Moto(USHORT(this->imgWidth)); 
  SOF.sigNum   = 0x03;
  SOF.YID     = 0x01; 
  SOF.QTY     = 0x00;
  SOF.UID     = 0x02;
  SOF.QTU     = 0x01;
  SOF.VID     = 0x03;
  SOF.QTV     = 0x01;
  SOF.HVU     = 0x11;
  SOF.HVV     = 0x11;
  /*switch (this->SamplingType)
  {
  case 1:
  SOF.HVY   = 0x11;
  break;

  case 2:
  SOF.HVY   = 0x12;
  break;

  case 3:
  SOF.HVY   = 0x21;
  break;

  case 4:
  SOF.HVY   = 0x22;
  break;
  }*/
  SOF.HVY   = 0x11;
  fwrite(&SOF,sizeof(SOF),1,this->pOutFile);
 }

 //д��DHT��
 void JEnc::WriteDHT(void)
 {
  UINT i = 0;

  JPEGDHT DHT;
  DHT.segmentTag = 0xC4FF;
  DHT.length   = Intel2Moto(19 + 12);
  DHT.tableInfo  = 0x00;
  for (i = 0; i < 16; i++)
  {
   DHT.huffCode[i] = STD_DC_Y_NRCODES[i + 1];
  } 
  fwrite(&DHT,sizeof(DHT),1,this->pOutFile);
  for (i = 0; i <= 11; i++)
  {
   WriteByte(STD_DC_Y_VALUES[i]);  
  }  
  //------------------------------------------------
  DHT.tableInfo  = 0x01;
  for (i = 0; i < 16; i++)
  {
   DHT.huffCode[i] = STD_DC_UV_NRCODES[i + 1];
  }
  fwrite(&DHT,sizeof(DHT),1,this->pOutFile);
  for (i = 0; i <= 11; i++)
  {
   WriteByte(STD_DC_UV_VALUES[i]);  
  } 
  //----------------------------------------------------
  DHT.length   = Intel2Moto(19 + 162);
  DHT.tableInfo  = 0x10;
  for (i = 0; i < 16; i++)
  {
   DHT.huffCode[i] = STD_AC_Y_NRCODES[i + 1];
  }
  fwrite(&DHT,sizeof(DHT),1,this->pOutFile);
  for (i = 0; i <= 161; i++)
  {
   WriteByte(STD_AC_Y_VALUES[i]);  
  }  
  //-----------------------------------------------------
  DHT.tableInfo  = 0x11;
  for (i = 0; i < 16; i++)
  {
   DHT.huffCode[i] = STD_AC_UV_NRCODES[i + 1];
  }
  fwrite(&DHT,sizeof(DHT),1,this->pOutFile); 
  for (i = 0; i <= 161; i++)
  {
   WriteByte(STD_AC_UV_VALUES[i]);  
  } 
 }

 //д��SOS��
 void JEnc::WriteSOS(void)
 {
  JPEGSOS_24BITS SOS;
  SOS.segmentTag   = 0xDAFF;
  SOS.length    = 0x0C00;
  SOS.sigNum    = 0x03;
  SOS.YID     = 0x01;
  SOS.HTY     = 0x00;
  SOS.UID     = 0x02;
  SOS.HTU     = 0x11;
  SOS.VID     = 0x03;
  SOS.HTV     = 0x11;
  SOS.Se     = 0x3F;
  SOS.Ss     = 0x00;
  SOS.Bf     = 0x00;
  fwrite(&SOS,sizeof(SOS),1,this->pOutFile); 
 }
 //д���ļ��������
 void JEnc::WriteEOI(void)
 {
  fwrite(&EOITAG,sizeof(EOITAG),1,this->pOutFile);
 }


 // ����8λ�͵�8λ����
 USHORT JEnc::Intel2Moto(USHORT val)
 {
  BYTE highBits = BYTE(val / 256);
  BYTE lowBits = BYTE(val % 256);

  return lowBits * 256 + highBits;
 }

 //д1�ֽڵ��ļ�
 void JEnc::WriteByte(BYTE val)
 {   
  fwrite(&val,sizeof(val),1,this->pOutFile);
 }

 // ���ɱ�׼Huffman��
 void JEnc::BuildSTDHuffTab(BYTE* nrcodes,BYTE* stdTab,HUFFCODE* huffCode)
 {
  BYTE i     = 0;             //��ʱ����
  BYTE j     = 0;
  BYTE k     = 0;
  USHORT code   = 0; 

  for (i = 1; i <= 16; i++)
  { 
   for (j = 1; j <= nrcodes[i]; j++)
   {   
    huffCode[stdTab[k]].code = code;
    huffCode[stdTab[k]].length = i;
    ++k;
    ++code;
   }
   code*=2;
  } 

  for (i = 0; i < k; i++)
  {
   huffCode[i].val = stdTab[i];  
  }
 }

 // ����DU(���ݵ�Ԫ)
 void JEnc::ProcessDU(FLOAT* lpBuf,FLOAT* quantTab,HUFFCODE* dcHuffTab,HUFFCODE* acHuffTab,SHORT* DC)
 {
  BYTE i    = 0;              //��ʱ����
  UINT j    = 0;
  SHORT diffVal = 0;                //DC����ֵ  
  BYTE acLen  = 0;               //�ر����AC�м���ŵ�����
  SHORT sigBuf[DCTBLOCKSIZE];              //�������źŻ���
  ACSYM acSym[DCTBLOCKSIZE];              //AC�м���Ż��� 

  FDCT(lpBuf);                 //��ɢ���ұ任

  for (i = 0; i < DCTBLOCKSIZE; i++)            //��������
  {          
   sigBuf[FZBT[i]] = (lpBuf[i] * quantTab[i] + 16384.5) - 16384;  
  }
  //-----------------------------------------------------
  //��DC�źű��룬д���ļ�
  //DPCM���� 
  diffVal = sigBuf[0] - *DC;
  *DC = sigBuf[0];
  //����Huffman��д����Ӧ������
  if (diffVal == 0)
  {  
   WriteBits(dcHuffTab[0]);  
  }
  else
  {   
   WriteBits(dcHuffTab[pVLITAB[diffVal]]);  
   WriteBits(BuildSym2(diffVal));    
  }
  //-------------------------------------------------------
  //��AC�źű��벢д���ļ�
  for (i = 63; (i > 0) && (sigBuf[i] == 0); i--) //�ж�ac�ź��Ƿ�ȫΪ0
  {
   //ע�⣬��ѭ��
  }
  if (i == 0)                //���ȫΪ0
  {
   WriteBits(acHuffTab[0x00]);           //д���������  
  }
  else
  { 
   RLEComp(sigBuf,&acSym[0],acLen);         //��AC���г��ȱ��� 
   for (j = 0; j < acLen; j++)           //���ζ�AC�м����Huffman����
   {   
    if (acSym[j].codeLen == 0)          //�Ƿ�������16��0
    {   
     WriteBits(acHuffTab[0xF0]);         //д��(15,0)    
    }
    else
    {
     WriteBits(acHuffTab[acSym[j].zeroLen * 16 + acSym[j].codeLen]); //
     WriteBits(BuildSym2(acSym[j].amplitude));    
    }   
   }
   if (i != 63)              //������λ��0������д��EOB
   {
    WriteBits(acHuffTab[0x00]);          
   }
  }
 }

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
 void JEnc::ProcessData(BYTE* lpYBuf,BYTE* lpUBuf,BYTE* lpVBuf)
 { 
  size_t yBufLen = _msize(lpYBuf);           //����Y���峤��
  size_t uBufLen = _msize(lpUBuf);           //ɫ��U���峤��          
  size_t vBufLen = _msize(lpVBuf);           //ɫ��V���峤��
  FLOAT dctYBuf[DCTBLOCKSIZE];            //Y�ź�FDCT������ʱ����
  FLOAT dctUBuf[DCTBLOCKSIZE];            //U�ź�FDCT������ʱ���� 
  FLOAT dctVBuf[DCTBLOCKSIZE];            //V�ź�FDCT������ʱ���� 
  UINT mcuNum   = 0;             //���MCU������ 
  SHORT yDC   = 0;             //Y�źŵĵ�ǰ���DC
  SHORT uDC   = 0;             //U�źŵĵ�ǰ���DC
  SHORT vDC   = 0;             //V�źŵĵ�ǰ���DC 
  BYTE yCounter  = 0;             //YUV�źŸ��Ե�д�������
  BYTE uCounter  = 0;
  BYTE vCounter  = 0;
  UINT i    = 0;             //��ʱ����              
  UINT j    = 0;                 
  UINT k    = 0;
  UINT p    = 0;
  UINT m    = 0;
  UINT n    = 0;
  UINT s    = 0; 

  mcuNum = (this->buffHeight * this->buffWidth * 3)
   / (DCTBLOCKSIZE * 3);         //����MCU������

  for (p = 0;p < mcuNum; p++)        //��������MCU��д��
  {
   yCounter = 1;//MCUIndex[SamplingType][0];   //��������ʽ��ʼ�����źż�����
   uCounter = 1;//MCUIndex[SamplingType][1];
   vCounter = 1;//MCUIndex[SamplingType][2];

   for (; i < yBufLen; i += DCTBLOCKSIZE)
   {
    for (j = 0; j < DCTBLOCKSIZE; j++)
    {
     dctYBuf[j] = FLOAT(lpYBuf[i + j] - 128);
    }   
    if (yCounter > 0)
    {    
     --yCounter;
     ProcessDU(dctYBuf,YQT_DCT,STD_DC_Y_HT,STD_AC_Y_HT,&yDC);     
    }
    else
    {
     break;
    }
   }  
   //------------------------------------------------------------------  
   for (; m < uBufLen; m += DCTBLOCKSIZE)
   {
    for (n = 0; n < DCTBLOCKSIZE; n++)
    {
     dctUBuf[n] = FLOAT(lpUBuf[m + n] - 128);
    }    
    if (uCounter > 0)
    {    
     --uCounter;
     ProcessDU(dctUBuf,UVQT_DCT,STD_DC_UV_HT,STD_AC_UV_HT,&uDC);         
    }
    else
    {
     break;
    }
   }  
   //-------------------------------------------------------------------  
   for (; s < vBufLen; s += DCTBLOCKSIZE)
   {
    for (k = 0; k < DCTBLOCKSIZE; k++)
    {
     dctVBuf[k] = FLOAT(lpVBuf[s + k] - 128);
    }
    if (vCounter > 0)
    {
     --vCounter;
     ProcessDU(dctVBuf,UVQT_DCT,STD_DC_UV_HT,STD_AC_UV_HT,&vDC);        
    }
    else
    {
     break;
    }
   }  
  } 
 }

 // 8x8�ĸ�����ɢ���ұ任
 void JEnc::FDCT(FLOAT* lpBuff)
 {
  FLOAT tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  FLOAT tmp10, tmp11, tmp12, tmp13;
  FLOAT z1, z2, z3, z4, z5, z11, z13;
  FLOAT* dataptr;
  int ctr;

  /* ��һ���֣����н��м��� */
  dataptr = lpBuff;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--)
  {
   tmp0 = dataptr[0] + dataptr[7];
   tmp7 = dataptr[0] - dataptr[7];
   tmp1 = dataptr[1] + dataptr[6];
   tmp6 = dataptr[1] - dataptr[6];
   tmp2 = dataptr[2] + dataptr[5];
   tmp5 = dataptr[2] - dataptr[5];
   tmp3 = dataptr[3] + dataptr[4];
   tmp4 = dataptr[3] - dataptr[4];

   /* ��ż����������� */    
   tmp10 = tmp0 + tmp3; /* phase 2 */
   tmp13 = tmp0 - tmp3;
   tmp11 = tmp1 + tmp2;
   tmp12 = tmp1 - tmp2;

   dataptr[0] = tmp10 + tmp11; /* phase 3 */
   dataptr[4] = tmp10 - tmp11;

   z1 = (tmp12 + tmp13) * (0.707106781); /* c4 */
   dataptr[2] = tmp13 + z1; /* phase 5 */
   dataptr[6] = tmp13 - z1;

   /* ����������м��� */
   tmp10 = tmp4 + tmp5; /* phase 2 */
   tmp11 = tmp5 + tmp6;
   tmp12 = tmp6 + tmp7;

   z5 = (tmp10 - tmp12) * ( 0.382683433); /* c6 */
   z2 = (0.541196100) * tmp10 + z5; /* c2-c6 */
   z4 = (1.306562965) * tmp12 + z5; /* c2+c6 */
   z3 = tmp11 * (0.707106781); /* c4 */

   z11 = tmp7 + z3;  /* phase 5 */
   z13 = tmp7 - z3;

   dataptr[5] = z13 + z2; /* phase 6 */
   dataptr[3] = z13 - z2;
   dataptr[1] = z11 + z4;
   dataptr[7] = z11 - z4;

   dataptr += DCTSIZE; /* ��ָ��ָ����һ�� */
  }

  /* �ڶ����֣����н��м��� */
  dataptr = lpBuff;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--)
  {
   tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
   tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
   tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
   tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
   tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
   tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
   tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
   tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];

   /* ��ż����������� */    
   tmp10 = tmp0 + tmp3; /* phase 2 */
   tmp13 = tmp0 - tmp3;
   tmp11 = tmp1 + tmp2;
   tmp12 = tmp1 - tmp2;

   dataptr[DCTSIZE*0] = tmp10 + tmp11; /* phase 3 */
   dataptr[DCTSIZE*4] = tmp10 - tmp11;

   z1 = (tmp12 + tmp13) * (0.707106781); /* c4 */
   dataptr[DCTSIZE*2] = tmp13 + z1; /* phase 5 */
   dataptr[DCTSIZE*6] = tmp13 - z1;

   /* ����������м��� */
   tmp10 = tmp4 + tmp5; /* phase 2 */
   tmp11 = tmp5 + tmp6;
   tmp12 = tmp6 + tmp7;

   z5 = (tmp10 - tmp12) * (0.382683433); /* c6 */
   z2 = (0.541196100) * tmp10 + z5; /* c2-c6 */
   z4 = (1.306562965) * tmp12 + z5; /* c2+c6 */
   z3 = tmp11 * (0.707106781); /* c4 */

   z11 = tmp7 + z3;  /* phase 5 */
   z13 = tmp7 - z3;

   dataptr[DCTSIZE*5] = z13 + z2; /* phase 6 */
   dataptr[DCTSIZE*3] = z13 - z2;
   dataptr[DCTSIZE*1] = z11 + z4;
   dataptr[DCTSIZE*7] = z11 - z4;

   ++dataptr;   /* ��ָ��ָ����һ�� */
  }
 }

 //********************************************************************
 // ��������:WriteBits 
 //
 // ����˵��:д���������
 //
 // ����˵��:
 // value:AC/DC�źŵ����
 //********************************************************************
 void JEnc::WriteBits(HUFFCODE huffCode)
 {  
  WriteBitsStream(huffCode.code,huffCode.length); 
 }
 void JEnc::WriteBits(SYM2 sym)
 {
  WriteBitsStream(sym.amplitude,sym.codeLen); 
 }

 //********************************************************************
 // ��������:WriteBitsStream 
 //
 // ����˵��:д���������
 //
 // ����˵��:
 // value:��Ҫд���ֵ
 // codeLen:�����Ƴ���
 //********************************************************************
 void JEnc::WriteBitsStream(USHORT value,BYTE codeLen)
 { 
  CHAR posval;//bit position in the bitstring we read, should be<=15 and >=0 
  posval=codeLen-1;
  while (posval>=0)
  {
   if (value & mask[posval])
   {
    bytenew|=mask[bytepos];
   }
   posval--;bytepos--;
   if (bytepos<0) 
   { 
    if (bytenew==0xFF)
    {
     WriteByte(0xFF);
     WriteByte(0);
    }
    else
    {
     WriteByte(bytenew);
    }
    bytepos=7;bytenew=0;
   }
  }
 }

 //********************************************************************
 // ��������:RLEComp 
 //
 // ����˵��:ʹ��RLE�㷨��ACѹ��,������������1,0,0,0,3,0,5 
 //     ���Ϊ(0,1)(3,3)(1,5),��λ��ʾ��λ����ǰ0�ĸ���
 //          ��λ��4bits��ʾ,0�ĸ���������ʾ��Χ�����Ϊ(15,0)
 //          �����0��������һ�������б�ʾ.
 //
 // ����˵��:
 // lpbuf:���뻺��,8x8�任�źŻ���
 // lpOutBuf:�������,�ṹ����,�ṹ��Ϣ��ͷ�ļ�
 // resultLen:������峤��,���������ŵ�����
 //********************************************************************
 void JEnc::RLEComp(SHORT* lpbuf,ACSYM* lpOutBuf,BYTE &resultLen)
 {  
  BYTE zeroNum     = 0;       //0�г̼�����
  UINT EOBPos      = 0;       //EOB����λ�� 
  const BYTE MAXZEROLEN = 15;          //���0�г�
  UINT i        = 0;      //��ʱ����
  UINT j        = 0;

  EOBPos = DCTBLOCKSIZE - 1;          //������ʼλ�ã������һ���źſ�ʼ
  for (i = EOBPos; i > 0; i--)         //������AC�ź���0�ĸ���
  {
   if (lpbuf[i] == 0)           //�ж������Ƿ�Ϊ0
   {
    --EOBPos;            //��ǰһλ
   }
   else              //������0������
   {
    break;                   
   }
  }

  for (i = 1; i <= EOBPos; i++)         //�ӵڶ����źţ���AC�źſ�ʼ����
  {
   if (lpbuf[i] == 0 && zeroNum < MAXZEROLEN)     //����ź�Ϊ0����������С��15
   {
    ++zeroNum;   
   }
   else
   {   
    lpOutBuf[j].zeroLen = zeroNum;       //0�г̣��������ȣ�
    lpOutBuf[j].codeLen = ComputeVLI(lpbuf[i]);    //���ȱ��볤��
    lpOutBuf[j].amplitude = lpbuf[i];      //���      
    zeroNum = 0;           //0��������λ
    ++resultLen;           //��������++
    ++j;             //���ż���
   }
  } 
 }

 //********************************************************************
 // ��������:BuildSym2 
 //
 // ����˵��:���źŵ����VLI����,���ر��볤�Ⱥ��ź�����ķ���
 //
 // ����˵��:
 // value:AC/DC�źŵ����
 //********************************************************************
 SYM2 JEnc::BuildSym2(SHORT value)
 {
  SYM2 Symbol;  

  Symbol.codeLen = ComputeVLI(value);              //��ȡ���볤��
  Symbol.amplitude = 0;
  if (value >= 0)
  {
   Symbol.amplitude = value;
  }
  else
  {
   Symbol.amplitude = SHORT(pow(2,Symbol.codeLen)-1) + value;  //���㷴��
  }

  return Symbol;
 }


 //���ط��ŵĳ���
 BYTE JEnc::ComputeVLI(SHORT val)
 { 
  BYTE binStrLen = 0;
  val = abs(val); 
  //��ȡ�������볤��   
  if(val == 1)
  {
   binStrLen = 1;  
  }
  else if(val >= 2 && val <= 3)
  {
   binStrLen = 2;
  }
  else if(val >= 4 && val <= 7)
  {
   binStrLen = 3;
  }
  else if(val >= 8 && val <= 15)
  {
   binStrLen = 4;
  }
  else if(val >= 16 && val <= 31)
  {
   binStrLen = 5;
  }
  else if(val >= 32 && val <= 63)
  {
   binStrLen = 6;
  }
  else if(val >= 64 && val <= 127)
  {
   binStrLen = 7;
  }
  else if(val >= 128 && val <= 255)
  {
   binStrLen = 8;
  }
  else if(val >= 256 && val <= 511)
  {
   binStrLen = 9;
  }
  else if(val >= 512 && val <= 1023)
  {
   binStrLen = 10;
  }
  else if(val >= 1024 && val <= 2047)
  {
   binStrLen = 11;
  }

  return binStrLen;
 }

 //********************************************************************
 // ��������:BuildVLITable 
 //
 // ����˵��:����VLI��
 //
 // ����˵��:
 //********************************************************************
 void JEnc::BuildVLITable(void)
 {
  int i   = 0;

  for (i = 0; i < DC_MAX_QUANTED; ++i)
  {
   pVLITAB[i] = ComputeVLI(i);
  }

  for (i = DC_MIN_QUANTED; i < 0; ++i)
  {
   pVLITAB[i] = ComputeVLI(i);
  }
 }
