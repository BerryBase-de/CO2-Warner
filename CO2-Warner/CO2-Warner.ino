
//-----------------------------------------------------
// CO2-Warner with Adafruit SCD30 and BME280 sensor
// displayed on an Adaruit 1.69" 280x240 IPS TFT
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 18. Nov 2021
// Update:  23. Nov 2021
//-----------------------------------------------------
//Include Libs
#include <Adafruit_BME280.h>
#include <Adafruit_SCD30.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "bitmap.h"   //This is a file in the same dir from this program(!!!)

//Some definitions
#define NOOFARRAY   15
#define NOOFENTRIES 60
#define UPDATETIME  60000
#define TFT_CS      0
#define TFT_RST     16                                            
#define TFT_DC      2
//Define some colors
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_YELLOW  0xFFE0
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
//Define CO2
#define CO2OK 1000
#define CO2NOK 2000
//Define map limits
#define MinMapValue 10
#define MaxMapValue 65
//Create needed objects
Adafruit_BME280 bme; // use I2C interface
Adafruit_SCD30  scd30;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//Some global vars
bool bFirstRun = true;
double dCurrentTemp = 0.0;
float fCurrentHum = 0.0;
float fCurrentPres = 0.0;
int iCO2Array[NOOFARRAY];  //Array for temporal course CO2
unsigned long iLastUpate = 0;

/*
  =================================================================
  Function:     setup
  Returns:      void
  Description:  Setup display and sensors
  =================================================================
*/
void setup() {
  Serial.begin(9600);
  while (!Serial) delay(20);    // Wait until serial communication is ready
  Serial.println("CO2-Warner");
  Serial.println("(c) Joern Weise");
  Serial.println("For berrybase.de");
  tft.init(240, 280);           // Init ST7789 280x240
  tft.fillScreen(COLOR_BLACK);  // Clear screen
  tft.setRotation(3);           // Set rotation to landscape
  //Start with some text in serial monitor and tft
  tft.setTextSize(3);
  tft.setTextColor(COLOR_RED);
  tft.setCursor(80, 20);
  tft.print("CO2");
  tft.setTextColor(COLOR_GREEN);
  tft.print("-Ampel");
  tft.setTextSize(2);
  tft.setTextColor(COLOR_YELLOW);
  tft.setCursor(65, 50);
  tft.print("(c) Joern Weise");
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(25, 80);
  tft.print("Init display ... ");
  tft.setTextColor(COLOR_GREEN);
  tft.println("DONE");
  tft.setCursor(25, 110);
  tft.setTextColor(COLOR_WHITE);
  Serial.print("Init BME     ... ");
  tft.print("Init BME     ... ");
  //Init BME280
  //if(!bme.begin()) //Only use if BME is failing
  if(!bme.begin(0x76, &Wire))
  {
    Serial.println("FAILED");
    Serial.println("Could not find a valid BME280 sensor, check wiring or change address!");
    tft.setTextColor(COLOR_RED);
    tft.println("FAIL");
    while (1); //Endless loop
  }
  tft.setTextColor(COLOR_GREEN);
  tft.println("DONE");
  Serial.println("DONE");
  tft.setCursor(25, 140);
  tft.setTextColor(COLOR_WHITE);
  Serial.print("Init SCD30   ... ");
  tft.print("Init SCD30   ... ");
  //Init SCD30-sensor
  if (!scd30.begin())
  {
    Serial.println("FAILED");
    Serial.println("Failed to find SCD30 chip");
    tft.setTextColor(COLOR_RED);
    tft.println("FAIL");
    while (1);
  }
  tft.setTextColor(COLOR_GREEN);
  tft.println("DONE");
  Serial.println("DONE");
  tft.setCursor(25, 170);
  tft.setTextColor(COLOR_WHITE);
  Serial.print("Clear Array  ... ");
  tft.print("Clear Array  ... ");
  //Clear array from maybe random entries
  for (int iCnt = 0; iCnt < NOOFARRAY; iCnt++)
    iCO2Array[iCnt] = 0;
  tft.setTextColor(COLOR_GREEN);
  tft.println("DONE");
  Serial.println("DONE");
  tft.setCursor(25, 200);
  tft.setTextColor(COLOR_WHITE);
  Serial.println("Wait 10 sec to start");
  tft.print("Wait 10 sec to start");
  delay(10000);
}

