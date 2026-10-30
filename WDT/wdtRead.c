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


extern  MPC860IO_DESC    idPort;

#if 0
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

#endif


#define  I2C_DEVICE "/dev/i2c-3"
#define  VME_ID_I2C_ADDR   0x18
#define  USB_RST_I2C_ADDR   0x1E
#define FRONT_LED_H__I2C_ADDR 0x1A
#define FRONT_LED_L_I2C_ADDR 0x1E

int esio_id_fd=-1;

int init_esio_id(void)
{
    if ((esio_id_fd = open(I2C_DEVICE,O_RDWR)) < 0) {
        
        printf("Failed to open the bus 0.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return -1 ;
     }
     
    if (ioctl(esio_id_fd,  I2C_SLAVE         ,VME_ID_I2C_ADDR) < 0) {
        printf("Failed to acquire bus access and/or talk to slave 0.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return -1 ;

    }
        return 0;
}

unsigned char  read_esio_id(void)
{
    unsigned char buf[2];
    int rc ;
   
    buf[0]=0x00;

    /* set addr */      
    rc =write(esio_id_fd, buf,1 ) ;
    if  ( rc < 0) 
        printf("can't write esio_id\r\n");
   
    /* read input register */
    rc=read(esio_id_fd, buf,1);    
    if  ( rc < 0) 
        printf("can't read esio_id\r\n");
    
    return buf[0];
}

/*
*  VMEBUS ID Read
*/
int readESIO_ID(void)
{

    unsigned char vme_id ;
    
    vme_id = read_esio_id() & 0x0F ;
    vme_id = (~vme_id) & 0x0F ;
    

    return (vme_id); 
}

/*
*  VMEBUS ID Read
*/
int readESIO_SWITCH(void)
{

    unsigned char vme_id ;
    
    vme_id = read_esio_id();
    vme_id = (vme_id >> 4) & 0x0F ;
    
    return (vme_id); 
    
}
