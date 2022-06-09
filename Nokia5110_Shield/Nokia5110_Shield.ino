#define MAGIC 0xabbaceca

typedef struct __attribute__((__packed__))
{
  uint32_t magic;
  int16_t x;
  int16_t y;
  uint8_t buttons;
}joystick_t;

int16_t FirstShotX , FirstShotY;
joystick_t joystick;

void setup()
{
	for(int i=0; i<9; i++)
	{
		pinMode(i, INPUT);
		digitalWrite(i, 1);
	}
	Serial.begin(38400);
	FirstShotX = 0;
	FirstShotY = 0;
  	joystick.magic = 0xabbaceca;
}

void loop(){
  	joystick.buttons = 0;
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
			//TODO figure out button maping!
			case 2: 
				joystick.buttons = 1;
				break;
			case 3: 
				joystick.buttons = 1 << 1;
				break;
			case 4:
				joystick.buttons = 1 << 2;
				break;
			case 5:
				joystick.buttons = 1 << 3;
				break;
			default: break;
		}
		flag=0;
	}
	int16_t sensorValue = analogRead(A0);
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
	byte* ptr = (byte*)&joystick;
	for(i = 0; i < sizeof(joystick_t); i++)
	{
	    Serial.write(ptr[i]);
	}
	//Serial.println(sizeof(joystick_t));
}