void loop() {
  //Just want data every 60sec or during first start
  if (millis() - iLastUpate > UPDATETIME || bFirstRun)
  {
    UpdateIndoorClimate();  // Receive data from BME280
    if(scd30.dataReady())   // Check if SCD30-data are available
    {
      Serial.println("New data from SCD30 available...");
      if(!scd30.read())
      {
        Serial.println("Error reading sensor data");
        iLastUpate += 1000; //Try after 1 sec again
      }
      else
      {
        UpdateArray(int(scd30.CO2));  // Update array with CO2-data
        if(bFirstRun)
          bFirstRun = false;
        WriteDataToSerialMonitor(); // Write data to serial monitor
        UpdateDisplayInfo();        // Update display
        iLastUpate = millis();      // Update time to get data after 60sec
      }
    }
    else  // if(scd30.dataReady()) 
    {
      Serial.println("No data from SCD30");
      iLastUpate = iLastUpate + 1000; //Try after 1 sec again
    }
  }
}

/*
  =================================================================
  Function:     UpdateIndoorClimate
  Returns:      void
  Description:  Setup display and sensors
  =================================================================
*/
void UpdateIndoorClimate() {
  dCurrentTemp = bme.readTemperature();
  fCurrentHum = bme.readHumidity();
  fCurrentPres = bme.readPressure() / 100.0F;
}

/*
  =================================================================
  Function:     WriteDataToSerialMonitor
  Returns:      void
  Description:  Setup display and sensors
  =================================================================
*/
void WriteDataToSerialMonitor() {
  Serial.print("Temperature = ");
  Serial.print(dCurrentTemp);
  Serial.println(" °C");

  Serial.print("Pressure = ");

  Serial.print(fCurrentPres);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(fCurrentHum);
  Serial.println(" %");

  Serial.print("CO2 = ");
  Serial.print(iCO2Array[NOOFARRAY - 1]);
  Serial.println(" ppm");
  Serial.println("-------------------------");
}

/*
* =================================================================
* Function:     UpdateArrays   
* Returns:      void
* Description:  Update data arrays with needed info for graphic
* =================================================================
*/
void UpdateArray(int iCO2)
{
  //Swap data array for the last minutes
  for (int iCnt = 0; iCnt < (NOOFARRAY - 1); iCnt++)
    iCO2Array[iCnt] = iCO2Array[iCnt + 1];
  iCO2Array[NOOFARRAY - 1] = iCO2;
  Serial.println("Current values in array:");
  for (int iCnt = 0; iCnt < NOOFARRAY; iCnt++)
  {
    if( iCnt != 0)
      Serial.print(" | ");
    Serial.print(iCO2Array[iCnt]);
  }
  Serial.println("");
  
}

