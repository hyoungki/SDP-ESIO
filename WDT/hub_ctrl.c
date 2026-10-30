/* 이 프로그램은 HUB PWR 제어를 1하씩 on 하여 시험하는 것 */


#if 0

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
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

 int  i2c_smbus_write_byte(int file, unsigned  char  value)
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

#define  I2C_DEVICE "/dev/i2c-1"
#define  USB_PWR_I2C_ADDR   0x18  
#define  USB_RST_I2C_ADDR   0x1E
#define FRONT_LED_H__I2C_ADDR 0x1A
#define FRONT_LED_L_I2C_ADDR 0x1E

//unsigned char BITS[]={ 0x00, 0x01, 0x02, 0x04,0x08, 0x10, 0x20, 0x40, 0x80 };
unsigned char BITS[]={ 0x01, 0x02, 0x04,0x08, 0x10, 0x20, 0x40, 0x80 };

unsigned char ADDED_BITS[]={ 0x00, 0x01, 0x03,0x07, 0x0F, 0x1F, 0x3F, 0x7F };

#define  WRITE_LED(x,y)  write_led(x,y)

// 출력을 input->out으로 변경전에 출력 값을 먼저 넣어야  glitch가 없다.
static int  chage_mode( int fd)
{
    char buf[3];
    int rc;
    




    /* chage to output  */
    buf[0]= 0x03 ;// register address 
    buf[1]= 0x00 ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy\r\n");
        return 0;
          
    }
    

    /* chage polarity */
    /* write 1 ,then IC set 0 with led on */
    buf[0]= 0x02 ;// register address 
    buf[1]= 0xFF ; // all to out  2025-11-04 오후 5:53:41 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy\r\n");
        return 0;
          
    }

    buf[0]=2;
    write(fd, buf,1);
    buf[0]=0xa5;
    read(fd, buf,1);    

    printf("Inversion %02X\r\n",buf[0]);

    return 0;
}



static int  write_led( int fd, unsigned char data)
{
    char buf[3];
    buf[0]=0x1;
    buf[1]=data ;
    int rc; 
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change mode\r\n");
        return 0;
          
    }

   return rc;
      
}

#define  DEBUG_PRINTF(x)   printf("%s:%d : %s\r\n",__FUNCTION__,__LINE__, x)

// 출력을 input->out으로 변경전에 출력 값을 먼저 넣어야  glitch가 없다.
int  chage_input_mode( int fd)
{
    char buf[3];
    int rc;
    
#if 0    
    buf[0]=3;
    write(fd, buf,1);
    buf[0]=0xa5;
    read(fd, buf,1);    

    printf("mode reg %02X\r\n",buf[0]);
#endif 


    /* chage to output  */
    buf[0]= 0x03 ;// register address 
    buf[1]= 0xFF ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy\r\n");
        return 0;
          
    }

#if 0  
      buf[0]=3;
    write(fd, buf,1);
    buf[0]=0xa5;
    read(fd, buf,1);    

    printf("mode reg %02X\r\n",buf[0]);
    
#endif     

    /* chage polarity */
    /* write 1 ,then IC set 0 with led on */
    buf[0]= 0x02 ;// register address 
    buf[1]= 0xFF ; // all to out  2025-11-04 오후 5:53:41 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy\r\n");
        return 0;
          
    }

    buf[0]=2;
    write(fd, buf,1);
    buf[0]=0xa5;
    read(fd, buf,1);    

    printf("Inversion %02X\r\n",buf[0]);

    return 0;
}


int hub_pwer_on(void ) 
{
    int file;

    char buffer[10];
    int addr = 0b00011000 ; // 0b00101001;  0x18 is i2c-exapnsion       // The I2C address of the ADC
    int i ;
    int rc;

    int ch ;

    int  i2c_fd_0;
    int  i2c_fd_1;
    

    char buf[10] = {0};
    float data;
    char channel;
    
 //   printf("open HUB-PWR-CTRL\r\n");

    
    if ((i2c_fd_0 = open(I2C_DEVICE,O_RDWR)) < 0) {
        DEBUG_PRINTF("%s : Failed to open the bus 0.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return LIOW_NOK ;
    }

    if (ioctl(i2c_fd_0,  I2C_SLAVE         ,USB_PWR_I2C_ADDR) < 0) {
        DEBUG_PRINTF("Failed to acquire bus access and/or talk to slave 0.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return LIOW_NOK ;
    }



 //   printf("open HUB-RST-CTRL\r\n");
    if ((i2c_fd_1 = open(I2C_DEVICE,O_RDWR)) < 0) {
        DEBUG_PRINTF("Failed to open the bus 1 .");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return LIOW_NOK ;
    }

    if (ioctl(i2c_fd_1,  I2C_SLAVE         ,USB_RST_I2C_ADDR) < 0) {
        DEBUG_PRINTF("Failed to acquire bus access and/or talk to slave  1.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return LIOW_NOK ;
    }

    /* out mode and change polirity */
//    printf("initial control pwr\r\n");
    buffer[0]=0x41| 0x02| 0x10 ; /* Serial#1,Serial#2, LAN3 */ 
    write_led(i2c_fd_0, buffer[0] ) ; /* 모든 변경전 출력값 지정, 기본 PWR off */
    chage_mode( i2c_fd_0 );

    /* 모든 변경전 출력값 지정, 기본 RST off, PWR에 rst가 연결되어 POR 된다. 모든 RESET을 high (inactive) 상태로. */    
 //   printf("initail control rst\r\n");
    write_led(i2c_fd_1, 0xFF ) ; 
    
    // 출력모드로 변경 */
    // chage_mode( i2c_fd_1 );
    // 2026-05-13 오후 6:45:13
    chage_input_mode(i2c_fd_1 ) ; /* PWR all on  */

    /* 모든 변경전 출력값 지정, 기본 RST off, PWR에 rst가 연결되어 POR 된다. 모든 RESET을 high (inactive) 상태로. */      
//    write_led(i2c_fd_1, 0xFF ) ; /* PWR all on  */ 

    sleep(3) ;
    
 //   printf("turn all hub  pwr\r\n");
 // 2026-05-12 오후 4:51:58    lan0/1 시간차 두기. 
 //          write_led(i2c_fd_0, 0xFF ) ; /* PWR all on  */

// 2026-05-12 오후 4:52:03 
// 2026-05-13 오후 5:22:28        
 
                
        // 3,4번은 off로 (이유는 지금 Serial 이 부족)
        // enable Port 6  LAN2
        buffer[0] = buffer[0] | 0x20 ;         
        write_led(i2c_fd_0, buffer[0] ) ; 
        sleep(5);
        

        
    return 0; 
}