/*****************************************************************//**
 * @file main_sampler_test.cpp
 *
 * @brief Basic test of nexys4 ddr mmio cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

// #define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "xadc_core.h"
#include "sseg_core.h"
#include "spi_core.h"
#include "i2c_core.h"
#include "ps2_core.h"
#include "ddfs_core.h"
#include "adsr_core.h"

void chasingLED(GpoCore *led_p, int numOfLED, int &delay, bool pause)
{
	static int counter = 0;
	static bool bounceLeft = false;

	if(counter == numOfLED - 1)
		bounceLeft = true;
	else if(counter == 0)
		bounceLeft = false;
	if(pause)						// If pause, stay at current position
	{
		led_p->write(1,counter);
		counter = counter;
	}
	else								// Bounce LED
	{
		led_p->write(1,counter);
		sleep_ms(delay);
		led_p->write(0,counter);
		sleep_ms(delay);
		if(!bounceLeft)
			counter++;
		else
			counter--;
	}
}

bool checkF1Key(char ch)
{
	if((int)ch == -16)			// -16 is F1 key
		return true;
	return false;
}

bool checkKeyNum(char ch)
{
	if(ch >= '0' && ch <= '9')		// Check if character pressed is between value 0 and 9
		return true;
	return false;
}

void pausePlay(bool &pause)
{
	uart.disp(pause ? "Pause" : "Play");		//Pause or play
	uart.disp("\n\r");
	pause = !pause;
}

int getThreeDigKey(Ps2Core *ps2_p, bool &invalid, bool &pause)
{
	char ch;
	int delayVal, val;
	static int counter = 0, prevDelayVal;
	while(counter != 3 || invalid)
	{
		if(ps2_p->get_kb_ch(&ch))
		{
			if(checkKeyNum(ch))
			{
				if(counter == 0)
					val = (ch - '0') * 100;			//First value is 100th place
				else if(counter == 1)
					val = (ch - '0') * 10;			//Next value is 10th place
				else if(counter == 2)
					val = (ch - '0');				//Last value is 1 place
				delayVal += val;					//Add to get final user input
			}
			else
			{
				invalid = true;						//Invalid means invalid key pressed (not 0-9
				delayVal = prevDelayVal;			//Reset the last known valid input
				if(ch == 'p')
					pausePlay(pause);				//Pause or play if user presses "p"
				break;

			}
			counter++;
		}
	}
	prevDelayVal = delayVal;
	counter = 0;
	return delayVal;
}

void kbChasingLED(Ps2Core *ps2_p,GpoCore *led_p, int numOfLED)
{
	bool invalid = false;
	static bool pause = false;
	char ch;
	static int currentDelay = 500;
	int newDelay ;

	if (ps2_p->get_kb_ch(&ch))
	{
		if(ch == 'p')				//Pause or play if user presses p
			pausePlay(pause);
		else
		{
			if(checkF1Key(ch))		//If user presses F1
				newDelay = getThreeDigKey(ps2_p, invalid, pause);	//Get the 3 values following
			uart.disp("Delay : ");
			uart.disp(newDelay);
			uart.disp("\n\r");
		}
	}
	if(!invalid)					//If user put sequence correctly, it will store the new value
		currentDelay = newDelay;
	chasingLED(led_p, numOfLED, currentDelay, pause);		//Chasing LED based off delay values
}

void ps2_check(Ps2Core *ps2_p) {
   int id;
   int lbtn, rbtn, xmov, ymov;
   char ch;
   unsigned long last;

   uart.disp("\n\rPS2 device (1-keyboard / 2-mouse): ");
   id = ps2_p->init();
   uart.disp(id);
   uart.disp("\n\r");
   last = now_ms();
   do {
      if (id == 2) {  // mouse
         if (ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov)) {
            uart.disp("[");
            uart.disp(lbtn);
            uart.disp(", ");
            uart.disp(rbtn);
            uart.disp(", ");
            uart.disp(xmov);
            uart.disp(", ");
            uart.disp(ymov);
            uart.disp("] \r\n");
            last = now_ms();

         }   // end get_mouse_activitiy()
      } else {
         if (ps2_p->get_kb_ch(&ch)) {
            uart.disp(ch);
            uart.disp(" ");
            last = now_ms();
         } // end get_kb_ch()
      }  // end id==2
   } while (now_ms() - last < 5000);
   uart.disp("\n\rExit PS2 test \n\r");

}

int determinePs2ID(Ps2Core *ps2_p)
{
	int id;
	uart.disp("\n\rPS2 device (1-keyboard / 2-mouse): ");
	id = ps2_p->init();
	uart.disp(id);
	uart.disp("\n\r");
	return id;
}

GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
XadcCore adc(get_slot_addr(BRIDGE_BASE, S5_XDAC));
PwmCore pwm(get_slot_addr(BRIDGE_BASE, S6_PWM));
DebounceCore btn(get_slot_addr(BRIDGE_BASE, S7_BTN));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
SpiCore spi(get_slot_addr(BRIDGE_BASE, S9_SPI));
I2cCore adt7420(get_slot_addr(BRIDGE_BASE, S10_I2C));
Ps2Core ps2(get_slot_addr(BRIDGE_BASE, S11_PS2));
DdfsCore ddfs(get_slot_addr(BRIDGE_BASE, S12_DDFS));
AdsrCore adsr(get_slot_addr(BRIDGE_BASE, S13_ADSR), &ddfs);

int main() {
   //uint8_t id, ;
	int id = determinePs2ID(&ps2);
	while (1) {
		kbChasingLED(&ps2, &led, 16);
	} //while
} //
