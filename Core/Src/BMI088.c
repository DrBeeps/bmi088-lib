#include "BMI088.h"

uint8_t BMI088_Init(BMI088IMU *imu, I2C_HandleTypeDef *I2Chandle)
{
    imu->I2Chandle     = I2Chandle;
    imu->acc[0] = 0.0f;
    imu->acc[1] = 0.0f;
    imu->acc[2] = 0.0f;
    imu->gyr[0] = 0.0f;
    imu->gyr[1] = 0.0f;
    imu->gyr[2] = 0.0f;

    uint8_t txBuf[2];

    /*
     * ACCELEROMETER
     */

    /* Soft reset */
    txBuf[0] = BMI088_ACC_SOFTRESET; txBuf[1] = 0xB6;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_ACC_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    HAL_Delay(50);

    /* Check chip ID */
    uint8_t chipID;
    debugTest = 0;
    HAL_I2C_Mem_Read(imu->I2Chandle, BMI088_ACC_I2C_ADDR, BMI088_ACC_CHIP_ID, I2C_MEMADD_SIZE_8BIT, &chipID, 1, BMI088_I2C_TIMEOUT);

    if (chipID != 0x1E)
    {
        debugTest = 1;
        return 0;
    }

    /* Configure accelerometer LPF bandwidth (OSR4, 1000) and ODR (200 Hz, 1001) --> Actual bandwidth = 20 Hz */
    uint8_t accConf = 0x89;
    txBuf[0] = BMI088_ACC_CONF; txBuf[1] = accConf;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_ACC_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    /* Accelerometer range (+-6G = 0x01) */
    uint8_t accRange = 0x01;
    txBuf[0] = BMI088_ACC_RANGE; txBuf[1] = accRange;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_ACC_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    /* Set accelerometer to active mode */
    txBuf[0] = BMI088_ACC_PWR_CONF; txBuf[1] = 0x00;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_ACC_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    /* Switch accelerometer on */
    txBuf[0] = BMI088_ACC_PWR_CTRL; txBuf[1] = 0x04;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_ACC_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);


    /*
     * GYROSCOPE
     */

    /* Soft reset */
    txBuf[0] = BMI088_GYR_SOFTRESET; txBuf[1] = 0xB6;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_GYR_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    HAL_Delay(35);

    /* Check chip ID */
    HAL_I2C_Mem_Read(imu->I2Chandle, BMI088_GYR_I2C_ADDR, BMI088_GYR_CHIP_ID, I2C_MEMADD_SIZE_8BIT, &chipID, 1, BMI088_I2C_TIMEOUT);

    if (chipID != 0x0F)
    {
            return 0;
    }

    /* Gyro range (+- 500deg/s) */
    uint8_t gyrRange = 0x01;
    txBuf[0] = BMI088_GYR_RANGE; txBuf[1] = gyrRange;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_GYR_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    /* Gyro bandwidth/ODR (116Hz / 1000 Hz) */
    uint8_t gyrBandwidth = 0x02;
    txBuf[0] = BMI088_GYR_BANDWIDTH; txBuf[1] = gyrBandwidth;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_GYR_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    /* Gyro power mode */
    txBuf[0] = BMI088_GYR_LPM1; txBuf[1] = 0x00;
    HAL_I2C_Master_Transmit(imu->I2Chandle, BMI088_GYR_I2C_ADDR, txBuf, 2, BMI088_I2C_TIMEOUT);

    return 1;
}

void BMI088_ReadAcc(BMI088IMU *imu)
{
    uint8_t rxBuf[6];
    HAL_I2C_Mem_Read(imu->I2Chandle, BMI088_ACC_I2C_ADDR, BMI088_ACC_DATA, I2C_MEMADD_SIZE_8BIT, rxBuf, 6, BMI088_I2C_TIMEOUT);

    int16_t accX = rxBuf[1];
            accX <<= 8;
            accX |= rxBuf[0];

    int16_t accY = rxBuf[3];
            accY <<= 8;
            accY |= rxBuf[2];

    int16_t accZ = rxBuf[5];
            accZ <<= 8;
            accZ |= rxBuf[4];

    /* Scale (to m/s^2) and re-map axes */
    imu->acc[0] = -accY * 0.00179626456f;
    imu->acc[1] = -accX * 0.00179626456f;
    imu->acc[2] = -accZ * 0.00179626456f;
}

void BMI088_ReadGyr(BMI088IMU *imu)
{
    uint8_t rxBuf[6];
    HAL_I2C_Mem_Read(imu->I2Chandle, BMI088_GYR_I2C_ADDR, BMI088_GYR_DATA, I2C_MEMADD_SIZE_8BIT, rxBuf, 6, BMI088_I2C_TIMEOUT);

    int16_t gyrX = rxBuf[1];
            gyrX <<= 8;
            gyrX |= rxBuf[0];

    int16_t gyrY = rxBuf[3];
            gyrY <<= 8;
            gyrY |= rxBuf[2];

    int16_t gyrZ = rxBuf[5];
            gyrZ <<= 8;
            gyrZ |= rxBuf[4];

    /* Scale (to rad/s) and re-map axes */
    imu->gyr[0] = -gyrY * 0.00026632423f;
    imu->gyr[1] = -gyrX * 0.00026632423f;
    imu->gyr[2] = -gyrZ * 0.00026632423f;
}


