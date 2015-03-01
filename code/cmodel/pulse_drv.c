#include <stdio.h>
#include "pulse_drv.h"

//#define PLUSE_ADC_CHN         HAL_ADC_CHANNEL_6
static int16 g_vpp = 0;
static int16 g_dc = 0;
int16 g_prec = 0;
static int16 g_adMax = 0;
static int16 g_adMin = 4095;

static uint8 g_badFlag = FALSE;
int16 g_pluseCnt = 0;
uint16 g_PulseTime = 0;
static uint16 g_InvalCnt=0;		//��¼ʱ������

static uint16 g_pulseState;
static int16 g_highTh,g_lowTh;
static int16 g_preState, g_currState, g_oldData, g_newData;
uint8 g_pulseStartFlag = FALSE;
uint16 g_pulseValue = 0;
static uint16 g_pulseMean = 0;

static uint8 g_validDataCnt = 0;
uint16 g_pulseReportNum = 0;
uint8 g_firstMeasureFlag = TRUE;
static uint16 g_reportLen;
int16 g_findFlagTmp;

static uint16 g_sampling = 0;

uint16 g_timeRec[REPORT_LEN];
uint16 g_IntvRec[REPORT_LEN];

extern FILE *g_FileRes, *g_FileRes2,*g_FileRes3;

//����������
void pulse_startRead(void)
{
   g_pulseStartFlag = TRUE;
   //osal_start_timerEx( simpleBLETaskId, BLE_PERIOD_PULSE_EVT, BLE_PLUSE_TIMER_5MS_CNT );

   return;

}

//�ϱ�������
void pluse_report(void)
{
  static int32 reportFilter = (int32)75<<16;
  
  if((g_pulseValue < MIN_PULSE_REPORT_VAL) || (g_pulseValue > MAX_PULSE_REPORT_VAL))
  {
	   g_pulseReportNum = 0;
  }
  else
  {
	
	if(g_firstMeasureFlag)
	{
	    g_firstMeasureFlag = FALSE;
	    g_pulseReportNum = g_pulseValue;
	    reportFilter = (int32)g_pulseReportNum<<16; //�˲�����ֵ
	}
	else
	{
	  
	 //����ǰһ���˲�
	  reportFilter = FILTER(g_pulseValue, reportFilter, 1);
	  g_pulseReportNum = LIMIT((reportFilter>>16), MIN_PULSE_REPORT_VAL, MAX_PULSE_REPORT_VAL);
	}
	
	//if(!LICENSE_IS_GOOD())
	{
	  //g_pulseReportNum = 1;
	}
  }

  //uart_send_pulse(g_pulseReportNum);
}




//�����ϱ�
void pulse_endRead(void)
{
    //led off
    //HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF); 
    g_pulseStartFlag = FALSE;
    g_vpp = 0;
    g_dc = 0;
    g_prec = 0;
    g_adMax = 0;
    g_adMin = 4095;
    g_highTh = 0;
    g_lowTh = 0;
    
    g_badFlag = FALSE;
    g_pluseCnt = 0;
    g_PulseTime = 0;
    g_InvalCnt=0;	   //��¼ʱ������
    
    g_pulseState = PULSE_BEGIN;
   
    g_preState = PULSE_LOW;
    g_currState = PULSE_LOW;
    g_oldData = 0;
    g_newData = 0;
    g_pulseValue = 0;
    g_validDataCnt = 0;
    g_pulseReportNum = 0;
    g_firstMeasureFlag = TRUE;
    
    g_sampling = 0;
    
    memset(&g_timeRec[0], 0, sizeof(g_timeRec));
    memset(&g_IntvRec[0], 0, sizeof(g_IntvRec));

    return;
}



