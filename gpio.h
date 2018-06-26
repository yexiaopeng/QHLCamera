#ifndef GPIO_H
#define GPIO_H

#include <QObject>
//
//fsdf
//工控机IO引脚配置：
//gpio1_12 -- 电子锁的驱动信号                 -- 输出模式
//gpio1_15 -- 电子锁的开关门检测信号       -- 输入模式  opened -- high  closed -- low
//gpio0_22 -- 人体检测传感器的检测信号   -- 输入模式
//gpio0_23 -- 温湿度传感器信号                -- 输入输出模式
#define GPIO_LOCK_OPEN_GROUNP 1
#define GPIO_LOCK_OPEN_NUMBER 12
#define GPIO_LOCK_CHECK_GROUNP 1
#define GPIO_LOCK_CHECK_NUMBER 15
#define GPIO_MAN_GROUNP 0
#define GPIO_MAN_NUMBER 22
#define GPIO_TEMP_GROUNP 0
#define GPIO_TEMP_NUMBER 23

#define LOCK_OPENSTATE_LEVEL 1

#define LOCK_CLOSESTATE_LEVEL 0


typedef struct {
        int pin;
        int data;
}am335x_gpio_arg;

class Gpio : public QObject
{
    Q_OBJECT
public:
    explicit Gpio(QObject *parent = 0);
    ~Gpio();

    int Write(int group, int num, int level);
    int Read(int group,int num);
    void AM2302_Mode_IPU(void);
    void AM2302_Mode_OUT(void);


signals:

public slots:
private:
    am335x_gpio_arg m_arg;
    int m_am335x_gpio_fd;
};

#endif // GPIO_H
