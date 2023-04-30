// include the library code:
#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RTClib.h>
#include <SD.h>


// variables de sistema
#define NUM_SAMPLES 20
#define PORT_VOL A0
#define PORT_AMP A1
#define DHTPin 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define oneWirePin 2
#define chipSelect 53 //Puerto CS
#define Nom_Fich "log.csv"
DeviceAddress insideThermometer = { 0x28, 0xFF, 0x64, 0x0E, 0x78, 0x28, 0xFA, 0x24 };
DeviceAddress outsideThermometer = { 0x28, 0xFF, 0x64, 0x0E, 0x78, 0x1B, 0x9D, 0xBA };
float SENSIBILITY = 0.066; // Modelo 30A
int samplesNumber = 100;
float v1 = 5.05; // valor real de la alimentacion de Arduino, Vcc
float r1 = 994000; // 1M
float r2 = 99000; // 100K
 
//creacion de estructuras
OneWire oneWireBus(oneWirePin);
DallasTemperature sensors(&oneWireBus);
DHT dht(DHTPin, DHTTYPE);

RTC_DS1307 rtc; //reloj DS1307
//RTC_DS3231 rtc; //reloj DS3231

void incializa_SD(){
  String str = Nom_Fich;
  int str_len = str.length() + 1;
  char char_fich[str_len];
  str.toCharArray(char_fich, str_len);
  Serial.println("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  if (!SD.exists(char_fich)) {
    File dataFile = SD.open(char_fich, FILE_WRITE);
    dataFile.println("Hora(sec);Amperios;Voltaje;Temperatura1;Temperatura2;Temperatura3");
    Serial.println("Creo nuevo fichero");
    dataFile.close();
  }
  else Serial.println("Fichero existente");
}

void escribe_SD(String dataString){
  String str = Nom_Fich;
  int str_len = str.length() + 1;
  char char_fich[str_len];
  str.toCharArray(char_fich, str_len);
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(char_fich, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.print("error opening ");
    Serial.println(String(char_fich));
  }
}

void arranco_reloj (){
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


}


float leer_voltio ()
{

   float voltage = 0.0;
   float v = (analogRead(PORT_VOL) * v1) / 1024.0;
   voltage = v / (r2 / (r1 + r2));
   return(voltage);
}

float leer_amperio ()
{
   float voltage=0;
   float corrienteSum = 0;
   for (int i = 0; i < samplesNumber; i++)
   {
      voltage = analogRead(PORT_AMP) * 5.0 / 1023.0;
      corrienteSum += (voltage - 2.5) / SENSIBILITY;
   }
   return(corrienteSum / samplesNumber);
}
// initialize the library with the numbers of the interface pins
void imprime() {
  float t1=0, t2=0 , t3=0, amp=0, vol=0;
  String dataString;
  DateTime now = rtc.now();
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  vol= leer_voltio ();
  amp= leer_amperio ();
  t1 = dht.readTemperature();
  sensors.requestTemperatures();
  //if (isnan(t1)) {
  //    return;
  //}
  t2=sensors.getTempC(insideThermometer);
  t3=sensors.getTempC(outsideThermometer);
  dataString = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + " " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) ;
  dataString = dataString + ";" + String(amp);
  dataString = dataString + ";" + String(vol);
  dataString = dataString + ";" + String(t1);
  dataString = dataString + ";" + String(t2);
  dataString = dataString + ";" + String(t3);
  Serial.println(dataString);
  escribe_SD(dataString);
 }

void setup() {
  // set up the LCD's number of columns and rows:
  Serial.begin(9600);
  dht.begin();
  sensors.begin();
  sensors.setResolution(insideThermometer, 10);
  sensors.setResolution(outsideThermometer, 10);
  arranco_reloj();
  incializa_SD();
}

void loop() {

  imprime();
  delay (1000);
 }

 