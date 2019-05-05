/***********************************************************
librairie pour BMP280 ** inspiree de la librairie Adafruit
 ***********************************************************/
#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include "jtBMP280.h"


/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/


jt_BMP280::jt_BMP280()
  : _cs(-1), _mosi(-1), _miso(-1), _sck(-1)
{ }

jt_BMP280::jt_BMP280(int8_t cspin)
  : _cs(cspin), _mosi(-1), _miso(-1), _sck(-1)
{ }

jt_BMP280::jt_BMP280(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin)
  : _cs(cspin), _mosi(mosipin), _miso(misopin), _sck(sckpin)
{ }


bool jt_BMP280::begin(uint8_t a) //I2c seulement
{
  _I2cad = a;
    Wire.begin();
  if (read8(BMP280_REGISTER_CHIPID) != 0x58) return false;
  coef();
  write8(BMP280_REGISTER_CONTROL, 0x3F);
  return true;
}
/**************************************************************************/
/*!
    @brief  Writes an 8 bit value over I2C/SPI
*/
/**************************************************************************/
void jt_BMP280::write8(byte reg, byte value)
{
    Wire.beginTransmission((uint8_t)_I2cad);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)value);
    Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t jt_BMP280::read8(byte reg)
{
  uint8_t value;
    Wire.beginTransmission((uint8_t)_I2cad);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_I2cad, (byte)1);
    value = Wire.read();
    return value;
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit value over I2C
*/
/**************************************************************************/
uint16_t jt_BMP280::read16(byte reg)
{
  uint16_t value;
    Wire.beginTransmission((uint8_t)_I2cad);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_I2cad, (byte)2);
    value = (Wire.read() << 8) | Wire.read();
    return value;
}

uint16_t jt_BMP280::read16_LE(byte reg) {
  uint16_t temp = read16(reg);
  return (temp >> 8) | (temp << 8);

}

/**************************************************************************/
/*!
    @brief  Reads a signed 16 bit value over I2C
*/
/**************************************************************************/
int16_t jt_BMP280::readS16(byte reg)
{
  return (int16_t)read16(reg);

}

int16_t jt_BMP280::readS16_LE(byte reg)
{
  return (int16_t)read16_LE(reg);

}


/**************************************************************************/
/*!
    @brief  Reads a signed 16 bit value over I2C
*/
/**************************************************************************/

uint32_t jt_BMP280::read24(byte reg)
{
  uint32_t valeur;
    Wire.beginTransmission((uint8_t)_I2cad);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_I2cad, (byte)3);
    valeur = Wire.read();
    valeur <<= 8;
    valeur |= Wire.read();
    valeur <<= 8;
    valeur |= Wire.read();
    return valeur;
}

/**************************************************************************/
/*!
    lecture coefficients d'usine
*/
/**************************************************************************/
void jt_BMP280::coef(void)
{
    _bmp280_calib.dig_T1 = read16_LE(BMP280_REGISTER_DIG_T1);
    _bmp280_calib.dig_T2 = readS16_LE(BMP280_REGISTER_DIG_T2);
    _bmp280_calib.dig_T3 = readS16_LE(BMP280_REGISTER_DIG_T3);

    _bmp280_calib.dig_P1 = read16_LE(BMP280_REGISTER_DIG_P1);
    _bmp280_calib.dig_P2 = readS16_LE(BMP280_REGISTER_DIG_P2);
    _bmp280_calib.dig_P3 = readS16_LE(BMP280_REGISTER_DIG_P3);
    _bmp280_calib.dig_P4 = readS16_LE(BMP280_REGISTER_DIG_P4);
    _bmp280_calib.dig_P5 = readS16_LE(BMP280_REGISTER_DIG_P5);
    _bmp280_calib.dig_P6 = readS16_LE(BMP280_REGISTER_DIG_P6);
    _bmp280_calib.dig_P7 = readS16_LE(BMP280_REGISTER_DIG_P7);
    _bmp280_calib.dig_P8 = readS16_LE(BMP280_REGISTER_DIG_P8);
    _bmp280_calib.dig_P9 = readS16_LE(BMP280_REGISTER_DIG_P9);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
float jt_BMP280::liretemp(void)
{
  int32_t var1, var2;

  int32_t adc_T = read24(BMP280_REGISTER_TEMPDATA);
  adc_T >>= 4;

  var1  = ((((adc_T>>3) - ((int32_t)_bmp280_calib.dig_T1 <<1))) *
	  ((int32_t)_bmp280_calib.dig_T2)) >> 11;

  var2  = (((((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1)) *
	    ((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1))) >> 12) *
	 ((int32_t)_bmp280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;

  float T  = (t_fine * 5 + 128) >> 8;
  return T/100;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
float jt_BMP280::lirepression(void) {
  int64_t var1, var2, p;

  liretemp();
  int32_t adc_P = read24(BMP280_REGISTER_PRESSUREDATA);
  adc_P >>= 4;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)_bmp280_calib.dig_P6;
  var2 = var2 + ((var1*(int64_t)_bmp280_calib.dig_P5)<<17);
  var2 = var2 + (((int64_t)_bmp280_calib.dig_P4)<<35);
  var1 = ((var1 * var1 * (int64_t)_bmp280_calib.dig_P3)>>8) +
    ((var1 * (int64_t)_bmp280_calib.dig_P2)<<12);
  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)_bmp280_calib.dig_P1)>>33;

  if (var1 == 0)    return 0;  // pas de division par 0
  p = 1048576 - adc_P;
  p = (((p<<31) - var2)*3125) / var1;
  var1 = (((int64_t)_bmp280_calib.dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((int64_t)_bmp280_calib.dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)_bmp280_calib.dig_P7)<<4);
  return (float)p/256;
}

