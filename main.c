/* ###################################################################
**     Filename    : main.c
**     Project     : ISPU24_ZAS_PCBV1.0
**     Processor   : MKL02Z32VFM4
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2021-03-05, 14:14, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */

/*
 * ###################################################################################################################
 * main.c
 *
 *  Created on: 5 mar 2021
 *      Author: Piotr.Dzierzak
 *
 * Najmniejszy zegar:
 * Core clock:	1,3 MHz
 * Bus clock:	0.16384 MHz
 *
 * Switch - wybor rodzaju wyjscia: 	ON-czestotliwosc (2-42Hz), przeciazenie 1Hz w przypadku kiedy Uwy<21,5V.
 * 									OFF-wyjscie przekaznikowe: zwarte jak OK, rozwarte jak Uwy<21,5V
 * 									histereza wyjsca=21500mV+20mV OK
 * 									histereza wyjsca=21500mV-20mV NOK
 * WDT=1000ms.
 *
 * ADC 12 bit@ 100 pomiarow trwa 240 ms.
 * Czyli stan LED jest odswiezany co 240ms ==ok 4Hz.
 *

 *###################################################################################################################
*/

/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "WAIT1.h"
#include "FRQ.h"
#include "LED1.h"
#include "SwitchS2.h"
#include "AD1_MIV.h"
#include "AdcLdd1.h"
#include "PPG1.h"
#include "WDog1.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* User includes (#include below this line is not maintained by Processor Expert) */
#include <math.h>	//do zaokroglania

#include <string.h>
#include <stdio.h>
//#include "SEGGER_RTT.h"

word 			ResultsADC[6];
unsigned char 	Stan[2];



#define UREF				(unsigned long int)3300			//w mV
#define RESOLUTION			(unsigned long int)4096			//bo 2^12.
float KWANT=((float)UREF)/((float)RESOLUTION);//				745      //74,5 x10 [nV] bierze sie to stad (unsigned long int)(UREF*10000000)/RESOLUTION

typedef struct _zas
{
	float 	napiecie;
	float 	prad;
//	uint16	czestotliwosc;
	float	czestotliwosc;
	uint16	okres_ms;
	uint16	okres_us;
} tZas;

tZas Zasilacz;
LDD_TDeviceData *MyPPG1Ptr;
LDD_TDeviceData *bit1Ptr;

enum st
{
	eCZESTOTLIWOSC,
	ePRZEKAZNIK,
	eSWITCH_ON,
	eSWITCH_OFF
};
enum st SwitchUstawiony;
enum st SwitchBiezacy;


#define WYJSCIE_FRQ_OTWARTE		FRQ_ClrVal(NULL)
#define WYJSCIE_FRQ_ZAMKNIETE	FRQ_SetVal(NULL)
#define SWITCH_ON			0
#define SWITCH_OFF			1
#define HISTEREZA			20		//100mV
#define ILOSC_PROBEK_ADC	100

int i;
unsigned long int NapSuma,PradSuma;
unsigned long int NapSrednia,PradSrednia;

uint16 ms;
float ms_float;
unsigned char str[100],str2[100];
unsigned int LicznikSwitchZmiana=0;

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
   // WAIT1_Waitms(1);		//100
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();							//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

 // PPG1_Init(NULL);
  WAIT1_Waitms(200);
  LicznikSwitchZmiana=0;

   if(SWITCH_ON==SwitchS2_GetVal(NULL))			//czyli stan ==0, czyli czestotliwosc
  {
//	  PE_low_level_init_PPG1();					//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  MyPPG1Ptr = PPG1_Init(NULL);
	  PPG1_Enable(MyPPG1Ptr);
	  PPG1_ConnectPin(MyPPG1Ptr,PPG1_OUT_PIN);	//!!!METODA DOLACZAJACA PIN DO OBSLUGI PPG1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  	  	  	  	  	  	  	  	  	  	  	///PPG1_OUT_PIN - zdefiniowany jako  Output pin mask w pliku PPG1.h
	  	  	  	  	  	  	  	  	  	  	  	//PTB6 - sharing pin !!!
      PPG1_SetRatio16(MyPPG1Ptr,32767);   //change duty 65535/2=32767 czyli 50% WYPELNIENIA!!!
      SwitchBiezacy=SwitchUstawiony=eSWITCH_ON;
  }
  else
  {
//	  PE_low_level_init_BitIO();	  			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	  //bit1Ptr =FRQ_Init( (LDD_TUserData *)NULL);
	  SwitchBiezacy=SwitchUstawiony=eSWITCH_OFF;
  }

