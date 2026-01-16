#include <Wire.h>
#include "bmi270.h"
#include "LPF.h"
#include <cmath>

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

// dt last time variable
unsigned long prev_time = 0;

// Gyro States
float gyr_pitch = 0;
float gyr_roll = 0;
float gyr_yaw = 0;

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

  // BMI270 API sensor config init
  struct bmi2_sens_config sens_conf;

  // Accel range
  sens_conf.type = BMI2_ACCEL;
  sens_conf.cfg.acc.range = BMI2_ACC_RANGE_4G; // 4g range
  sens_conf.cfg.acc.odr = 200; 
  sens_conf.cfg.acc.bwp = BMI2_ACC_OSR4_AVG1;
  bmi2_set_sensor_config(&sens_conf, 1, &bmi);

  // Gyro range
  sens_conf.type = BMI2_GYRO;
  sens_conf.cfg.gyr.range = BMI2_GYR_RANGE_500; 
  sens_conf.cfg.gyr.odr = 200;
  sens_conf.cfg.gyr.bwp = BMI2_GYR_NORMAL_MODE;
  bmi2_set_sensor_config(&sens_conf, 1, &bmi);


  uint8_t sens_list[2] = {BMI2_ACCEL, BMI2_GYRO};
  bmi270_sensor_enable(sens_list, 2, &bmi);

  // Low pass filtering
  float accFc = 10.0;
  float gyrFc = 5.0;

  initButterworthLPF(&accX_LPF, accFc, 200.0f);
  initButterworthLPF(&accY_LPF, accFc, 200.0f);
  initButterworthLPF(&accZ_LPF, accFc, 200.0f);

  initButterworthLPF(&gyrX_LPF, gyrFc, 200.0f);
  initButterworthLPF(&gyrY_LPF, gyrFc, 200.0f);
  initButterworthLPF(&gyrZ_LPF, gyrFc, 200.0f);

  // Prev time init
  prev_time = micros();

}

/*Get Data*/
void loop()
{
  if (bmi2_get_sensor_data(&sensor_data, &bmi) == BMI2_OK){

    //dt calculation
    unsigned long current_time = micros();
    float dt = (current_time - prev_time) * 1e-6;
    prev_time = current_time;

    // 32768 = 16 bit signed int
    float g_per_lsb = 4.0f / 32768.0f;      
    float dps_per_lsb = 500.0f / 32768.0f;

    //Get raw data and convert to Gs for accel and deg./sec. 
    float ax = sensor_data.acc.x * g_per_lsb;
    float ay = sensor_data.acc.y * g_per_lsb;
    float az = sensor_data.acc.z * g_per_lsb;
    float gx = sensor_data.gyr.x * dps_per_lsb;
    float gy = sensor_data.gyr.y * dps_per_lsb;
    float gz = sensor_data.gyr.z * dps_per_lsb;

    float accX = processButterworthLPF(&accX_LPF, ax);
    float accY = processButterworthLPF(&accY_LPF, ay);
    float accZ = processButterworthLPF(&accZ_LPF, az);
    float gyrX = processButterworthLPF(&gyrX_LPF, gx);
    float gyrY = processButterworthLPF(&gyrY_LPF, gy);
    float gyrZ = processButterworthLPF(&gyrZ_LPF, gz);

    //Accel to pitch and roll instead of raw Gs 
    float roll = atan2(accX, accZ);
    float pitch = atan2(-accX, sqrtf((accY * accY) + (accZ * accZ)));

    float roll_degrees = roll * (180/M_PI);
    float pitch_degrees = pitch * (180/M_PI);

    // Integrate gyro data 
    gyr_roll += gyrX * dt;
    gyr_pitch += gyrY * dt;
    gyr_yaw += gyrZ * dt;

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

    Serial.print(roll_degrees);
    Serial.print(",");
    Serial.print(pitch_degrees);
    Serial.print(",");
    Serial.print(gyr_pitch);  // Check python for variables / change them to pitch, roll, yaw
    Serial.print(",");
    Serial.print(gyr_roll);
    Serial.print(",");
    Serial.println(gyr_yaw);
  }

  delay(5); //200hz
}
