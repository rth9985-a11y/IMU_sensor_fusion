#include <Wire.h>
#include "bmi270.h"

#define BMI270_I2C_ADDR 0x68
/*Can also be replaced with "contexpr byte BMI270_I2C_ADDR = 0x68" macros outdated for this use case???*/

/*Hardware Abstraction Layer*/

int8_t bmi2_i2c_read(uint8_t reg, uint8_t *data, uint32_t len, void *intf_ptr)
{
  /*Prep to send data to sensor at 0x68*/
  Wire2.beginTransmission(BMI270_I2C_ADDR);
  /*Tell sensor what register to read (eg. gyro x, accel y, mag y, etc...)*/
  Wire2.write(reg);
  /*FALSE = Doesn't release I2C bus but restarts transmission
  if the error message isn't 0 (ACK) transmission failed*/
  if (Wire2.endTransmission(false) != 0)
    return BMI2_E_COM_FAIL;

  /*Set length of byte so send from address*/
  Wire2.requestFrom((uint8_t)BMI270_I2C_ADDR, (uint8_t)len);
  /*Loop over data stream in length of bytes*/
  for (uint32_t i = 0; i < len && Wire2.available(); i++)
  {
    data[i] = Wire2.read();
  }
  /*return 0 if everything works*/
  return BMI2_OK;
}

int8_t bmi2_i2c_write(uint8_t reg, const uint8_t *data, uint32_t len, void *intf_ptr)
{
  Wire2.beginTransmission(BMI270_I2C_ADDR);
  Wire2.write(reg);
  for (uint32_t i = 0; i < len; i++)
  {
    Wire2.write(data[i]);
  }
  return (Wire2.endTransmission() == 0) ? BMI2_OK : BMI2_E_COM_FAIL;
}

void bmi2_delay_us(uint32_t period, void *intf_ptr)
{
  delayMicroseconds(period);
}

/*Bosch objects*/
struct bmi2_dev bmi;
struct bmi2_sens_data sensor_data;

/*Teensy Init*/
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("t_us,ax,ay,az,gx,gy,gz");

  Wire2.begin();
  Wire2.setClock(400000);

  bmi.intf = BMI2_I2C_INTF;
  bmi.read = bmi2_i2c_read;
  bmi.write = bmi2_i2c_write;
  bmi.delay_us = bmi2_delay_us;
  bmi.read_write_len = 32;

  if (bmi270_init(&bmi) != BMI2_OK)
  {
    Serial.println("BMI270 init failed");
    while (1)
      ;
  }

  uint8_t sens_list[2] = {BMI2_ACCEL, BMI2_GYRO};
  bmi270_sensor_enable(sens_list, 2, &bmi);
}

/*Get Data*/
void loop()
{
  if (bmi2_get_sensor_data(&sensor_data, &bmi) == BMI2_OK)
  {
    Serial.print(sensor_data.acc.x);
    Serial.print(",");
    Serial.print(sensor_data.acc.y);
    Serial.print(",");
    Serial.print(sensor_data.acc.z);
    Serial.print(",");
    Serial.print(sensor_data.gyr.x);
    Serial.print(",");
    Serial.print(sensor_data.gyr.y);
    Serial.print(",");
    Serial.println(sensor_data.gyr.z);
  }

  delay(1); // ~200 Hz
}
