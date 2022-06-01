/*********************************************************************
** Device: Joystick **
** File: EF_Joystick_Test.c **
** **
** Created by ElecFreaks Robi.W /10 June 2011 **
** **
** Description: **
** This file is a sample code for your reference. **
** **
** Copyright (C) 2011 ElecFreaks Corp. **
*********************************************************************/

typedef struct
{
	unsigned a		: 1;
	unsigned b		: 1;
	unsigned c		: 1;
	unsigned d		: 1;
	int x;
	int y;
}joystick_t;

int FirstShotX , FirstShotY;
joystick_t joystick;
const int joystick_size = sizeof(joystick);
char buffer[joystick_size];

void setup()
{
	for(int i=0; i<9; i++)
	{
		pinMode(i, INPUT);
		digitalWrite(i, 1);
	}
	Serial.begin(9600);
	FirstShotX = 0;
	FirstShotY = 0;
	//joystick = (joystick_t*)calloc(1, sizeof(joystick_t));
}

void loop(){
	memset(buffer, 0, joystick_size);
	joystick.a = 0;
	joystick.b = 0;
	joystick.c = 0;
	joystick.d = 0;
	joystick.x = 0;
	joystick.y = 0;

	int i, someInt, flag = 0;
	for(i=2; i<9; i++)
	{
		someInt = digitalRead(i);
		if(someInt == 0)
		{
			flag =1;
			break;
		}
	}
	if(flag == 1)
	{
		switch(i)
		{
			case 2: 
				joystick.a = 1;
				break;
			case 3: 
				joystick.b = 1;
				break;
			case 4:
				joystick.c = 1;
				break;
			case 5:
				joystick.d = 1;
				break;
			default: break;
		}
		flag=0;
	}
	int sensorValue = analogRead(A0);
	if(FirstShotX == 0)
	{
		FirstShotX = sensorValue;
	}
	joystick.x = sensorValue - FirstShotX;
	sensorValue = analogRead(A1);
	if(FirstShotY == 0)
	{
		FirstShotY = sensorValue;
	}
	joystick.y = sensorValue - FirstShotY;
	memcpy(buffer, &joystick, sizeof(joystick));
	Serial.write((char*)(&joystick), sizeof(joystick));
	//Serial.println(joystick.x);
	//Serial.println(joystick.y);
	delay(200);
}