void pulse_UpdatePara(int16 adc)
{
    //����������Сֵ
    g_adMax = MAX(adc, g_adMax);
    g_adMin = MIN(adc, g_adMin);
	g_reportLen = (TRUE == g_firstMeasureFlag) ? REPORT_LEN : (REPORT_LEN2);

    g_sampling += 1;
    if(g_sampling > PULSE_PARA_UPDATE_PERIOD)
    {
        g_sampling = 0;
		g_vpp = g_adMax - g_adMin;
		g_dc = (g_adMax + g_adMin)/2;
		g_highTh = g_dc + (((g_adMax - g_dc) *1) >> 4);
		g_lowTh = g_dc - ((g_dc - g_adMin) >> 1);		 

		g_badFlag = FALSE;	
		
		//vpp��?
	    if (g_vpp >= MIN_PULSE_VPP)
		{
			g_prec = (g_vpp*11)>>4;
		} 
		else 
		{ 
		  	g_prec = 4095;
		  	g_badFlag = TRUE;
		}

        /*���ĺܽ�*/ 
		if(g_adMin > 350)
		{
		    g_badFlag = TRUE;
		}

		g_adMax = -4096;
		g_adMin = 4095;
    }

	return;
}
void pulse_CntJudge(uint16 adAvg)
{
   uint8 findFlagTmp;
   uint16 index, indexPre;
   
   if(adAvg > g_highTh)
   {
	   g_preState = g_currState;
	   g_currState = PULSE_HIGH;
	   /*��һ��״̬�л�������һ��״̬����Ĵ���ֵ��λ*/ 
	   if(g_preState != g_currState)
	   {
		   g_oldData= g_newData;
	   }
	   
	   /*��¼�����ֵ*/
	   if(adAvg > g_newData)
	   {
		   g_newData = adAvg;
	   }
   }
   else if(adAvg < g_lowTh)
   {
	   g_preState = g_currState;
	   g_currState = PULSE_LOW;

	   /*��һ��״̬�л�������һ��״̬����Ĵ���ֵ��λ*/ 
	   if(g_preState!= g_currState)
	   {
		   g_oldData= g_newData;
	   }
	   
	   /*��¼�����ֵ*/
	   if(adAvg < g_newData)
	   {
		   g_newData = adAvg;
	   }
	}
			 
	findFlagTmp= FALSE;
	if((g_oldData - g_newData)> g_prec)
	{
		findFlagTmp = TRUE;
		g_oldData =  g_newData;
	} 


   if(( findFlagTmp == TRUE)&&(g_badFlag == FALSE))
   {
		//findFlag = TimeWindow();
		
		index = g_pluseCnt % g_reportLen;
		indexPre = (index + g_reportLen - 1) % g_reportLen;

		g_timeRec[index] = g_PulseTime;
		g_IntvRec[index] = g_timeRec[index] - g_timeRec[indexPre];
		g_InvalCnt = 0;
		
		g_pluseCnt ++;

   }
 
   else 
   {
	   g_InvalCnt ++;  
   }

   g_findFlagTmp = findFlagTmp;

   return;
}

void pulse_stateTran()
{
	switch(g_pulseState)
	{
        /*��ʼ�������*/ 
	    case PULSE_BEGIN:
	  	    if(g_pluseCnt > MIN_PULSE_FIND_CNT)
	        {
				g_pulseState = PULSE_RUN;
	    	}

	  		break;

        /*������������*/ 
	  	case PULSE_RUN:
		  	if(g_InvalCnt >= MAX_PULSE_BREAK_INTERVAL1)
		  	{
				g_pulseState = PULSE_PAUSE;
			  }

		  	break;
	  	/*��ͣ��������*/
		case PULSE_PAUSE:
	  
			if(0 == g_InvalCnt)
			{
				g_pulseState = PULSE_RUN;
			}
	  		break;
	  	default:
	  	    break;
	}

	return;
}

