
int count = 0;

void setup() {
  Serial.begin(9600);
  if (Particle.variable("countVar", count)) {
    Serial.println("Registered");
  }
}

void loop() {
  delay(5000);
  count = count + 1;
  Serial.println(count);
}
