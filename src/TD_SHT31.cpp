/**
 * ----------------------------------------------------------------------------
 * @file TD_SHT31.cpp
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

#include "TD_SHT31.h"

/**
 * ----------------------------------------------------------------------------
 * @brief Initialize TD_SHT31 Class.
 * ----------------------------------------------------------------------------
*/
TD_SHT31::TD_SHT31(uint8_t i2c_device_address)
{
    _i2c_device_address = i2c_device_address;
    _useCRC     = ENABLE_CRC;
    _tUnit      = CELSIUS;      
    _error_code = NO_ERROR;
}

/**
 * ----------------------------------------------------------------------------
 * @brief Functions bool begin().
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::begin()
{
     return begin(&Wire);
}

bool TD_SHT31::begin(TwoWire *wire)
{
    _i2c = wire;
    _i2c->begin();
    _i2c->setClock(100000); // 100kHz
    return resetSensor(CMD_GCALL_RESET);
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function bool isSensorConnected().
 * @details Check if sensor is connected.
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::isSensorConnected()
{
    _i2c->beginTransmission(_i2c_device_address);
    int retval = _i2c->endTransmission();
    if (retval != 0)
    { 
        _error_code |= ERROR_END_TRANSMISSION;
        return false;
    }
    return true;
}

/**
 * ----------------------------------------------------------------------------
 * @brief Functions void set_defaults(bool useCRC, bool tUnit).
 * ----------------------------------------------------------------------------
*/
void TD_SHT31::set_defaults(bool useCRC, bool tUnit)
{
    _useCRC = useCRC;
    _tUnit = tUnit;
}

