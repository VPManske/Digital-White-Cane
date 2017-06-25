int pinout[] = {3,4,6,9,10};
int analog[] = {A0,A1,A2,A5,A6};


void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
}

void loop() {
  int digitaldist = 0;
  int sensorValue = 0;
  // put your main code here, to run repeatedly:
  for(int i = 0; i < 5; i++)
  {
        sensorValue = analogRead(analog[i]);
        digitaldist = 10650.08 * pow(sensorValue,-0.935) - 10;
        if (digitaldist <= 140 && digitaldist >=100)
          analogWrite(pinout[i], 20);
        else if (digitaldist <=99 && digitaldist >= 50)
          analogWrite(pinout[i], 100);
        else if (digitaldist <49 && digitaldist >=10)
          analogWrite(pinout[i], 255);
        else 
          analogWrite(pinout[i], 0);
  }
}
