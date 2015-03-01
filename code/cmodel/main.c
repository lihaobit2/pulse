#include <stdio.h>
#include<string.h>
#include "Pulse_drv.h"

#define MAX_IN_DATA_NUM 16384*8

int16 g_in[MAX_IN_DATA_NUM] = {0};
FILE *g_FileRes, *g_FileRes2, *g_FileRes3;
int16 g_adc;
#define INTER_RATIO 4
int main()
{
    uint32 i, j;
    FILE *fp;
    int16 tmp1,tmp2;
    uint32 ulDataNum;

    (void)memset(g_in, 0, sizeof(g_in));
	fp = fopen("data.txt","r");
	if(fp == NULL)
	{
		printf("File open error!");
		
		return 1;
	}



	i = 0;
	ulDataNum = 0;
	while(!feof(fp))
	{
	    
		fscanf(fp, "%d %d %d",&g_in[i],&tmp1, &tmp2);
		for(j = 1; j < INTER_RATIO; j++)
		{
			g_in[i+j] = g_in[i];
		}
		
		i += INTER_RATIO;
		
		if(i > MAX_IN_DATA_NUM - INTER_RATIO)
		{
		    printf("data size large than buf!\n");
			break;
		}
	}
    ulDataNum = i;
	fclose(fp);
	
    g_FileRes = fopen("result.txt","w");
	if(feof(g_FileRes))
	{
		return 0;
	}	 
	g_FileRes2 = fopen("time.txt","w");
	if(feof(g_FileRes2))
	{
		return 0;
	}
	g_FileRes3 = fopen("pulse_cnt.txt","w");
	if(feof(g_FileRes3))
	{
		return 0;
	}	
	pulse_endRead();
    pulse_startRead();
	for(i = 0;i < ulDataNum; i ++)
	{
    	g_adc = g_in[i];
   	
		pulse_update();
	}
	
	fclose(g_FileRes);
	fclose(g_FileRes2);
	fclose(g_FileRes3);

	return 0;
}

