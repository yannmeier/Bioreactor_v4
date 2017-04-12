void setup() {
  // put your setup code here, to run once:
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(9, HIGH);
  delayMicroseconds(500);
  digitalWrite(9, LOW);
  delayMicroseconds(500);
}