/*
* =================================================================
* Function:     UpdateDisplayInfo   
* Returns:      void
* Description:  Update display content with latest data
* =================================================================
*/
void UpdateDisplayInfo()
{
  tft.fillScreen(COLOR_BLACK);
  tft.drawLine(40, 220, 40, 140, COLOR_WHITE);  // Y-axis
  tft.drawLine(40, 220, 240, 220, COLOR_WHITE); // X-Axis

  //Arrows Y-Axis
  tft.drawLine(40,140,35,150,COLOR_WHITE);
  tft.drawLine(40,140,45,150,COLOR_WHITE);
  //Arrows X-Axis
  tft.drawLine(230,215,240,220,COLOR_WHITE);
  tft.drawLine(230,225,240,220,COLOR_WHITE);

  //Get highest PPM to calculate max for graph
  int iHighestPPM = 0;
  int iHighestMap = 0;
  for(int iCnt = 0; iCnt < NOOFARRAY; iCnt++)
  {
    if(iHighestPPM < iCO2Array[iCnt])
    {
       iHighestPPM = iCO2Array[iCnt];
       Serial.println("Highest PPM: " + String(iCO2Array[iCnt]) + " ppm");
       iHighestMap = int(iHighestPPM / 100) * 100 + 200;
       Serial.println("Calc highest map: " + String(iHighestMap));
    }
  }

  //Go through array, starting at the end
  int iNoValues = 0;
  int iSumArray = 0;
  for (int iCnt = NOOFARRAY-1; iCnt >=0; iCnt--)
  {
    if(iCO2Array[iCnt] != 0)
    {
      iNoValues++;
      iSumArray += iCO2Array[iCnt];
      //Correction and mapping value for graph
      int iMappedValue = map(iCO2Array[iCnt], 400, iHighestMap, MinMapValue, MaxMapValue);
      //Find correct graph-color
      uint32_t color;
      if(iCO2Array[iCnt] < CO2OK)
        color = COLOR_GREEN;
      else if(iCO2Array[iCnt] >= CO2OK && iCO2Array[iCnt] < CO2NOK)
        color = COLOR_YELLOW;
      else
        color = COLOR_RED;
      //Draw the graph-segments for each item from array
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 2, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 2, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 3, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 3, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 4, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 4, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 5, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 5, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 6, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 6, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 7, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 7, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 8, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 8, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 9, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 9, 219, color);
      tft.drawLine(40 + (NOOFARRAY-1-iCnt) * 10 + 10, 219-iMappedValue, 40 + (NOOFARRAY-1-iCnt) * 10 + 10, 219, color);
    }
  }

  //Draw Average
  int iAverage = int((iSumArray / iNoValues) / 100) * 100;
  if((iAverage % 100) >= 50)
    iAverage+=50;
  int iMappedValue = map(iAverage, 400, iHighestMap, MinMapValue, MaxMapValue);
  tft.drawLine(40, 219-iMappedValue, 220, 219-iMappedValue, COLOR_BLUE);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_BLUE);
  tft.setCursor(200, 219-iMappedValue-10);
  tft.print(iAverage);
  
  //Write latest data to display
  tft.setTextSize(2);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(30, 20);
  tft.print("Temp: " + String(int(dCurrentTemp)) + " C");
  tft.setCursor(30, 40);
  tft.print("Humi: " + String(int(fCurrentHum)) + " %");
  tft.setCursor(30, 60);
  tft.print("CO2:  ");
  if(iCO2Array[NOOFARRAY - 1] < CO2OK)
    tft.setTextColor(COLOR_GREEN);
  else if(iCO2Array[NOOFARRAY - 1] >= CO2OK && iCO2Array[NOOFARRAY - 1] < CO2NOK)
    tft.setTextColor(COLOR_YELLOW);
  else
    tft.setTextColor(COLOR_RED);
  tft.print(String(iCO2Array[NOOFARRAY - 1]));
  tft.setTextColor(COLOR_WHITE);
  tft.println(" ppm");
  tft.setCursor(30, 80);
  tft.print("Frischluft");
  tft.setCursor(30, 100);
  if(iCO2Array[NOOFARRAY - 1] < CO2OK)
  {
    tft.setTextColor(COLOR_GREEN);
    tft.print("nicht notwendig!");
    tft.drawBitmap(220,30,okay,60,60,COLOR_GREEN);
  }
  else if(iCO2Array[NOOFARRAY - 1] >= CO2OK && iCO2Array[NOOFARRAY - 1] < CO2NOK)
  {
    tft.setTextColor(COLOR_YELLOW);
    tft.print("ist sinnvoll!");
    tft.drawBitmap(220,30,warning,60,60,COLOR_YELLOW);
  }
  else
  {
    tft.setTextColor(COLOR_RED);
    tft.print("sofort notwendig!");
    tft.drawBitmap(220,30,error,60,60,COLOR_RED);
  } 
}
