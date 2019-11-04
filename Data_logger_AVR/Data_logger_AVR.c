#include <kamavr.h>
#include <stdio.h>

int MAX_SIZE = 100;
int vtg_dec,speed_select;

float buff[100];
int front = -1;
int rear  = -1;

//Function to check if the buffer is full.
int Is_full ()
{
	if (rear == ((front % MAX_SIZE) + 1))
		return 1;
	else
		return 0;
}

//Function to check if buffer is empty or not.
int Is_empty ()
{
	if (front == rear)
		return 1;
	else
		return 0;
}

//Function to write data to buffer. Increment the front pointer circularly.
void write_to_buffer (float vtg)
{
	int full,empty;
	full = Is_full ();
	empty = Is_empty ();

	if (full)
	{
		return;
	}
	else if (empty)
	{
		front = 0;
		rear = 0;
		buff[front] = vtg;
		front = 1;
	}
	else
	{
		buff[front] = vtg;
		front = ((front + 1) % MAX_SIZE);
		
	}
}

//Function to read from buffer. Increment the rear pointer circularly.
void read_from_buffer ()
{
	int empty;
	empty = Is_empty ();
	if (empty)
	{
		return;
	}
	else 
	{
		rear = (rear + 1) % MAX_SIZE;
	}
}

//Function to convert the decimal voltage value into float in range 0-5 volt.
//Returns the voltage in float ranging from 0-5 V.
float conv_vtg (int vtg_dec)
{
	float step_size,vtg;

	step_size = 0.0049;


	vtg = step_size*vtg_dec;

	return vtg;
}

//Function to convert Analog to Digital voltage using internal AD converter.
//It returns the Voltage value in decimal (0 - 1023) range.
int getadc ()
{
	unsigned int val;

	ADCSR |= (1<<ADSC);

	while ((ADCSR & (1<<ADSC)));

	val = ADC;

	return val; 
}

//This is a function for the initialization of the ADC
void init_ADC ()
{
	ADCSR = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);	//125KHz prescalar
	ADCSR |= (1<<ADEN);
}

void init_timer1 ()
{
	TCCR1B = 0x0B;
	OCR1AH = speed_select;
	TIMSK = 0x10;
}

ISR (TIMER1_COMPA_vect)
{
	vtg_dec = getadc ();			//Get decimal value of vtg by doing A-D.
}
//This is a function to display the voltage on LCD
void disp_vtg_lcd (float vtg)
{
	int voltage;

	voltage = vtg*10;					//Get only 1 digit after the decimal point.

	write_lcd (0,0x8D);
	write_lcd (0x8D,'0'+ voltage/10);	//Display the 1st digit on LCD.

	write_lcd (0x8E,0x2E);				//Display the decimal point.

	write_lcd (0x8F,'0'+ voltage%10);	//Display the digit after the decimal point.

}


void disp_bar_lcd (float vtg)
{
	int i;
		if (vtg <= 1.0)
	{
		char patt1[] = {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b11111};
		write_lcd (0,0x40);
		for (i=0;i<8;i++)
		{
			write_lcd (1,patt1[i]);
		}
		write_lcd (0,0x8C);
		write_lcd (1,0);
		
	}
	else if (vtg > 1.0 && vtg <= 2.0)
	{
		char patt2[] = {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b11111,0b11111};
		write_lcd (0,0x48);
		for (i=0;i<8;i++)
		{
			write_lcd (1,patt2[i]);
		}
		write_lcd (0,0x8C);
		write_lcd (1,1);
		
	}
	else if (vtg > 2.0 && vtg <= 3.0)
	{
		char patt3[] = {0b00000,0b00000,0b00000,0b00000,0b11111,0b11111,0b11111,0b11111};
		write_lcd (0,0x50);
		for (i=0;i<8;i++)
		{
			write_lcd (1,patt3[i]);
		}
		write_lcd (0,0x8C);
		write_lcd (1,2);
		
	}
	else if (vtg > 3.0 && vtg <= 4.0)
	{
		char patt4[] = {0b00000,0b00000,0b00000,0b11111,0b11111,0b11111,0b11111,0b11111};
		write_lcd (0,0x58);
		for (i=0;i<8;i++)
		{
			write_lcd (1,patt4[i]);
		}
		write_lcd (0,0x8C);
		write_lcd (1,3);
		
	}
	else if (vtg > 4.0 && vtg < 5.0)
	{
		char patt5[] = {0b00000,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111};
		write_lcd (0,0x60);
		for (i=0;i<8;i++)
		{
			write_lcd (1,patt5[i]);
		}
		write_lcd (0,0x8C);
		write_lcd (1,4);
		
	}
	else
	{
		char patt6[] = {0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111};
		write_lcd (0,0x68);
		for (i=0;i<8;i++)
		{
			write_lcd (1,patt6[i]);
		}
		write_lcd (0,0x8C);
		write_lcd (1,5);
	}


}

void main()
{
	
	float vtg;
	unsigned char key_val_speed;
	int full;
	int speed_control[] = {2500,2750,3125,3500,4250,5000,6250,8250,12500};
	
	DDRC = 0xff;

	init_ADC ();

	init_lcd ();

	init_7seg ();

	while ((key_val_speed = readkey()) == 0xff);
	speed_select = speed_control[key_val_speed - 1];

	init_timer1 ();

	sei ();

	while (1)
	{

		dis_7seg_num (1,vtg_dec);				

		vtg = conv_vtg (vtg_dec);		//Convert the decimal value into float.
		
		if ((front < (MAX_SIZE - 1)) && ((rear == -1)||(rear == 0)))
		{
			write_to_buffer (vtg);			//Store voltage in the buffer.
		} 

		if ((front == (MAX_SIZE - 1)) && (rear == 0))
		{
			write_to_buffer (vtg);
			read_from_buffer ();
			
		}

		if ((full = Is_full()) == 1)
		{
			read_from_buffer ();
			write_to_buffer (vtg);
		}

		disp_vtg_lcd (vtg);					//Display the voltage to LCD.
	
		disp_bar_lcd (vtg);					//Display the bar graph for the voltage.

		write_lcd (0,0xC0);					//Display speed on the LCD
		write_lcd (1,'S');
		write_lcd (1,0x3A);
		write_lcd (1,'0' + key_val_speed);

		num_lcd (0xCD,front);				//Display the current sample index.
		
	
	}
	
}
