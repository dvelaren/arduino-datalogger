//Name:         datalogDAQ
//Author:       David Velasquez
//Date:         6/02/2018
//Description: This program serves as a system datalogger for system identification. It generates a .txt file with
//Setpoint, Measurement and Time. The Measurement signal is filtered through a Recursive Moving Average (RMA)

//Library Import
#include <SPI.h>  //SPI Library
#include <SD.h> //SD Library

//I/O pin definition
#define SENSOR 0 //Light dependant resistor on pin A0
#define ACTUATOR 2  //Power LEDs on pin 2
#define SW 3  //Switch for injecting setpoint
#define CSSD 4  //Chip Select pin for SD on pin 4

//Constant definitions
const unsigned int NUMREADS = 2;  //Number of samples for moving average
const unsigned long TPRINT = 1000;  //Debug printing time (1 sec)
const unsigned long TSAM = 10;  //Sampling time (10 ms)
unsigned long TDEB = 50;    // the debounce time; increase if the output flickers
const float SPF = 500;  //Fixed setpoint constant

//Variable definitions
unsigned int Setpoint = 0, Measurement = 0; //Fixed setpoint to inject and var to store measurement
float stime = 0;  //Variable to store the aproximated sampling time
//Variables for RMA
unsigned int readings[NUMREADS] = {0};
unsigned int readIndex = 0;
unsigned long total = 0;
//Debouncing vars
boolean ledState = true;         // the current state of the output pin
boolean buttonState;             // the current reading from the input pin
boolean lastButtonState = false;   // the previous reading from the input pin

//Timing vars
unsigned long tinip = 0;  //Initial time for printing
unsigned long tinis = 0;  //Initial time for sampling
unsigned long tinid = 0;  //Initial time for debouncing

//Subroutines and functions
unsigned int smooth(unsigned int analogPin, unsigned long &total, unsigned int *readings, unsigned int &readIndex, unsigned int NUMREADS) {
  total = total - readings[readIndex];  //Subtract the last reading
  readings[readIndex] = analogRead(analogPin); //Read from the sensor:
  total = total + readings[readIndex];  //Add the reading to the total:
  readIndex = readIndex + 1;  //Advance to the next position in the array:

  if (readIndex >= NUMREADS) { //If we're at the end of the array,
    readIndex = 0;  //Start again at the beginning
  }
  return total / NUMREADS; //Calculate the average
}

void datalog() {
  if (millis() - tinis >= TSAM) {
    stime = stime + TSAM / 1000.0;
    //SD Logging
    String datalog = ""; //Define datalog as string for storing data in SD as text
    datalog += String(Setpoint, 4);
    datalog += ",";
    datalog += String(Measurement, 4);
    datalog += ",";
    datalog += String(stime, 4);
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(datalog);
      dataFile.close();
      // print to the serial port too:
      //Serial.println("Success logging data");
    }
    // if the file isn't open, pop up an error:
    else {
      //Serial.println("error opening datalog.txt");
    }
    tinis = millis(); //Reset tini
  }
}

void printvars() {
  if (millis() - tinip >= TPRINT) {
    Serial.print(F("SP: "));
    Serial.print(Setpoint);
    Serial.print(F(" Y: "));
    Serial.print(Measurement);
    Serial.print(F(" t: "));
    Serial.println((millis() - tinip)/1000.0);
    tinip = millis();
  }
}

void MeasInitialize() {
  for (unsigned int i = 0; i < NUMREADS*2; i++) {
    Measurement = smooth(SENSOR, total, readings, readIndex, NUMREADS);
  }
}

boolean debounce(unsigned int buttonPin, boolean &ledState, boolean &buttonState, boolean &lastButtonState, unsigned long &lastDebounceTime) { //Function to debounce the switch
  // read the state of the switch into a local variable:
  boolean reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > TDEB) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      // only toggle the LED if the new button state is HIGH
      if (buttonState == true) {
        ledState = !ledState;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  return ledState;
}

void setup() {
  //Pin configuration
  pinMode(SW, INPUT); //Switch as input
  pinMode(ACTUATOR, OUTPUT); //Actuator as output

  //Output cleaning
  digitalWrite(ACTUATOR, LOW);
  MeasInitialize(); //Initialize measurements
  
  //Communications
  Serial.begin(9600);
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(CSSD)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    while (1);
  }
  Serial.println(F("Card initialized."));
  tinip = millis();  //Initial time reset
  tinis = millis();  //Initial time reset
  tinid = millis(); //Initial time reset
}

void loop() {
  Measurement = smooth(SENSOR, total, readings, readIndex, NUMREADS); //Acquire measurement and smooth it through RMA
  if (debounce(SW,ledState,buttonState,lastButtonState,tinid)==true) {
    Setpoint = SPF; //Assign the desired setpoint
  }
  else {
    Setpoint = 0; //Clear the desired setpoint
  }
  datalog();  //Datalog variables
  printvars();  //Print vars for debug
  analogWrite(ACTUATOR, Setpoint*255.0/1023.0);  //Inject the setpoint to the output
}
