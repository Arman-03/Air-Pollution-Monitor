const int CO2_PIN = 13; 
unsigned long duration, th, tl;          
int ppm;       

void CO2_Monitor() {
  th = pulseIn(CO2_PIN, HIGH, 2008000) / 1000;
  tl = 1004 - th;
  ppm = 2000 * (th - 2) / (th + tl - 4);
  Serial.print("CO2 Concentration: ");
  Serial.println(ppm);
  }

  void setup() {
  Serial.begin(115200);      
  pinMode(CO2_PIN, INPUT);   
  delay(1000);     
}

void loop() {
  CO2_Monitor();  
  delay(6000);            
}