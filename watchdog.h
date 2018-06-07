#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <QObject>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <linux/watchdog.h>
#include <signal.h>
#include <sys/ioctl.h>


class WatchDog : public QObject
{
    Q_OBJECT
public:
    explicit WatchDog(QObject *parent = 0);
    ~WatchDog();

    void feedDog();


signals:

public slots:

private:
    int m_am335x_watchDog_fd;

};

#endif // WATCHDOG_H
