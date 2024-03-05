const int pinoLed = 33;
const int pinoMotor = 26;

void setup() {
  pinMode(pinoLed, OUTPUT);
  pinMode(pinoMotor, OUTPUT);
}

void loop() {
  for(int i = 0; i<256; i++){
    analogWrite(pinoLed, i);
    analogWrite(pinoMotor, i);
    delay(100);

  }
}