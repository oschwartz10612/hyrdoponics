/*
  28/8/2015  Michael Ratcliffe  Mike@MichaelRatcliffe.com
  23/12/2017  Owen Schwartz
*/
 
 
//************************** Libraries Needed To Compile The Script [See Read me In Download] ***************//
// Both below Library are custom ones [ SEE READ ME In Downloaded Zip If You Dont Know how To install] Use them or add a pull up resistor to the temp probe
 
 
#include <OneWire.h>
#include <DallasTemperature.h> 
 
 
//************************* User Defined Variables ********************************************************//
 
 const int pingPin = 7;

//##################################################################################
//-----------  Do not Replace R1 with a resistor lower than 300 ohms    ------------
//##################################################################################
 
 
int R1 = 1000;
int Ra = 25; //Resistance of powering Pins
int ECPin = A3;
int ECGround = A2;
int ECPower = A1;
 
 
//*********** Converting to ppm [Learn to use EC it is much better]**************//
// Hana      [USA]        PPMconverion:  0.5
// Eutech    [EU]          PPMconversion:  0.64
//Tranchen  [Australia]  PPMconversion:  0.7
 
 
float PPMconversion = 0.5;
 
 
//*************Compensating for temperature ************************************//
//The value below will change depending on what chemical solution we are measuring
//0.019 is generaly considered the standard for plant nutrients [google "Temperature compensation EC" for more info]
float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring
 
 
 
 
//********************** Cell Constant For Ec Measurements *********************//
//Mine was around 2.9 with plugs being a standard size they should all be around the same
//But If you get bad readings you can use the calibration script and fluid to get a better estimate for K
float K = 2.88;
 
 
 
 
//************ Temp Probe Related *********************************************//
#define ONE_WIRE_BUS 4            // Data wire For Temp Probe is plugged into pin 10 on the Arduino
 
 
 
//***************************** END Of Recomended User Inputs *****************************************************************//
 
 
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
 
 
float Temperature=10;
float EC=0;
float EC25 =0;
int ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;
 
long duration, inches, cm;
 
 
//*********************************Setup - runs Once and sets pins etc ******************************************************//
void setup()
{
  Serial.begin(9600);
  pinMode(TempProbeNegative, OUTPUT ); //seting ground pin as output for tmp probe
  digitalWrite(TempProbeNegative, LOW );//Seting it to ground so it can sink current
  pinMode(ECPin,INPUT);
  pinMode(ECPower, OUTPUT);//Setting pin for sourcing current
  pinMode(ECGround, OUTPUT);//setting pin for sinking current
  digitalWrite(ECGround,LOW);//We can leave the ............ground connected permanantly
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
 
  delay(100);// gives sensor time to settle
  sensors.begin();
  delay(100);
  //** Adding Digital Pin Resistance to [25 ohm] to the static Resistor *********//
  // Consule Read-Me for Why, or just accept it as true
  R1=(R1+Ra);// Taking into acount Powering Pin Resitance
  
/*Serial.println("Make sure Probe and Temp Sensor are in Solution and solution is well mixed");
  Serial.println("");
  Serial.println("Measurments at 5's Second intervals [Dont read Ec morre than once every 5 seconds]:"); */
 
 
};
//******************************************* End of Setup **********************************************************************//
 
  
 
 
//************ This Loop Is called From Main Loop************************//
void GetEC(){
 
 
//*********Reading Temperature Of Solution *******************//
sensors.requestTemperatures();// Send the command to get temperatures
Temperature=sensors.getTempCByIndex(0); //Stores Value in Variable
 
 
 
 
//************Estimates Resistance of Liquid ****************//
digitalWrite(ECPower,HIGH);
raw= analogRead(ECPin);
raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
digitalWrite(ECPower,LOW);
 
 
 
 
//***************** Converts to EC **************************//
Vdrop= (Vin*raw)/1024.0;
Rc=(Vdrop*R1)/(Vin-Vdrop);
Rc=Rc-Ra; //acounting for Digital Pin Resitance
EC = 1000/(Rc*K);
 
 
//*************Compensating For Temperaure********************//
EC25 = EC/ (1+ TemperatureCoef*(Temperature-25.0));
ppm=(EC25)*(PPMconversion*1000);
 
 
}
//************************** End OF EC Function ***************************//
 
 
 
 
//***This Loop Is called From Main Loop - Prints to serial info as json to main process***//
void PrintReadings(){
Serial.print("{");
Serial.print("\"rc\":");
Serial.print(Rc);
Serial.print(",");
Serial.print("\"ec\":");
Serial.print(EC);
Serial.print(",");
Serial.print("\"ppm\":");
Serial.print(ppm);
Serial.print(",");
Serial.print("\"temperature\":");
Serial.print(Temperature);
Serial.print(",");
Serial.print("\"cm\":");
Serial.print(cm);
Serial.print(",");
Serial.print("\"in\":");
Serial.print(inches);
Serial.println("}");
 
/*
//********** Usued for Debugging ************
Serial.print("Vdrop: ");
Serial.println(Vdrop);
Serial.print("Rc: ");
Serial.println(Rc);
Serial.print(EC);
Serial.println("Siemens");
//********** end of Debugging Prints *********
*/
};

long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are 73.746
  // microseconds per inch (i.e. sound travels at 1130 feet per second).
  // This gives the distance travelled by the ping, outbound and return,
  // so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}
 
 
//************************************* Main Loop - Runs Forever ***************************************************************//
//Moved Heavy Work To subroutines so you can call them from main loop without cluttering the main loop
void loop()
{
  // establish variables for duration of the ping, and the distance result
  // in inches and centimeters:
 
  
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  
  // The same pin is used to read the signal from the PING))): a HIGH pulse
  // whose duration is the time (in microseconds) from the sending of the ping
  // to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  
  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
   
  GetEC();          //Calls Code to Go into GetEC() Loop [Below Main Loop] dont call this more that 1/5 hhz [once every five seconds] or you will polarise the water
  PrintReadings();  // Cals Print routine [below main loop]
   
   
  delay(5000);
 
}
//************************************** End Of Main Loop **********************************************************************//
 

