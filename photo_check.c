#include "stdio.h"
#include <direct.h>
#include <stdlib.h>
#include <io.h>

using namespace std;
void listFiles(char * dir);
static int currentLay = 0;
FILE *fp=NULL;

int main(int argc,char **argv)
{
	char buf[80];
//	getcwd(buf,sizeof(buf));
	scanf("%s",buf);
	if(buf == NULL || strcmp(buf,"") == 0)
	{
		printf("get failed");
	}
	else
	{
		printf("buf == %s\n",buf);
	}
//	strcat(buf,"\\*.*");
	
	fp = fopen("D:\\test\\at.txt","wt+");
	if(fp == NULL)
	{
		printf("fp =========== %d\n",fp);
		return 0;
	}	
	listFiles(buf);
	fclose(fp);
	return 0;
}
void listFiles(char * dir)
{
	intptr_t handle;//intptr_t 指针类型
	_finddata_t findData;//记录查找到的文件的信息
	strcat(dir,"\\*.*");
//	printf("dir === %s\n",dir);
	handle = _findfirst(dir, &findData);//_findfirst,_findnext,_findclose三件套
	char *pnewchar = NULL;
	currentLay++;
	if(handle == -1)
	{		
		printf("fail to find first file\n");
		return ;
	}
	do
	{
	//	if (findData.attrib & _A_SUBDIR && (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0))
	//		printf("subdir %s \t<dir>\n",findData.name);
	//	else			
	//		printf("filename = %s  %d\t\n",findData.name,findData.size);
		pnewchar = (char *)malloc(strlen(dir) + sizeof(findData.name)+1);
		memset(pnewchar,'\0',sizeof(pnewchar));
	//	printf("before cut dir ========== %s\n",dir);
		char *pos = strrchr(dir,'\\');
	//	printf("pos ======= %s\n",pos);
	//	printf("len ======== %d \n",strlen(dir)-strlen(pos)+1);
		strncpy(pnewchar,dir,strlen(dir)-strlen(pos)+1);
		pnewchar[strlen(dir)-strlen(pos)+1] = '\0';
		//int pos = strrchr(dir,"\\");
		//printf("pos ============= %d\n",pos);
	//	strcat(pnewchar,dir);
	//	strcat(pnewchar,"\\");
	//	printf("pnewchar ====================== %s\n",pnewchar);
		strcat(pnewchar,findData.name);
	//	printf("pnewchar ====== &&&&&&&&& %s\n",pnewchar);
		
	//	printf("attrib ==== == %d _A_NORMAL = %d   \n",findData.attrib,_A_NORMAL);
		
		if(findData.attrib & _A_ARCH)
		{
			fprintf(fp,"%s ; %d\t\n",pnewchar,currentLay);
		//	fwrite(pnewchar,strlen(pnewchar),1,fp);
		}
		else
		{
			if(findData.attrib & _A_SUBDIR && (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0))
			{	
			//	printf("pnewchar ==&&&&&&& %s\n",pnewchar);
				listFiles(pnewchar);
			}
		}
		delete pnewchar;
	}while(_findnext(handle, &findData) == 0);
	
	printf("Done!\n");
	_findclose(handle);
}