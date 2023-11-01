/**
 * ----------------------------------------------------------------------------
 * @file TD_SHT31.h
 * @brief Arduino I2C library for SENSIRION SHT31 sensor (temperature & humidity).
 * @details Written by Honee52 for Technode Design (info@technode.fi).
 * You may use this library as it is or change it without limitations. 
 * Current version supports only sigle shot commands without stretching.
 * Beerware license.
 * @version 1.0.0
 * @note 'Simple is beatiful'
 * @todo Single shot commands with stretching and periodic commands.
 * Version history:
 * Version 1.0.0    Initial version
 * ----------------------------------------------------------------------------
*/
#ifndef TD_SHT31_H
#define TD_SHT31_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Wire.h"
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define TD_SHT31_VERSION "1.0.0"

/**
 * @brief Commands.
*/

/** @brief Single shot (SS) commands, clock stretching enabled (CSE). */
/** @note: TwoWire class does not support stretching. */
#define CMD_SS_CSE_HIGH     0x2C06
#define CMD_SS_CSE_MEDIUM   0x2C0D
#define CMD_SS_CSE_LOW      0x2C10

/** @brief Single shot (SS) commands, clock stretching disabled (CSD). */
#define CMD_SS_CSD_HIGH     0x2400
#define CMD_SS_CSD_MEDIUM   0x240B
#define CMD_SS_CSD_LOW      0x2416

/** @brief Periodic commands 0.5/12/4/10 mps. */
#define CMD_PER_05_HIGH     0x2032
#define CMD_PER_05_MEDIUM   0x2024
#define CMD_PER_05_LOW      0x202F

#define CMD_PER_1_HIGH      0x2130
#define CMD_PER_1_MEDIUM    0x2126
#define CMD_PER_1_LOW       0x212D

#define CMD_PER_2_HIGH      0x2236
#define CMD_PER_2_MEDIUM    0x2220
#define CMD_PER_2_LOW       0x222B

#define CMD_PER_4_HIGH      0x2334
#define CMD_PER_4_MEDIUM    0x2322
#define CMD_PER_4_LOW       0x2329

#define CMD_PER_10_HIGH     0x2737
#define CMD_PER_10_MEDIUM   0x2721
#define CMD_PER_10_LOW      0x272A

/** @brief Fetch Data.  */
#define CMD_PER_FETCH_DATA  0xE000

/** @brief ART (accelerated  response  time). */
#define CMD_PER_ART         0x2B32

/** @brief Break. */
#define CMD_PER_BREAK       0x3093

/** @brief Soft reset & General call reset. */
#define CMD_SOFT_RESET      0x30A2
#define CMD_GCALL_RESET     0x0006

/* Heater enable/diable. */
#define CMD_HEATER_ON       0x306D
#define CMD_HEATER_OFF      0x3066

/** @brief Status register. */
#define CMD_READ_STATUS     0xF32D
#define CMD_CLEAR_STATUS    0x3041

/**
 * @brief Some definitions.
 * @details Used in function set_defaults.
*/
#define ENABLE_CRC          true
#define DISABLE_CRC         false
#define CELSIUS             true
#define FARENHEIT           false

/**
 * @brief Error codes & error masks.
*/
#define NO_ERROR                    0b0000000000000000
#define ERROR_TRANSMISSION_LEN      0b0000000000000001
#define ERROR_END_TRANSMISSION      0b0000000000000010
#define ERROR_REQUEST_LEN           0b0000000000000100
#define ERROR_WRITE_LEN             0b0000000000001000
#define ERROR_WRONG_SENSOR_ID       0b0000000000010000
#define ERROR_FM_TIMEOUT            0b0000000000100000
#define ERROR_NOT_CONNECTED         0b0000000001000000
#define ERROR_CRC_CHECK             0b0000000010000000
#define ERROR_WRONG_COMMAND         0b0000000100000001

/**
 * @class TD_SHT31.
 * @brief TD_SHT31 Class definition.
*/
class TD_SHT31
{
    public:
    /**
     * @brief TD_SHT31 Class forward declaration.
     * @param[in] I2C address of the SHT31 device
    */
    TD_SHT31(uint8_t i2c_device_address);

    /**
     * @brief Function begin.
     * @return boolean result
    */
    bool begin();

    /**
     * @brief Function begin.
     * @param *wire
     * @return boolean result
    */    
    bool begin(TwoWire *wire);

    /**
     * @brief Check if sensor is connected.
     * @return boolean result
    */
    bool isSensorConnected();   

    /**
     * @brief Set enable/disable crc and temperature unit. 
     * @param useCRC (ENABLE_CRC or DISABLE_CRC)
     * @param tUnit temperature unit (CELSIUS or FARENHEITH)
     * @return void
    */
    void set_defaults(bool useCRC, bool tUnit); 

     /**
     * @brief Set enable/disable crc, temperature unit, SDA-pin and SLC-pin. 
     * @param useCRC (ENABLE_CRC or DISABLE_CRC)
     * @param tUnit temperature unit (CELSIUS or FARENHEITH)
     * @param dataPIN I2C SDA-pin (only ESP8266 or ESP32)
     * @param clockPIN I2C SCL-pin (only ESP8266 or ESP32)
     * @return void
    */   
    void set_defaults(bool useCRC, bool tUnit, uint8_t dataPIN, uint8_t clockPIN);     

    /**
     * @brief Reset sensor
     * @param command
     * @return boolean result
     * @note Only commands CMD_SOFT_RESET or CMD_GCALL_RESET are allowed.
    */
    bool resetSensor(uint16_t command);

    /**
     * @brief Execute single shot measurement.
     * @param u16Command
     * @param *fT [out] float *temperature
     * @param *fH [out] float *humidity
     * @return boolean result
    */    
    bool runSingleShot(uint16_t u16Command, float *fT, float *fH);

    /**
     * @brief Clear sensor status.
     * @param void
     * @return boolean result
    */
    bool clearSensorStatus();

    /**
     * @brief Read sensor status.
     * @param void
     * @return Sensor status
     * @note If function fails to read status, value 0xFFFF is returned.
    */
    uint16_t readSensorStatus();    
    
    /**
     * @brief Return last error.
     * @param void
     * @return error code (_error_code)
     * @note When reading _error_code is cleared.
    */
    int getLastError();

    /**
     * @brief TD_SHT31 Class private declarations.
    */
    private:  
    TwoWire* _i2c;
    uint8_t _sdaPIN;
    uint8_t _slcPIN;
    uint8_t _i2c_device_address;
    int _error_code;
    float _humidity;
    float _temperature;
    bool _useCRC = ENABLE_CRC;
    bool _tUnit = CELSIUS;   

    /**
     * @brief Read bytes to buffer.
     * @param *buffer [out] data buffer
     * @param data length (len)
     * @return boolean result
    */
    bool readBytes(uint8_t *buffer, uint8_t len);

    /**
     * @brief Read sensor data into _temperature and _humidity.
     * @param void
     * @return boolean result
    */    
    bool readSensorData();

    /**
     * @brief Write command to sensor.
     * @param command
     * @return boolean result
    */
    bool writeCommand(uint16_t command);

    /**
     * @brief Calculate checksum.
     * @param *data [in] data buffer
     * @param len data length (len)
     * @return CRC (uint8_t)
    */
    uint8_t crc8(const uint8_t *data, uint8_t len);
};

#endif  //TD_SHT31_H
