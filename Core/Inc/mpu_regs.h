/*
 * mpu_regs.h
 *
 *  Created on: 20 de jul. de 2026
 *      Author: pedro.neto
 */

#ifndef INC_MPU_REGS_H_
#define INC_MPU_REGS_H_

#define MPU6050_REG_SELF_TEST_X 0x0D
#define MPU6050_REG_SELF_TEST_Y 0x0E
#define MPU6050_REG_SELF_TEST_Z 0x0F
#define MPU6050_REG_SELF_TEST_A 0x10

#define MPU6050_REG_SMPRT_DIV 0x19
#define MPU6050_REG_CONFIG 0x1A
#define MPU6050_REG_GYRO_CONFIG 0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C

#define MPU6050_REG_FIFO_EN_CFG 0x23

#define MPU6050_REG_INT_PIN_CFG 0x37
#define MPU6050_REG_INT_EN 0x38
#define MPU6050_REG_INT_STATUS 0x3A

#define MPU6050_REG_ACCEL_DATA 0x3B
#define MPU6050_REG_TEMP_DATA 0x41
#define MPU6050_REG_GYRO_DATA 0x43

#define MPU6050_REG_SIGNAL_RST 0x68
#define MPU6050_REG_USER_CTRL 0x6A
#define MPU6050_REG_PWR_MGMT1 0x6B
#define MPU6050_REG_PWR_MGMT2 0x6C
#define MPU6050_REG_WHOAMI 0x75

#define MPU6050_CHIPID ((0b110100) << 1)

/* MPU6050_REG_CONFIG (0x1A) */
typedef enum {
    MPU6050_SYNC_DISABLED = 0,
    MPU6050_SYNC_TEMP_OUT_L,
    MPU6050_SYNC_GYRO_XOUT_L,
    MPU6050_SYNC_GYRO_YOUT_L,
    MPU6050_SYNC_GYRO_ZOUT_L,
    MPU6050_SYNC_ACCEL_XOUT_L,
    MPU6050_SYNC_ACCEL_YOUT_L,
    MPU6050_SYNC_ACCEL_ZOUT_L,
} MPU6050_ExtSync_t;

typedef enum {
    MPU6050_DLPF_260HZ = 0,
    MPU6050_DLPF_184HZ,
    MPU6050_DLPF_94HZ,
    MPU6050_DLPF_44HZ,
    MPU6050_DLPF_21HZ,
    MPU6050_DLPF_10HZ,
    MPU6050_DLPF_5HZ,
} MPU6050_Dlpf_t;

typedef struct {
    MPU6050_ExtSync_t sync;
    MPU6050_Dlpf_t dlpf;
} MPU6050_Config_t;

/* MPU6050_REG_GYRO_CONFIG (0x1B) */
typedef enum {
    MPU6050_GYRO_SELF_TEST_X = (1 << 7),
    MPU6050_GYRO_SELF_TEST_Y = (1 << 6),
    MPU6050_GYRO_SELF_TEST_Z = (1 << 5),
} MPU6050_GyroFlags_t;

typedef enum {
    MPU6050_GYRO_250DPS = 0,
    MPU6050_GYRO_500DPS,
    MPU6050_GYRO_1000DPS,
    MPU6050_GYRO_2000DPS,
} MPU6050_GyroRange_t;

typedef struct {
    uint8_t flags;
    MPU6050_GyroRange_t range;
} MPU6050_GyroConfig_t;

/* MPU6050_REG_ACCEL_CONFIG (0x1C) */
typedef enum {
    MPU6050_ACCEL_SELF_TEST_X = (1 << 7),
    MPU6050_ACCEL_SELF_TEST_Y = (1 << 6),
    MPU6050_ACCEL_SELF_TEST_Z = (1 << 5),
} MPU6050_AccelFlags_t;

typedef enum {
    MPU6050_ACCEL_2G = 0,
    MPU6050_ACCEL_4G,
    MPU6050_ACCEL_8G,
    MPU6050_ACCEL_16G,
} MPU6050_AccelRange_t;

typedef struct {
    uint8_t flags;
    MPU6050_AccelRange_t range;
} MPU6050_AccelConfig_t;

