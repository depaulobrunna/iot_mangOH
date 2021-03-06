/**
 * @file
 *
 * This app reads all the Register on BQ24196 or BQ24296 Battery Chargers. It also writes output
 * voltage for battery charger to 3.7V.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless, Inc. Use of this work is subject to license.
 */

#include "legato.h"
#include "interfaces.h"
#if 1
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#else
#include <linux/i2c-dev-user.h>
#endif
//--------------------------------------------------------------------------------------------------
//* Static functions
//--------------------------------------------------------------------------------------------------

/**
 * Get the file handle for the given I2C bus and configure it for access to the given I2C address.
 *
 * @return
 *      - LE_FAULT on failure
 *      - A filehandle to the I2C bus
 */
//--------------------------------------------------------------------------------------------------
static int I2cAccessBusAddr
(
    uint8_t i2cBus,
    uint8_t i2cAddr
)
{
    const size_t filenameSize = 32;
    char filename[filenameSize];

    snprintf(filename, filenameSize, "/dev/i2c/%d", i2cBus);

    LE_DEBUG("Opening I2C bus: '%s'", filename);
    int fd = open(filename, O_RDWR);
    if (fd < 0 && (errno == ENOENT || errno == ENOTDIR))
    {
        snprintf(filename, filenameSize, "/dev/i2c-%d", i2cBus);
        LE_DEBUG("Opening I2C bus: '%s'", filename);
        fd = open(filename, O_RDWR);
    }

    if (fd < 0)
    {
        if (errno == ENOENT)
        {
            LE_ERROR(
                "Could not open file /dev/i2c-%d or /dev/i2c/%d: %s",
                i2cBus,
                i2cBus,
                strerror(ENOENT));
        }
        else
        {
            LE_ERROR("Could not open file %s': %s", filename, strerror(errno));
        }

        return LE_FAULT;
    }

    if (ioctl(fd, I2C_SLAVE_FORCE, i2cAddr) < 0)
    {
        LE_ERROR("Could not set address to 0x%02x: %s", i2cAddr, strerror(errno));
        close(fd);
        return LE_FAULT;
    }

    return fd;
}

//--------------------------------------------------------------------------------------------------
/**
 * Performs an SMBUS read of a 1 byte register
 *
 * @return
 *      - LE_OK
 *      - LE_FAULT
 */
//--------------------------------------------------------------------------------------------------
static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
                                     int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;
    args.read_write = read_write;
    args.command = command;
    args.size = size;
    args.data = data;
    return ioctl(file,I2C_SMBUS,&args);
}

static inline __s32 i2c_smbus_write_byte(int file, __u8 value)
{
    return i2c_smbus_access(file,I2C_SMBUS_WRITE,value,
                            I2C_SMBUS_BYTE,NULL);
}

static inline __s32 i2c_smbus_read_byte(int file)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data))
        return -1;
    else
        return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command,
                                              __u8 value)
{
    union i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
                            I2C_SMBUS_BYTE_DATA, &data);
}

static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                         I2C_SMBUS_BYTE_DATA,&data))
        return -1;
    else
        return 0x0FF & data.byte;
}