void pulse_stateProc()
{
	uint8 i;
	uint16 pulseValTmp;
	
    switch(g_pulseState)
	{
		 case PULSE_BEGIN:

			 break;
		 case PULSE_RUN:
			 //�������ɴμ�⵽���������ϱ����
			
			 if(g_reportLen == g_pluseCnt)
			 {
 				 /*60��ʾ60s,100��1/10ms�� */
				 g_validDataCnt = 0;
				 pulseValTmp = 0;
				 
				 for(i = 0; i < g_reportLen; i ++)
				 {
				    /*�������ε�ʱ�����������ж�*/
				 	if((g_IntvRec[i] < MAX_PULSE_INTV)&&(g_IntvRec[i] > MIN_PULSE_INTV))
				 	{
				 	    pulseValTmp += g_IntvRec[i];
				 	    g_validDataCnt ++;
				 	}
				 }
                 g_pulseMean = pulseValTmp/g_validDataCnt;
                 
				 for(i = 0; i < g_reportLen; i ++)
				 {
				    /*�޳����ֵ̫Զ�ĵ�*/
				    
				 	if((g_IntvRec[i] < MAX_PULSE_INTV)&&(g_IntvRec[i] > MIN_PULSE_INTV))
					{
					 	if((g_IntvRec[i] < ((g_pulseMean >> 13) >> 4))||(g_IntvRec[i] > ((g_pulseMean * 19) >> 4)))
					 	{
					 	    pulseValTmp -= g_IntvRec[i];
					 	    g_validDataCnt --;
					 	}
				 	}
				 }
				 
				 if((g_validDataCnt != 0)&&(g_validDataCnt >= ((g_reportLen * 1>> 2))))
				 {
				 
				    g_pulseValue = (60 * (1000/INTR_INTERVAL) * g_validDataCnt/pulseValTmp);
  					pluse_report();
    				 g_InvalCnt = 0;

				 }
				 else
				 {
				 	//g_pulseValue = 0;
				 }
	
				 // ��λ������
				 g_pluseCnt = 0;

	
				 {				
					 int16 i;
					 fprintf(g_FileRes2,"time:%5d, pulse value:%4d, mean:%3d,valid cnt:%d,g_reportLen:%d",g_PulseTime, g_pulseValue,g_pulseMean, g_validDataCnt,g_reportLen);
					 for(i = 0; i < REPORT_LEN;i ++)
					 {
						 fprintf(g_FileRes2,"%5d,",g_timeRec[i]);
					 }
					 fprintf(g_FileRes2," ");	
					 fprintf(g_FileRes2,"pulse intv:");
					 for(i = 0; i < REPORT_LEN;i ++)
					 {
						 fprintf(g_FileRes2,"%5d,",g_IntvRec[i]);
					 }
					 
					 fprintf(g_FileRes2,"\n");	 
				  } 	   
			 }
			 break;
		 case PULSE_PAUSE:
		     
		    if(g_InvalCnt >= MAX_PULSE_BREAK_INTERVAL2)
		 	{
			   //�������һ��ʱ������Ȼû���ҵ���һ�������������¿�ʼ����
      		    if(g_PulseTime & (REPORT_PERIOD - 1))
			     {
			         g_pulseValue = 0;
			         pluse_report();
			     }	
			     
			    g_pluseCnt = 0;
			    g_firstMeasureFlag = TRUE;

                //��ֹ���
			    g_InvalCnt = MAX(20000,g_InvalCnt);

			} 		     
			 break;
		 default:
			 break;
	 }

	return;
}
//���������㷨
void pulse_update(void)
{
    static int16 loops = 0;
    static int32 adFilter = 0;
    static int16 adc, adcPre;
    static int16 adAvg;
    
    g_PulseTime ++; 
    //����Ƿ�����
    if( g_pulseStartFlag != TRUE ) 
    {
        loops = 0;
        g_sampling = 0;
        return;
    }
    
    //С��1.2�ڵȴ�����
    loops ++;
    if(loops <= 120) //12s
    {
        //led flash
        if(loops == 1) 
        {
            //HalSetFlashPeriod(200);
            //HalLedSet( HAL_LED_1, HAL_LED_MODE_FLASH); 
        } 
        
        if(loops == 120)
        {
            //��������������Ҫ��led����
            //HalLedSet( HAL_LED_1, HAL_LED_MODE_ON); 
            
        } 

        return;
    }
    else
    {
        loops = 120;
    }
    
    //����
    //adc = HalAdcRead (PLUSE_ADC_CHN, HAL_ADC_RESOLUTION_12);
    adcPre = adc;
    adc = g_adc;

        
    adFilter = FILTER(adc, adFilter, 2);
    adAvg = LIMIT((adFilter>>16), 0, 4095);
    //adAvg = adc;
    
    pulse_UpdatePara(adAvg);
     

    /*�쳣�����ж�*/
    if(ABS(adc - adcPre) > MAX_JUMP_TH)
    {
    	 g_badFlag = TRUE;
    }
    /////////////////////

    pulse_CntJudge(adAvg);
    
    pulse_stateTran();
    pulse_stateProc();
 
    fprintf(g_FileRes,"Time:%5d,g_pluseCnt:%3d,g_pulseValue:%3d,g_pulseReportNum:%3d,g_validDataCnt:%2d,\\
			  findFlagTmp:%2d,g_badFlag:%2d,g_InvalCnt:%d,g_pulseState:%d,\
			  g_oldData:%5d, g_newData:%5d,adc:%5d,adAvg:%5d,g_vpp:%5d,g_dc:%5d,g_prec:%5d,g_adMax:%5d,g_adMin:%5d,\\
			  g_highTh:%5d,g_lowTh:%5d,\\
              g_preState:%2d,g_currState:%2d,,loop:%d,g_reportLen:%2d\n",
              g_PulseTime, g_pluseCnt, g_pulseValue,g_pulseReportNum,g_validDataCnt,
			  g_findFlagTmp,g_badFlag,g_InvalCnt, g_pulseState,
			   g_oldData, g_newData,adc,adAvg,g_vpp,g_dc, g_prec,g_adMax,g_adMin,
			  g_highTh, g_lowTh,
             g_preState,g_currState,loops,g_reportLen);
     
    fprintf(g_FileRes3,"Time:%4d,g_pluseCnt:%4d,prec:%4d\n",g_PulseTime, g_pluseCnt, g_prec);
}
