/*----------------------------------------------------------------------------
# Implemented code for lab 6
1. Activity 1: Download, read and run the sample project using the accelerometer and a terminal emulator (as in lab 5).
2. Activity 2: Detect the orientation of the development board.
3. Activity 3: Specify and implement a timed sequence of orientation states.
4. NOTE: Activity 3 is still under progress. It shows the transitions between the states but the flags/interupts has not been implemented in the code included
The project uses a single thread. But, we have created another thread for activity 3, since, activity 3 is not complete so we have commented out that thread for now.
    
 *---------------------------------------------------------------------------*/
 
#include "cmsis_os2.h"
#include <MKL25Z4.h>
#include <stdbool.h>
#include "../inc/rgb.h"
#include "../inc/serialPort.h"
#include "../inc/led.h"
#include "../inc/i2c.h"
#include "../inc/accel.h"

/*--------------------------------------------------------------
 *   Thread t_accel
 *      Read accelarations periodically  
 *      Write results to terminal
 *      Toggle green LED on each poll
 *--------------------------------------------------------------*/
osThreadId_t t_accel;      /* id of thread to poll accelerometer */

// convert signed integer +/- 999 to +/-X.XX
//   SX.XX
//   01234
void aToString(int16_t a, char* s) {  
    bool negative = false ;
    if (a < 0) {
        a = -a ;
        negative = true ; 
    }

    // get digits
    s[4] = '0' + (a % 10) ;
    a = a / 10 ;
    s[3] = '0' + (a % 10) ;
    a = a / 10 ;
    s[1] = '0' + (a % 10) ;
        
    // set sign
    s[0] = '+' ;
    if (negative) {
        s[0] = '-' ;
    }
}

// buffer for message
char xyzStr[] = "X=SX.XX Y=SX.XX Z=SX.XX" ;
//               01234567890123456789012


//defining all the states used in activity 2
#define Intermidiate 0
#define Flat 1
#define Right 2
#define Left 3
#define Over 4
#define Up 5
#define Down 6
int x=0;
int y=0;
int z=0;
  
//code for activity 2
void accelThread(void *arg) {
    int16_t xyz[3] ; // array of values from accelerometer
        // signed integers in range +8191 (+2g) to -8192 (-2g)
    
    // initialise green LED
    setRGB(GREEN, RGB_ON) ;
    int state = Intermidiate ; //setting initial state
    
    // initialise accelerometer
    int aOk = initAccel() ;
    if (aOk) {
        sendMsg("Accel init ok", CRLF) ;
    } else {
        sendMsg("Accel init failed", CRLF) ;
    }
    while(1) {
        osDelay(20) ;   //accelerometer is polled every 20ms approximately
        readXYZ(xyz) ; // read X, Y Z values
      
        x=(xyz[0] * 100) / 4096;
      
        // write X scaled integer
        aToString((xyz[0] * 100) / 4096, &xyzStr[2]) ;

        y=(xyz[1] * 100) / 4096;

        // write Y scaled integer
        aToString((xyz[1] * 100) / 4096, &xyzStr[10]) ;

        z=(xyz[2] * 100) / 4096;
 
        // write Z scaled integer
        aToString((xyz[2] * 100) / 4096, &xyzStr[18]) ;
        sendMsg(xyzStr, CRLF) ;  
        if((x*x)+(y*y)+(z*z)<=12000)  //should have been 10000 but used 12000 to have some buffer for errors
        {
        
        // toggle green LED
        switch (state) {
            case Intermidiate:  //Intermidiate state
              if(z>90)
              {
                 state = Flat ;
                 sendMsg("Flat state", CRLF) ;  
              }
              else if(z<-90)
              {
                 state = Over ;
                 sendMsg("Over state", CRLF) ;  
              }
              else if(y<-90)
              {
                 state = Right ;
                 sendMsg("Right state", CRLF) ;  
              }
              else if(y>90)
              {
                 state = Left ;
                 sendMsg("Left state", CRLF) ;  
              }              
              else if(x<-90)
              {
                 state = Up ;
                 sendMsg("Up state", CRLF) ;  
              }
              else if(x>90)
              {
                 state = Down ;
                 sendMsg("Down state", CRLF) ;  
              }
                break ;
            case Flat:  //flat state
                if(z<80)
                {
                  state = Intermidiate ;
                }
                break ;
            case Right:  //right state
                if(y>-80)
                {
                  state = Intermidiate ;
                }
                break ;
            case Left:  //left state
                if(y<80)
                {
                  state = Intermidiate ;
                }
                break ;
            case Over:  //over state
                if(z>-80)
                {
                  state = Intermidiate ;
                }
                break ;
            case Up:  //Up state
                if(x>-80)
                {
                  state = Intermidiate ;
                }
                break ;
            case Down:  //down state
                if(x<80)
                {
                  state = Intermidiate ;
                }
                break ;
            
        }    
      }        
    }
}
 



