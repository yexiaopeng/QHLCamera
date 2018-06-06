#ifndef QHLMQTT_H
#define QHLMQTT_H

#include <QObject>
#include <QTimer>

#include "QtMqtt/QMqttClient"

//#include "QtScript/QtScript"

#include <QSettings>

#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QProcess>

#include "cJSON.h"
#include <QtNetwork>
#include "qftp.h"
#include "V4l2CameraControl.h"
#include "gpio.h"

typedef struct
{
    uint8_t  humi_high8bit;		//原始数据：湿度高8位
    uint8_t  humi_low8bit;	 	//原始数据：湿度低8位
    uint8_t  temp_high8bit;	 	//原始数据：温度高8位
    uint8_t  temp_low8bit;	 	//原始数据：温度高8位
    uint8_t  check_sum;	 	    //校验和
  float    humidity;        //实际湿度
  float    temperature;     //实际温度
} AM2302_Data_TypeDef;

class QHLMqtt : public QObject
{
    Q_OBJECT
public:
    explicit QHLMqtt(QObject * parent = 0);
    virtual  ~QHLMqtt();
public:
    QMqttClient * m_client;
    QTimer * m_timer_publish;
    QTimer * m_timer;
    QTimer * m_timer_camera;

    QUrl * m_ftpurl;
    QFtp * m_ftp;

    int state_flag = 0;

    Gpio * m_gpio;

    QString mqttHostName;
    quint16 mqttHostPort;
    quint16 deviceId;

    QString mqttSubscribeTopic;
    QString mqttPublishTopic;

    QNetworkAccessManager * manager;


public:

    void mysleep(int msec);

    void setDeviceId(quint16 id);
    void setMqttServerAddr(QString hostname, quint16 port);
    void setMqttSubscribeTopic(QString topic);
    void setMqttPublishTopic(QString topic);

    int onMqttConnectToServer();
    int onMqttPublishDataToServer(const QByteArray message);
    int onMqttSubscribeDataToServer();
    int onFtpConnectToServer(void);
    int onFtpPutFileToServer(QString fileName);

    int onV4l2GetJgp();
    int onOpenLock(int index);

    //AM2302
    uint8_t AM2302_ReadByte(void);
    uint8_t AM2302_Read_TempAndHumidity(AM2302_Data_TypeDef *AM2302_Data);
    void AM2302_Delay(uint16_t time);

public slots:

   void onMqttConnected(void);

   void onMqttDisconnected(void);

   void onMqttSubscribeReceive(const QByteArray &message,
                               const QMqttTopicName &topic);
    void showTime();
    void timerPublish();
    void timerCamera();
};

#endif // QHLMQTT_H