void TD_SHT31::set_defaults(bool useCRC, bool tUnit, uint8_t dataPIN, uint8_t clockPIN)
{
    _useCRC = useCRC;
    _tUnit = tUnit;   
    #if defined(ESP8266) || defined(ESP32)
    Wire.setPins(dataPIN,clockPIN);
    #endif
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function bool resetSensor(uint16_t command).
 * @details Allowed commands CMD_SOFT_RESET or CMD_GCALL_RESET.
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::resetSensor(uint16_t command)
{
    byte buffer[2];
    buffer[0] = command >> 8;
    buffer[1] = command & 0xFF;
    //    
    if ((command != CMD_SOFT_RESET) && \
        (command != CMD_GCALL_RESET))
    {
        _error_code |= ERROR_WRONG_COMMAND;
        return false;
    }
    //
    _i2c->beginTransmission(_i2c_device_address);
    if (_i2c->write(buffer, 2) != 0x02)
    {
        _error_code |= ERROR_WRITE_LEN;
    }
    _i2c->endTransmission();    /* Dummy call */
    if (_i2c->endTransmission() != 0)
    {
        _error_code |= ERROR_END_TRANSMISSION;
        return false;
    }
    return true;    
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function bool runSingleShot(uint16_t u16Command, float *fT, float *fH).
 * @details Calls runSingleShot(u16Command).
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::runSingleShot(uint16_t u16Command, float *fT, float *fH)
{
    uint8_t u8Delay;

    if ((u16Command != CMD_SS_CSD_HIGH) && \
        (u16Command != CMD_SS_CSD_MEDIUM) && \
        (u16Command != CMD_SS_CSD_LOW))
    {
        _error_code |= ERROR_WRONG_COMMAND;
        return false;        
    }

    if (writeCommand(u16Command) == false)
    {
        return false;
    }
    switch (u16Command)
    {
        /* Refer datasheet page 7 */
        case CMD_SS_CSD_HIGH:   { u8Delay = 16; break; }
        case CMD_SS_CSD_MEDIUM: { u8Delay = 7;  break; }
        case CMD_SS_CSD_LOW:    { u8Delay = 5;  break; }
        default:                { u8Delay = 16;}
    }  
    delay(u8Delay);

    if(readSensorData())
    {
        *fT = _temperature;
        *fH = _humidity;
        return true;        
    }
    return false;
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function bool clearSensorStatus().
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::clearSensorStatus()
{   
    if (writeCommand(CMD_CLEAR_STATUS) == false)
    {
        return 0xFFFF;
    }
    return true;
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function uint16_t readSensorStatus().
 * ----------------------------------------------------------------------------
*/
uint16_t TD_SHT31::readSensorStatus()
{
    uint8_t buffer[3] = { 0, 0, 0 };
    
    if (writeCommand(CMD_READ_STATUS) == false)
    {
        return 0xFFFF;
    }
    
    /* Read status bytes */
    if (readBytes((uint8_t*) &buffer[0], 3) == false)
    {
        return 0xFFFF;
    }

    /* CRC  */
    if (buffer[2] != crc8(buffer, 2)) 
    {
        _error_code |= ERROR_CRC_CHECK;
        return 0xFFFF;
    }

    return (uint16_t) (buffer[0] << 8) + buffer[1];
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function int getLastError().
 * ----------------------------------------------------------------------------
*/
int TD_SHT31::getLastError()
{
  int retval = _error_code;
  _error_code = NO_ERROR;
  return retval;
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function bool readBytes(uint8_t *buffer, uint8_t len).
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::readBytes(uint8_t *buffer, uint8_t len)
{
    int retval = _i2c->requestFrom(_i2c_device_address, (uint8_t) len);
    if (retval == len)
    {
        for (uint8_t i = 0; i < len; i++)
        {
            buffer[i] = _i2c->read();
        }
        return true;
    }
    _error_code |= ERROR_REQUEST_LEN;
    return false;
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function bool readSensorData().
 * @details
 * - Read from sendor.
 * - Make CRC-check if enabled.
 * - Calculate temperature and humidity values and save to _temperature and
 * - _humidity.
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::readSensorData()
{
    uint8_t buffer[6];
    if (readBytes((uint8_t*) &buffer[0], 6) == false)
    {
        return false;
    }

    if (_useCRC)
    {
        if (buffer[2] != crc8(buffer, 2)) 
        {
            _error_code |= ERROR_CRC_CHECK;
            return false;
        }
        if (buffer[5] != crc8(buffer + 3, 2)) 
        {
            _error_code |= ERROR_CRC_CHECK;
            return false;
        }
    }

    /* Conversion formulas - refer datasheet page 14 */
     uint16_t data = (buffer[0] << 8) + buffer[1];
    if (_tUnit)
    {
        _temperature = data * (175.0 / 65535) - 45; // Celsius
    } else
    {
        _temperature = data * (315.0 / 65535) - 49; // Farenheit
    }
    data = (buffer[3] << 8) + buffer[4];
    _humidity = data * (100.0 / 65535);

    return true;
}

/**
 * ----------------------------------------------------------------------------
 *  @brief Function bool writeCommand(uint16_t command).
 * ----------------------------------------------------------------------------
*/
bool TD_SHT31::writeCommand(uint16_t command)
{
    byte buffer[2];
    buffer[0] = command >> 8;
    buffer[1] = command & 0xFF;
    _i2c->beginTransmission(_i2c_device_address);
    if (_i2c->write(buffer, 2) != 0x02)
    {
        _error_code |= ERROR_WRITE_LEN;
    }
    if (_i2c->endTransmission() != 0)
    {
        _error_code |= ERROR_END_TRANSMISSION;
        return false;
    }
    return true;
}

/**
 * ----------------------------------------------------------------------------
 * @brief Function uint8_t crc8(const uint8_t *data, uint8_t len).
 * @details Calculate CRC - refer datasheet page 14.
 * ----------------------------------------------------------------------------
*/
uint8_t TD_SHT31::crc8(const uint8_t *data, uint8_t len) 
{
    const uint8_t POLY(0x31);
    uint8_t crc(0xFF);

    for (uint8_t j = len; j; --j) 
    {
        crc ^= *data++;
        for (uint8_t i = 8; i; --i) 
        {
            crc = (crc & 0x80) ? (crc << 1) ^ POLY : (crc << 1);
        }
    }
    return crc;
}