/*

   SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
   SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n\r\n");
   SEGGER_RTT_WriteString(0, "###### Testing SEGGER_printf() ######\r\n");
//   SEGGER_RTT_Init();
*/

  for(;;)
  {
	  //OBIEG PETLI NA TAK NISKIM ZEGARZE TRWA 250ms (przy 100 pomiarach ADC).
	  //WDT jest ustawiony na 1s.


/*
		strcpy  (str,"ISPU24 ");
		sprintf (str2, "V=%d	I=%d mA	        f=%d	t=%d ", 	(uint16)Zasilacz.napiecie, (uint16)Zasilacz.prad, (uint16)(Zasilacz.czestotliwosc*10), (uint16)(Zasilacz.okres_ms));
		strcat (str,str2);
		strcat (str," \r\n");
		SEGGER_RTT_printf(0, str);
*/

	  WDog1_Clear(NULL);
	  //WAIT1_Waitms(100);

	  NapSuma = 0;
	  PradSuma = 0;
	  //WYJSCIE_FRQ_ZAMKNIETE;		//tutaj do testow czasu trwania adc
	  for(i=0; i<ILOSC_PROBEK_ADC; i++)
	  {
		  (void)AD1_MIV_Measure(TRUE);
		  (void)AD1_MIV_GetValue16( ResultsADC);

		  ResultsADC[0]=ResultsADC[0]>>4;
		  ResultsADC[1]=ResultsADC[1]>>4;

		  PradSuma	+=(unsigned long int)ResultsADC[0];
		  NapSuma	+=(unsigned long int)ResultsADC[1];
	  }
	  //WYJSCIE_FRQ_OTWARTE;		//tutaj do testow czasu trwania adc
	  PradSrednia	=PradSuma/ILOSC_PROBEK_ADC;
	  NapSrednia	=NapSuma/ILOSC_PROBEK_ADC;

	  Zasilacz.napiecie	=(float)NapSrednia*KWANT*10.2307;
      Zasilacz.prad		=((float)PradSrednia*KWANT)/5.1;		//domyslnie jest ideowo ustawione wzmocnienie na 5, jednak rozrzut materialowy i tu =5.1
      //Zasilacz.prad=400;
      //Zasilacz.czestotliwosc=(uint16)(3.2*Zasilacz.prad+400);	//V1.0 -> 400..2000Hz, 200Hz - przeciazenie
      //Zasilacz.czestotliwosc=(0.095*Zasilacz.prad+2.0);		//V1.1 -> 2..40Hz przy czym 1Hz - przeciazenie
      Zasilacz.czestotliwosc=(0.1*Zasilacz.prad+2.0);			//V1.2 -> 2..42Hz przy czym 1Hz - przeciazenie (dodano obsluge w us)
//      Zasilacz.czestotliwosc=(0.05*Zasilacz.prad+2.0);			//V1.2 -> 2..42Hz przy czym 1Hz - przeciazenie (dodano obsluge w us)

      if(Zasilacz.czestotliwosc!=0)
      {
    	  ms_float=((float)1000.0/Zasilacz.czestotliwosc);
    	  Zasilacz.okres_ms=(uint16)roundf(ms_float);

    	  Zasilacz.okres_us= (uint16) ((float)1000000.0/Zasilacz.czestotliwosc);

      }




      if(Zasilacz.napiecie>21500)
       	  LED1_ClrVal(NULL);				//LED_ON
      else
    	  LED1_NegVal(NULL);




      //LED1_NegVal(NULL);
//=================================================================================================================================================================
       if(SWITCH_ON==SwitchS2_GetVal(NULL))	//SKANOWANIE USTAWIENIA SWITCHA
    	  SwitchBiezacy=eSWITCH_ON;
      else
    	  SwitchBiezacy=eSWITCH_OFF;

      if(SwitchBiezacy!=SwitchUstawiony)
   	  {
		  LicznikSwitchZmiana++;


	  	  if(LicznikSwitchZmiana>40)			//40*0,25s=10 SEKUND
	  	  {
	  		  LED1_SetVal(NULL);			//LED_OFF
	  		  WAIT1_Waitms(100);
   	  	  	  while(1);						//RESTART APLIKACJI - JESLI W TRAKCIE PRACY SWITCH ZOSTANIE PRZELACZONY TO BEDZIE NIEZGODNOSC I ZAINICJOWANYM HARDWAREM.
      	  	  	  	  	  	  	  	  	  	//RESTART I PONOWNA INICJALIZACJA ZGODNIE Z NOWYM USTAWIENIEM SWITCHA
	  	  }
   	  } else LicznikSwitchZmiana=0;
//=================================================================================================================================================================



 	  //PPG1_SetPeriodMS(MyPPG1Ptr,  Zasilacz.okres_ms );
      //PPG1_SetPeriodMS(MyPPG1Ptr,  25 );			//uzyta funkcja w ms bo w Hz za mala byla rozdzielczoœc zadawanych Hz!!!!
      //PPG1_SetPeriodUS(MyPPG1Ptr,  23810);


      if(Zasilacz.napiecie>21500+HISTEREZA)
      {
    	  if(SwitchUstawiony==eSWITCH_ON)			//czyli stan ==0
    	  {
    		  //PPG1_SetFrequencyHz(MyPPG1Ptr, Zasilacz.czestotliwosc);
    		  //PPG1_SetPeriodMS(MyPPG1Ptr,  (uint16_t)((float)(1000)/(float)(Zasilacz.czestotliwosc)) );
    		  if(Zasilacz.czestotliwosc<20)
    		  	  PPG1_SetPeriodMS(MyPPG1Ptr,  Zasilacz.okres_ms );			//uzyta funkcja w ms bo w Hz za mala byla rozdzielczoœc zadawanych Hz!!!!
    		  else
    			  PPG1_SetPeriodUS(MyPPG1Ptr,  Zasilacz.okres_us);

    	  }
    	  else
    		  {
    		  	  WYJSCIE_FRQ_ZAMKNIETE;
    		  }
      }
      else
      {
          if(Zasilacz.napiecie<21500-HISTEREZA)
          {
			  if(SwitchUstawiony==eSWITCH_ON)			//czyli stan ==0
			  {
//				  PPG1_SetFrequencyHz(MyPPG1Ptr, 3);
				  Zasilacz.czestotliwosc=1.0;
				  Zasilacz.okres_ms=1000;					//1Hz - przeciazenie !!!!!!!!!!!!!!!!!!!
	    		  PPG1_SetPeriodMS(MyPPG1Ptr,  Zasilacz.okres_ms );	//uzyta funkcja w ms bo w Hz za mala byla rozdzielczoœc zadawanych Hz!!!!
			  }
			  else
    	      	  {
    		  	  	  WYJSCIE_FRQ_OTWARTE;					//- PRZECIAZENIE LUB ZWARCIE
    	      	  }
          }
      }




      //OK!      PPG1_SetFrequencyHz(MyPPG1Ptr, 400);
      //ok!      PPG1_SetFrequencyHz(MyPPG1Ptr, Zasilacz.czestotliwosc);
       //     PPG1_SetPeriodMS(MyPPG1Ptr,100);
      /*
            for(int i = 0; i < 65535; ++i) {
              PPG1_SetRatio16(MyPPG1Ptr,i);   //change duty
              for(int j = 0; j < 6555; ++j);  //delay
            }
      */
      //      FRQ_NegVal(NULL);



  }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
