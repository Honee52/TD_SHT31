/**
* @file TD_SHT31_test.ino
* @brief
* This simple code show how to use TD_SHT31 library to read temperature,
* and humidity from SHT31 sensor.

* Interface:
* Sensor         Arduino Uno Board
* --------------------------------
* Vin (3.3V)      3.3V
* Gnd             Gnd
* SDA             A4
* SCK             A5
* --------------------------------
*
* Written by Honee52.
 */

#include <TD_SHT31.h>

/**
 * ----------------------------------------------------------------------------
 * Define SHT31 and variables.
 * ----------------------------------------------------------------------------
 */
TD_SHT31 sht(0x44);
char str_humidity[8];
char str_temperature[8];
float temperat_o, humidity_o;
uint8_t retval;

/**
 * ----------------------------------------------------------------------------
 * Setup
 * ----------------------------------------------------------------------------
*/
void setup() {
	/* Initialize serial port */
	Serial.begin(9600);
	while (!Serial) {
		; // Wait for serial port. Remove wait if not native USB port.
	}
  delay(1000);
  Serial.println(" ");

  /**
    * ----------------------------------------------------
    * SHT31 setup
    * ----------------------------------------------------
  */  
  sht.set_defaults(ENABLE_CRC, CELSIUS);

  if (sht.isSensorConnected() == false)
  {
    Serial.println("Sensor is not connected! Check wiring!");
    while (true) { ; }
  }

  if (sht.begin() == false)
  {
    Serial.print("Error in begin(): 0b"); 
    Serial.println(sht.getLastError(), BIN);
    while (true) { ; }
  }

  uint16_t u16Status = sht.readSensorStatus();
  if (u16Status == 0xFFFF)
  {
    Serial.println("readSensorStatus failed");  
    while (true) { ; }  
  }
  Serial.print("Sensor status: 0x"); 
  Serial.println(u16Status, HEX);

  if (sht.clearSensorStatus() == false)
  {
    Serial.println("clearSensorStatus failed");
    while (true) { ; }
  }
  Serial.println(" ");

}

/**
 * ----------------------------------------------------------------------------
 * Main loop.
 * ----------------------------------------------------------------------------
*/
void loop() {
  if (sht.isSensorConnected())
  {
    if (sht.runSingleShot(CMD_SS_CSD_LOW, &temperat_o, &humidity_o))
    {
      dtostrf(temperat_o,  4, 2, str_temperature);
      dtostrf(humidity_o,  4, 2, str_humidity);
    } else
    {
      Serial.println("Error in readSingleShot");
      Serial.print("Error: 0x"); 
      Serial.println(sht.getLastError(), HEX);
    }             
  } else
  {
    Serial.println("Error in isSensorConnected");
    Serial.print("Error: 0x");
    Serial.println(sht.getLastError(), HEX);     
  }

  Serial.print("Temperature: ");
  Serial.println(str_temperature);
  Serial.print("Humidity:    ");
  Serial.println(str_humidity);
  Serial.println(" ");

  delay(5000);
}
