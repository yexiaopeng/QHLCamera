#include "gpio.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>

/************************
gpmc_ad8.gpio0_22
gpmc_ad9.gpio0_23
gpmc_ad10.gpio0_26
gpmc_ad11.gpio0_27
gpmc_ad12.gpio1_12
gpmc_ad13.gpio1_13
gpmc_ad14.gpio1_14
gpmc_ad15.gpio1_15
************************/

#define DEV_GPIO "/dev/am335x_gpio"

#define GPIO_IOC_MAGIC   'G'
#define IOCTL_GPIO_SETOUTPUT              _IOW(GPIO_IOC_MAGIC, 0, int)
#define IOCTL_GPIO_SETINPUT               _IOW(GPIO_IOC_MAGIC, 1, int)
#define IOCTL_GPIO_SETVALUE               _IOW(GPIO_IOC_MAGIC, 2, int)
#define IOCTL_GPIO_GETVALUE    		  _IOR(GPIO_IOC_MAGIC, 3, int)


#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))



Gpio::Gpio(QObject *parent) :
    QObject(parent),
    m_am335x_gpio_fd(-1)
{

    m_am335x_gpio_fd = open(DEV_GPIO, O_RDWR);
    if (m_am335x_gpio_fd < 0) {
        printf("Error device open fail! %d!!!!!!!!!!!!!!!\n", m_am335x_gpio_fd);
        return;
    }

    printf("\r\n ---- GPIO ok ---- ");

    am335x_gpio_arg arg;
    // 1_12 - lock control -- output
    arg.pin = GPIO_TO_PIN(GPIO_LOCK_OPEN_GROUNP, GPIO_LOCK_OPEN_NUMBER);
    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_SETOUTPUT , &arg);
    this->Write(GPIO_LOCK_OPEN_GROUNP,GPIO_LOCK_OPEN_NUMBER,0);
     am335x_gpio_arg arg_2;
    //1_15 - lock check -- input
    arg.pin = GPIO_TO_PIN(GPIO_LOCK_CHECK_GROUNP , GPIO_LOCK_CHECK_NUMBER);
    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_SETINPUT, &arg_2);

//    //0_22 main -- input
//    arg.pin = GPIO_TO_PIN(GPIO_MAN_GROUNP, GPIO_MAN_NUMBER);
//    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_SETINPUT, &arg);
//    // 0_23  temp -- input/output
//    arg.pin = GPIO_TO_PIN(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER);
//    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_SETINPUT, &arg);
}

Gpio::~Gpio()
{
    close(m_am335x_gpio_fd);
}

int Gpio::Write(int group, int num,int level)
{

    am335x_gpio_arg arg;
    arg.data = level;

    arg.pin = GPIO_TO_PIN(group, num);

    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_SETVALUE, &arg);

    return arg.data;

}

int Gpio::Read(int group, int num)
{
    am335x_gpio_arg arg;

    arg.pin = GPIO_TO_PIN(group, num);

    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_GETVALUE, &arg);

    return arg.data;
}

void Gpio::AM2302_Mode_IPU()
{
 //#define GPIO_TEMP_GROUNP 0
 //#define GPIO_TEMP_NUMBER 23
    am335x_gpio_arg arg;
    arg.pin = GPIO_TO_PIN(GPIO_TEMP_GROUNP, GPIO_TEMP_NUMBER);
    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_SETINPUT , &arg);
}

void Gpio::AM2302_Mode_OUT()
{
    am335x_gpio_arg arg;
    arg.pin = GPIO_TO_PIN(GPIO_TEMP_GROUNP, GPIO_TEMP_NUMBER);
    ioctl(m_am335x_gpio_fd, IOCTL_GPIO_SETOUTPUT , &arg);
}
