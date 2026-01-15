#include <Wire.h>
#include "bmi270.h"
#include "LPF.h"

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

// THIS DOESN'T COMPILE, FIX THIS SOON!!!!!!!!!
void bmi2_delay_us(uint32_t period_us, void *intf_ptr)
{
  (void) intf_ptr;
  delayMicroseconds(period_us);
}

/*Bosch objects*/
struct bmi2_dev bmi;
struct bmi2_sens_data sensor_data;

/*Init LFP class*/
ButterworthLPF accX_LPF;
ButterworthLPF accY_LPF;
ButterworthLPF accZ_LPF;

ButterworthLPF gyrX_LPF;
ButterworthLPF gyrY_LPF;
ButterworthLPF gyrZ_LPF;

/*Teensy Init*/
void setup()
{
  Serial.begin(921600);
  while (!Serial);

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
    while (1);
  }

  uint8_t sens_list[2] = {BMI2_ACCEL, BMI2_GYRO};
  bmi270_sensor_enable(sens_list, 2, &bmi);

  float accFc = 10.0;
  float gyrFc = 5.0;

  initButterworthLPF(&accX_LPF, accFc, 200.0f);
  initButterworthLPF(&accY_LPF, accFc, 200.0f);
  initButterworthLPF(&accZ_LPF, accFc, 200.0f);

  initButterworthLPF(&gyrX_LPF, gyrFc, 200.0f);
  initButterworthLPF(&gyrY_LPF, gyrFc, 200.0f);
  initButterworthLPF(&gyrZ_LPF, gyrFc, 200.0f);
}

/*Get Data*/
void loop()
{
  if (bmi2_get_sensor_data(&sensor_data, &bmi) == BMI2_OK){

    float accX = processButterworthLPF(&accX_LPF, sensor_data.acc.x);
    float accY = processButterworthLPF(&accY_LPF, sensor_data.acc.y);
    float accZ = processButterworthLPF(&accZ_LPF, sensor_data.acc.z);

    float gyrX = processButterworthLPF(&gyrX_LPF, sensor_data.gyr.x);
    float gyrY = processButterworthLPF(&gyrY_LPF, sensor_data.gyr.y);
    float gyrZ = processButterworthLPF(&gyrZ_LPF, sensor_data.gyr.z);

    /* Use for raw data only
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
    */
    Serial.print(accX);
    Serial.print(",");
    Serial.print(accY);
    Serial.print(",");
    Serial.print(accZ);
    Serial.print(",");
    Serial.print(gyrX);
    Serial.print(",");
    Serial.print(gyrY);
    Serial.print(",");
    Serial.println(gyrZ);
  }

  delay(5); //200hz
}
