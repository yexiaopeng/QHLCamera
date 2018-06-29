#include <QCoreApplication>
#include "QHLMqtt.h"
#include <QApplication>
int main(int argc, char *argv[])
{
   // QCoreApplication a(argc, argv);
  QApplication a(argc, argv);
    qDebug() << "------- QHLCameraLock V1 ------- NTSYANS";






    QSettings * configIniRead = new QSettings(QCoreApplication::applicationDirPath() +"/QHLConfig.ini",
                                              QSettings::IniFormat);
    QString hostname = configIniRead->value("serverip").toString();
    quint16 hostport = configIniRead->value("serverport").toInt();
    quint16 deviceid = configIniRead->value("deviceid").toInt();
    QString  mqttSubscribeTopic  = "cname_xh/project_plock/gw_gw/d_plock/dsn_"+QString::number(deviceid)+"/";
    QString  mqttPublishTopic= "myhouse/trade_gov/cname_xh/project_plock/gw_gw/d_plock/dsn_"+QString::number(deviceid)+"/";
    QString localip = configIniRead->value("localip").toString();
    QString netmask = configIniRead->value("netmask").toString();
    QString gateway = configIniRead->value("gateway").toString();

    delete configIniRead;




    qDebug() << "------- ifconfig network -------";


   // hlmqtt.setloaclNetWork(localip,netmask,gateway);
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressesList.size(); ++i) {
            if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
                ipAddress = ipAddressesList.at(i).toString();
                break;
            }
     }
    if (ipAddress.isEmpty())
            ipAddress = QHostAddress(QHostAddress::LocalHost).toString();


    qDebug() << "deviceid=" << deviceid;
    qDebug() << "localip=" << ipAddress;
    qDebug() << "hostname=" << hostname << " hostport=" << hostport;

    qDebug() << "mqttPublishTopic=" << mqttPublishTopic;
    qDebug() << "mqttSubscribeTopic=" << mqttSubscribeTopic;


    QHLMqtt hlmqtt;

    hlmqtt.setDeviceId(deviceid);
    hlmqtt.setMqttServerAddr(hostname,hostport);
    hlmqtt.setMqttPublishTopic(mqttPublishTopic);

    hlmqtt.serialOpenCmd();

    //hlmqtt.onFtpConnectToServer(); into  onMqttConnectToServer
    hlmqtt.setMqttSubscribeTopic(mqttSubscribeTopic);
    hlmqtt.onMqttConnectToServer();

    return a.exec();
}
