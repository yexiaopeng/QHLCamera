#include "watchdog.h"

WatchDog::WatchDog(QObject *parent) : QObject(parent)
{
    int data = 0;
    int ret;

    //open
    m_am335x_watchDog_fd = open("/dev/watchdog",O_WRONLY);
    if (m_am335x_watchDog_fd==-1) {
                perror("watchdog");
                return ;
        }

    // set timeout time
    data = 58;
    ret = ioctl (m_am335x_watchDog_fd, WDIOC_SETTIMEOUT, &data);
    if (ret) {
       printf ("\nWatchdog Timer : WDIOC_SETTIMEOUT failed");
    }
    else {
       printf ("\nSet  timeout value is : %d seconds\n", data);
    }
}

WatchDog::~WatchDog()
{
    // close(m_am335x_watchDog_fd);
}

void WatchDog::feedDog()
{
    write(m_am335x_watchDog_fd, "\0", 1);
}