/* MPU6050_REG_FIFO_EN (0x23) */
typedef enum {
    MPU6050_FIFO_TEMP      = (1 << 7),
    MPU6050_FIFO_GYRO_X    = (1 << 6),
    MPU6050_FIFO_GYRO_Y    = (1 << 5),
    MPU6050_FIFO_GYRO_Z    = (1 << 4),
    MPU6050_FIFO_ACCEL     = (1 << 3),

    MPU6050_FIFO_SLV2      = (1 << 2),
    MPU6050_FIFO_SLV1      = (1 << 1),
    MPU6050_FIFO_SLV0      = (1 << 0),
} MPU6050_FifoFlags_t;

/* MPU6050_REG_INT_PIN_CFG (0x37) */
typedef enum {
    MPU6050_INT_ACTIVE_LOW     = (1 << 7),
    MPU6050_INT_OPEN_DRAIN     = (1 << 6),
    MPU6050_INT_LATCH          = (1 << 5),
    MPU6050_INT_CLEAR_ON_READ  = (1 << 4),
    MPU6050_FSYNC_ACTIVE_LOW   = (1 << 3),
    MPU6050_FSYNC_INT_ENABLE   = (1 << 2),
    MPU6050_I2C_BYPASS_ENABLE  = (1 << 1),
} MPU6050_IntPinFlags_t;

/* MPU6050_REG_INT_EN (0x38) */
typedef enum {
    MPU6050_INT_DATA_READY      = (1 << 0),
    MPU6050_INT_I2C_MASTER      = (1 << 3),
    MPU6050_INT_FIFO_OVERFLOW   = (1 << 4),
    MPU6050_INT_MOTION          = (1 << 6),
} MPU6050_IntEnableFlags_t;

/* MPU6050_REG_USER_CTRL (0x6A) */
typedef enum {
    MPU6050_USER_SIG_COND_RESET = (1 << 0),
    MPU6050_USER_I2C_RESET      = (1 << 1),
    MPU6050_USER_FIFO_RESET     = (1 << 2),
    MPU6050_USER_I2C_MASTER     = (1 << 5),
    MPU6050_USER_FIFO_ENABLE    = (1 << 6),
    MPU6050_USER_DMP_ENABLE     = (1 << 7),
} MPU6050_UserCtrlFlags_t;

/* MPU6050_REG_PWER_MGMT1 (0x6B) */
typedef enum {
    MPU6050_PWR_TEMP_DISABLE = (1 << 3),
    MPU6050_PWR_CYCLE        = (1 << 5),
    MPU6050_PWR_SLEEP        = (1 << 6),
    MPU6050_PWR_DEVICE_RESET = (1 << 7),
} MPU6050_PwrFlags_t;

typedef enum {
    MPU6050_CLK_INTERNAL = 0,
    MPU6050_CLK_PLL_XGYRO,
    MPU6050_CLK_PLL_YGYRO,
    MPU6050_CLK_PLL_ZGYRO,
    MPU6050_CLK_EXT_32KHZ,
    MPU6050_CLK_EXT_19MHZ,
    MPU6050_CLK_STOP,
} MPU6050_ClockSource_t;

typedef struct {
    uint8_t flags;
    MPU6050_ClockSource_t clock;
} MPU6050_PwrMgmt1_t;

/* MPU6050_REG_PWER_MGMT2 (0x6C) */
typedef enum {
    MPU6050_STBY_GYRO_X  = (1 << 2),
    MPU6050_STBY_GYRO_Y  = (1 << 1),
    MPU6050_STBY_GYRO_Z  = (1 << 0),
    MPU6050_STBY_ACCEL_X = (1 << 5),
    MPU6050_STBY_ACCEL_Y = (1 << 4),
    MPU6050_STBY_ACCEL_Z = (1 << 3),
} MPU6050_StandbyFlags_t;

typedef enum {
    MPU6050_WAKE_1P25HZ = 0,
    MPU6050_WAKE_2P5HZ,
    MPU6050_WAKE_5HZ,
    MPU6050_WAKE_10HZ,
} MPU6050_WakeFreq_t;

typedef struct {
    uint8_t flags;
    MPU6050_WakeFreq_t wake_freq;
} MPU6050_PwrMgmt2_t;

#endif /* INC_MPU_REGS_H_ */
