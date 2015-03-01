#ifndef _BT4_PULSE_H_
#define _BT4_PULSE_H_

//��������
#define INTR_INTERVAL 10 /*10 ms*/

#define MIN_PULSE_REPORT_VAL 50
#define MAX_PULSE_REPORT_VAL 120
#define MAX_PULSE_INTV (1200/INTR_INTERVAL) /*1400 ms*/
#define MIN_PULSE_INTV (500/INTR_INTERVAL) /*400ms*/

#define REPORT_LEN 8
#define REPORT_LEN2 4


#define PULSE_PARA_UPDATE_PERIOD (1000/INTR_INTERVAL)  
#define MAX_PULSE_BREAK_INTERVAL1 (6000/INTR_INTERVAL)  
#define MAX_PULSE_BREAK_INTERVAL2 (8000/INTR_INTERVAL)  
#define MIN_PULSE_VPP 250

#define MIN_PULSE_FIND_CNT 2
#define MAX_JUMP_TH 1000

#define REPORT_PERIOD (5120/INTR_INTERVAL)
#define PULSE_HIGH 1
#define PULSE_LOW 0

#define PULSE_BEGIN 0
#define PULSE_RUN 1
#define PULSE_PAUSE 2

//yn = a*xn + (1-a)*yn_1 = yn_1 + a*(xn-yn-1)
#define FILTER(adc, Yn_1, deep)   (Yn_1 + ((((int32)(adc)<<16)-(Yn_1))>>(deep)))
#define LIMIT(x, low, high)       MAX((low), MIN((x), (high)))

typedef signed   char   int8;
typedef unsigned char   uint8;

typedef signed   short  int16;
typedef unsigned short  uint16;

typedef signed   long   int32;
typedef unsigned long   uint32;

typedef unsigned char   bool;

typedef uint8           halDataAlign_t;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef BV
#define BV(n)      (1 << (n))
#endif

#ifndef BF
#define BF(x,b,s)  (((x) & (b)) >> (s))
#endif

#ifndef MIN
#define MIN(n,m)   (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
#define MAX(n,m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef ABS
#define ABS(n)     (((n) < 0) ? -(n) : (n))
#endif

#define LED_FLASH_TIME 1200
#define LED_ON_TIME 2000

#define CNT_TIME 15000


extern uint16 g_adc;
extern void pulse_update(void);
extern void pulse_startRead(void);
extern void pulse_endRead(void);


extern uint8 g_pulseStartFlag;

#endif
