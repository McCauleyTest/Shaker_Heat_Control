
/* McCauley Propeller Systems
   Hot Box Control v1.0
   Mar 29, 2013
   Programmer: Timothy Williams
*/


// which analog pin to connect
const int THERMISTORPIN[] = {A0,A1};

// resistance at 25 degrees C
const int THERMISTORNOMINAL = 10000;      
// temp. for nominal resistance (almost always 25 C)
const int TEMPERATURENOMINAL = 25;   
// how many samples to take and average, more takes longer
// but is more 'smooth'
const int NUMSAMPLES = 5;
// The beta coefficient of the thermistor (usually 3000-4000)
const int BCOEFFICIENT = 3977;
// the value of the 'other' resistor
const int SERIESRESISTOR = 10000;  
// the output for temp data
const int ANTEMPIN = 10;
const int ANTEMPOUT = 11;
// control outputs for airconditioner
const int  AC1 = 2;
const int  AC2 = 3;
//control parameters for air conditioner loop
const int TEMPLOW = 200;     //the low temp point where heat gun turn on
const int TEMPHI = 205;      // the high temp point where heat gun turns off
const int SHORTCYCLE = 10 * 1000;  //short cycle delay to allow compressor time to depressurize
const int SHORTCYCLELED = 13; 
int samples[NUMSAMPLES];
long previousMillis = 0;
unsigned long currentMillis = 0;
int intialshortcycle = 0;


void setup(void) 
{
  Serial.begin(9600);
  //analogReference(EXTERNAL);  // sets the analog in reference to some other voltage.
                                // In this case is used to be to the 3.3 Vdc but the 
                                //Sparkfun Pro Mini doesn't have analog reference.
  pinMode(AC1, OUTPUT);
  pinMode(AC2, OUTPUT);
}
 
void loop(void) 
{
  
  
  
  Serial.println("Begin");
  if (intialshortcycle == 0)            //In case power is short-cycled on the controller, this
  {                                     //will engage a delay so the airconditioners aren't 
    digitalWrite(SHORTCYCLELED, HIGH);  //damaged from power cycling
    Serial.println("Short Cycle Delay ON");
    previousMillis = millis();
    delay(60000);          
    digitalWrite(SHORTCYCLELED, LOW); 
    intialshortcycle = 1;
    Serial.println("Short Cycle Delay OFF");
  }
  
  
  int a;
  int i;
  float average[2];
  
  
  // take N samples in a row, with a slight delay, loop through twice for each temp
  for (a=0; a<2; a++)
  {
  for (i=0; i< NUMSAMPLES; i++) 
  {
   samples[i] = analogRead(THERMISTORPIN[a]);
   delay(10);
  }
  
  // average all the samples out
  average[a] = 0;
    for (i=0; i< NUMSAMPLES; i++) {
    
     average[a] += samples[i];
  }
  average[a] /= NUMSAMPLES;
  }
  Serial.print("Average analog reading inside the box "); 
  Serial.println(average[0]);
  Serial.print("Average analog reading outside the box ");
  Serial.println(average[1]);
 
  // convert the value to resistance
  for (a=0; a<2; a++) {
  average[a] = 1023 / average[a] - 1;
  average[a] = SERIESRESISTOR / average[a];
  }
  Serial.print("Thermistor resistance inside the box is "); 
  Serial.println(average[0]);
  Serial.print("Thermistor resistance outside the box is ");
  Serial.println(average[1]); 
 
 float steinhart[2];
 for (a=0; a<2; a++){
  
  steinhart[a] = average[a] / THERMISTORNOMINAL;     // (R/Ro)
  steinhart[a] = log(steinhart[a]);                  // ln(R/Ro)
  steinhart[a] /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart[a] += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart[a] = 1.0 / steinhart[a];                 // Invert
  steinhart[a] -= 273.15;                         // convert to C
  steinhart[a] *= 9;                             // begin conversion to F
  steinhart[a] /= 5;
  steinhart[a] += 32;                             // finish conversion to F
 }
 
  Serial.print("Temperature inside the box is "); 
  Serial.print(steinhart[0]);
  Serial.println(" *F");
  Serial.print("Temperature outside the box is ");
  Serial.print(steinhart[1]);
  Serial.println(" *F");
  
  
  // Write the temperatures to PWM outputs for shaker controller input
  float tempin = map(steinhart[0], 30, 230, 0, 255);
  float tempout = map(steinhart[1], 30, 230, 0, 255);
  analogWrite(ANTEMPIN, tempin);
  analogWrite(ANTEMPOUT, tempout);
  
  currentMillis = millis();
  // First control loop to control one air conditioner
  if (steinhart[0] < TEMPLOW)
  {
      digitalWrite(AC1, HIGH);
      digitalWrite(AC2, HIGH);
      previousMillis = currentMillis;
    }
 
 
   if (steinhart[0] > TEMPHI)
   {
    if (currentMillis - previousMillis > 10000)
    {
     digitalWrite(AC1, LOW);
     digitalWrite(AC2, LOW);
     digitalWrite(SHORTCYCLELED, LOW);
     
    }
    else
    {
      Serial.println("Short Cycle Delay");
      digitalWrite(SHORTCYCLELED, HIGH);
    }
   }

  delay(100);
}