static le_result_t SmbusReadReg
(
    uint8_t i2cBus,  ///< [IN] I2C bus to perform the read on
    uint8_t i2cAddr, ///< [IN] I2C address to read from
    uint8_t  reg,    ///< [IN] Register within the I2C device to read
    uint8_t *data    ///< [OUT] Value stored within the register.  This value is only valid if the
                     ///  function returned LE_OK.
)
{
    int i2cFd = I2cAccessBusAddr(i2cBus, i2cAddr);
    if (i2cFd < 0)
    {
        LE_ERROR("failed to open i2c bus %d for access to address %d", i2cBus, i2cAddr);
        return  LE_FAULT;
    }

    le_result_t result;

    const int readResult = i2c_smbus_read_byte_data(i2cFd, reg);

    if (readResult < 0)
    {
        LE_ERROR("smbus read failed with error %d", readResult);
        result = LE_FAULT;
    }
    else
    {
        *data = readResult;
        LE_DEBUG("SMBUS READ addr=0x%x, reg=0x%x, data=0x%x", i2cAddr, reg, *data);
        result = LE_OK;
    }

    close(i2cFd);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Performs an SMBUS write of a 1 byte register
 *
 * @return
 *      - LE_OK
 *      - LE_FAULT
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SmbusWriteReg
(
    uint8_t i2cBus,   ////< I2C bus to perform the write on
    uint8_t i2cAddr,  ////< I2C device address
    uint8_t reg,      ////< Register within the I2C device to write to
    uint8_t data      ////< Data to write to the given register
)
{
    int i2cFd = I2cAccessBusAddr(i2cBus, i2cAddr);

    if (i2cFd == LE_FAULT)
    {
        LE_ERROR("failed to open i2c bus %d for access to address %d", i2cBus, i2cAddr);
        return LE_FAULT ;
    }

    le_result_t result;

    const int writeResult = i2c_smbus_write_byte_data(i2cFd, reg, data);
    if (writeResult < 0)
    {
        LE_ERROR("smbus write failed with error %d", writeResult);
        result = LE_FAULT;
    }
    else
    {
        LE_DEBUG("SMBUS Write addr 0x%x, reg=0x%x, data=0x%x", i2cAddr, reg, data);
        result = LE_OK;
    }

    close(i2cFd);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Enables I2C Switch.
 * This switch is used on mangoh to provide buffering on the i2c lines.
 * If this is not enabled then you cannot access the Battery charger.
 */
//--------------------------------------------------------------------------------------------------
static void EnableI2cBus
(
    void
)
{
    LE_DEBUG("Enabling TCA9546A I2C switch...");
    const int i2cBus = 0;
    const int i2cdev_fd = I2cAccessBusAddr(i2cBus, 0x71);
    LE_FATAL_IF(i2cdev_fd == LE_FAULT, "failed to open i2cbus %d", i2cBus);

    const uint8_t enableAllPorts = 0xff;
    LE_FATAL_IF(i2c_smbus_write_byte(i2cdev_fd, enableAllPorts) == -1, "failed to write i2c data");
}

//--------------------------------------------------------------------------------------------------
/**
 * Reads and prints all the registers on an SMBUS write of a 1 byte register
 */
//--------------------------------------------------------------------------------------------------
static void ReadBatteryChargerRegister
(
    void
)
{
    // read all ten registers of Battery charger with i2c address 0x6B
    uint8_t registerReading;

    SmbusReadReg(0, 0x6B, 0x00, &registerReading);
    LE_DEBUG("Input Source Control value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x01, &registerReading);
    LE_DEBUG("Power-On Configuration Register value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x02, &registerReading);
    LE_DEBUG("Charge Current Control Register value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x03, &registerReading);
    LE_DEBUG("Pre-Charge/Termination Current Control Register value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x04, &registerReading);
    LE_DEBUG("Charge Voltage Control Register value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x05, &registerReading);
    LE_DEBUG("Charge Termination/Timer Control Register value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x06, &registerReading);
    LE_DEBUG("Thermal Regulation Control Register value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x07, &registerReading);
    LE_DEBUG("Misc Operation Control Register value is %d", registerReading);

    sleep(1);

    SmbusReadReg(0, 0x6B, 0x08, &registerReading);
    LE_DEBUG("System Status Register value is %d", registerReading);

    SmbusReadReg(0, 0x6B, 0x09, &registerReading);
    LE_DEBUG("Fault Register value is %d", registerReading);

    SmbusReadReg(0, 0x6B, 0x0A, &registerReading);
    LE_DEBUG("Vendor / Part / Revision Status Register value is %d", registerReading);
}

//--------------------------------------------------------------------------------------------------
/**
 * Changes battery charger output voltage and reads back register.
 */
//--------------------------------------------------------------------------------------------------
static void OutputBatteryChargerVoltage
(
    void
)
{
    uint8_t OutputVoltage;
    SmbusWriteReg(0, 0x6B, 0x04, 0xBA);

    SmbusReadReg(0, 0x6B, 0x04, &OutputVoltage);
    LE_DEBUG("Output voltage is set to %d", OutputVoltage);
}


COMPONENT_INIT
{

    LE_INFO("=============== I2C Reading & Writing application has started");
    EnableI2cBus();
    ReadBatteryChargerRegister();
    OutputBatteryChargerVoltage();
}