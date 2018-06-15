#include <stdio.h>
#include <iostream>
#include "iostream"
#include "Windows.h"
#include "PlayM4.h"
using namespace std;
LONG  g_nPort  = -1;
#define READ_BUF_SIZE 200
#define PRIVT_HEAD_LEN 40


void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,
					   FRAME_INFO * pFrameInfo, 
					   long nReserved1,long /*nReserved2*/)
{
	
     //---------------------------------------
     // 获取解码后音频数据
     if (pFrameInfo->nType == T_AUDIO16)
     {
       printf("test:: get audio data !\n");
      }

     //---------------------------------------
     // 获取解码后视频数据
      else if ( pFrameInfo->nType == T_YV12 ) 
     {   
        printf("test:: get video data !\n");
      }
}


using namespace std;

typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindow;

int main() 
{
  BOOL bFlag    = FALSE;
  int  nError   = 0;
  FILE* fp      =  NULL;
  char* pBuffer = NULL;

 //---------------------------------------
 // 获取控制台窗口句柄
 HMODULE hKernel32 = GetModuleHandle("kernel32");
 GetConsoleWindow  = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");


  //---------------------------------------
  // 获取播放库通道号
  PlayM4_GetPort(&g_nPort);

  //打开待解码文件
  fp = fopen( "test.mp4", ("rb") );
  if (fp == NULL)
  {
     printf("cannot open the file !\n");
     return;
  }


  pBuffer = new char[READ_BUF_SIZE];
  if (pBuffer == NULL)
  {
    return;
  }

  //读取文件中文件头
  fread( pBuffer, PRIVT_HEAD_LEN, 1, fp );

  //设置流模式类型 
  PlayM4_SetStreamOpenMode(g_nPort,STREAME_FILE); 
  //打开流模式
  PlayM4_OpenStream(g_nPort,(PBYTE)pBuffer,PRIVT_HEAD_LEN,1024 * 1024);

  //设置解码回调
  PlayM4_SetDecCallBackExMend(g_nPort,DecCBFun,NULL,0,NULL);

  //---------------------------------------
  // 获取显示窗口句柄
  HWND hWnd = GetConsoleWindow();     
  PlayM4_Play(g_nPort,hWnd);

  while (!feof(fp))
  {		
     fread( pBuffer, READ_BUF_SIZE, 1, fp );
	  
     while (1)
     {
       bFlag = PlayM4_InputData(g_nPort,(PBYTE)pBuffer,READ_BUF_SIZE);
       if (bFlag == FALSE)
       {
           nError = PlayM4_GetLastError(g_nPort);
			  
           //若缓冲区满，则重复送入数据
           if (nError == PLAYM4_BUF_OVER)
          {
	      Sleep(2);
	      continue;
           }
        }
		  
        //若送入成功，则继续读取数据送入到播放库缓冲
        break; 
      }
  }


  Sleep(10000);
  //---------------------------------------
  // 停止解码
   PlayM4_Stop(g_nPort);
   
   //关闭流，回收源数据缓冲
   PlayM4_CloseStream(g_nPort);

   //释放播放库端口号
   PlayM4_FreePort(g_nPort);

   if (fp != NULL)
   {
        fclose(fp);
        fp = NULL;
   }


  if (pBuffer != NULL)
  {
    delete [] pBuffer;
    pBuffer = NULL;
  }
  return 0;
}

