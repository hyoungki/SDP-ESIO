#if 0
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include <errno.h>
#include <stddef.h>
#include  "smbus.h"
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include "i2c-dev.h"


#include <strings.h>
#endif

#include	"localLib.h"
#include    "smbus.h"
#include    "i2c-dev.h"





__s32 i2c_smbus_access(int file, char read_write, __u8 command,
		       int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;
	__s32 err;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	err = ioctl(file, I2C_SMBUS, &args);
	if (err == -1)
		err = -errno;
	return err;
}


__s32 i2c_smbus_write_byte(int file, __u8 value)
{
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, value,
				I2C_SMBUS_BYTE, NULL);
}


int  i2c_smbus_read_byte(int file)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data);
	if (err < 0)
	{
         printf("%s\n\n", strerror(errno));
		return err;
    }
	return 0x0FF & data.byte;
}