//defining the states used in activity 3
#define Flat_min10 21
#define Right_wait_2sec 22
#define Up_wait_4sec 23
#define sequenceError 24
#define timeError 25

//code for activity 3
void accelThread_act3(void *arg) {
    int16_t xyz[3] ; // array of values from accelerometer
        // signed integers in range +8191 (+2g) to -8192 (-2g)
    
    // initialise green LED
    setRGB(GREEN, RGB_ON) ;
    int state = Flat_min10 ;  //initial state
	int time;
    uint32_t flags;
	  uint32_t delay;
    // initialise accelerometer
    int aOk = initAccel() ;
    if (aOk) {
        sendMsg("Accel init ok", CRLF) ;
    } else {
        sendMsg("Accel init failed", CRLF) ;
    }
    while(1) {
        osDelay(20) ;  //accelerometer is polled every 20ms approximately
		//flags= osEventFlagsWait(stateFlags, MASK(Reset_evt),NULL, delay);
        readXYZ(xyz) ; // read X, Y Z values
    
        x=(xyz[0] * 100) / 4096;
      
        // write X scaled integer
        aToString((xyz[0] * 100) / 4096, &xyzStr[2]) ;

        y=(xyz[1] * 100) / 4096;

        // write Y scaled integer
        aToString((xyz[1] * 100) / 4096, &xyzStr[10]) ;

        z=(xyz[2] * 100) / 4096;
 
        // write Z scaled integer
        aToString((xyz[2] * 100) / 4096, &xyzStr[18]) ;
        sendMsg(xyzStr, CRLF) ;  
        if((x*x)+(y*y)+(z*z)<=12000)  //should have been 10000 but used 12000 to have some buffer for errors
        {
        
        // toggle green LED
        switch (state) {
            
			case Flat_min10:  //Flat_min10 state
                ledOnOff(LED1,1);
                if(state!=Right && time>=10)
				{
					state=sequenceError;
				}
				else if(time<10 && state!=Flat)
				{
					state=timeError;
				}
				else if(state==Right && time>=10){
					state=Right_wait_2sec;
				}
                break ;
            case Right_wait_2sec:  //Right_wait_2sec state
				ledOnOff(LED1,1);
				ledOnOff(LED2,1);
                if(state!=Right )
				{
					state=sequenceError;
				}
				else if(time<2 && state!=Right)
				{
					state=timeError;
				}
				else if(state==Right){
				    state=Right;
			    }
                break ;
            case Right:  //right state
                ledOnOff(LED1,1);
				ledOnOff(LED2,1);
				if(state!=Up )
				{
				    state=sequenceError;
				}
				else if(time>6)
				{
					state=timeError;
				}
				else if(state==Up){
					state=Up_wait_4sec;
				}
                break ;
            case Up_wait_4sec:  //Up_wait_4sec state
                ledOnOff(LED1,1);
				ledOnOff(LED2,1);
				ledOnOff(LED3,1);
				if(state!=Up )
				{
					state=sequenceError;
				}
				else if(time<4)
				{
					state=timeError;
				}
				else if(state==Up){
					state=Up;
				}
                break ;

            case Up:  //up state
                ledOnOff(LED1,1);
				ledOnOff(LED2,1);
			    ledOnOff(LED3,1);
				if(state!=Flat )
				{
					state=sequenceError;
				}
				else if(time>8)
				{
					state=timeError;
				}
				else if(state==Flat){
					state=Flat;
				}
                break ;
            case Flat:  //Flat state
                ledOnOff(LED1,0);
				ledOnOff(LED2,0);
				ledOnOff(LED3,0);
                break ;
            case sequenceError:  //sequenceError state
                sendMsg("sequence error!", CRLF) ; 
                break ;
            case timeError:  //timeError state
                sendMsg("time error!", CRLF) ; 
                break ;
        }    
      }        
    }
}












/*----------------------------------------------------------------------------
 * Application main
 *   Initialise I/O
 *   Initialise kernel
 *   Create threads
 *   Start kernel
 *---------------------------------------------------------------------------*/

int main (void) { 
    
    // System Initialization
    SystemCoreClockUpdate();

    //configureGPIOinput();
    init_UART0(115200) ;

    // Initialize CMSIS-RTOS
    osKernelInitialize();
    
    // initialise serial port 
    initSerialPort() ;

    // Initialise I2C0 for accelerometer 
    i2c_init() ;
    
    // Initialise GPIO for on-board RGB LED
    configureRGB() ;
    
    // Create threads
    t_accel = osThreadNew(accelThread, NULL, NULL); 
    //t_accel = osThreadNew(accelThread_act3, NULL, NULL); 
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
