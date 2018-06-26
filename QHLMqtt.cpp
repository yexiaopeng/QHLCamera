#include "QHLMqtt.h"
#include "stdio.h"


QHLMqtt::QHLMqtt(QObject *parent) :
    QObject(parent)
{
    state_flag = 0;
    m_client = new QMqttClient(this);
    m_ftpurl = new QUrl();
    m_timer  = new QTimer(this);
    m_timer_publish = new QTimer(this);
    m_timer_camera  = new QTimer(this);
    m_ftp = new QFtp(this);
    manager = new QNetworkAccessManager();
    m_gpio = new Gpio();
    m_SerialPort = new QSerialPort(this);
    m_watchdog = new WatchDog();
    m_timer_feedDog = new QTimer(this);
    //receive data
    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
            onMqttSubscribeReceive(message,topic);});
    //if mqtt connected
    connect(m_client,SIGNAL(connected()) ,this,SLOT(onMqttConnected()));
    connect(m_timer,SIGNAL(timeout()),this,SLOT(showTime()));
    connect(m_timer_publish,SIGNAL(timeout()),this,SLOT(timerPublish()));
    connect(m_timer_camera,SIGNAL(timeout()),this,SLOT(timerCamera()));
    connect(m_timer_feedDog,SIGNAL(timeout()),this,SLOT(timerFeedDog()));
    connect(m_client, &QMqttClient::disconnected, this, &QHLMqtt::onMqttDisconnected);

    //serial
    connect(m_SerialPort,SIGNAL(readyRead()),this,SLOT(on_comserial_read()));
    //watch dog


    onSetSerialCom();

    //m_timer->start(5000);
    //printf("\r\n AM2302 START \r\n");



    m_timer_feedDog->start(15000);

    state_flag = 0;
}

QHLMqtt::~QHLMqtt()
{

}

void QHLMqtt::mysleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);


    while( QTime::currentTime() < dieTime )

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void QHLMqtt::setDeviceId(quint16 id)
{
    deviceId = id;
}

void QHLMqtt::setloaclNetWork(QString localip, QString netmask, QString gateway)
{
    QProcess process;
    QString cmd = "ifconfig  eth0 ";
    cmd.append(localip);
    process.start(cmd);
    process.close();
    process.waitForFinished();

   cmd = "ifconfig eth0 netmask ";
   cmd.append(netmask);

    process.start(cmd);
    process.close();
    process.waitForFinished();

    cmd = "route add default gw ";
    cmd.append(gateway);

     process.start(cmd);
     process.close();
     process.waitForFinished();

    this->localip = localip;
    this->netmask = netmask;
    this->gateway = gateway;
}

void QHLMqtt::setMqttServerAddr(QString hostname, quint16 port)
{
    mqttHostName = hostname;
    mqttHostPort = port;
}

void QHLMqtt::setMqttSubscribeTopic(QString topic)
{
    mqttSubscribeTopic = topic;
}

void QHLMqtt::setMqttPublishTopic(QString topic)
{
    mqttPublishTopic = topic;
}

int QHLMqtt::onMqttConnectToServer()
{
    printf("\r\n ---  onMqttConnectToServer ---");
    if( m_client->state() == QMqttClient::Disconnected ){
        // if client is not connect server
        m_client->setHostname(mqttHostName);
        m_client->setPort(mqttHostPort);
        m_client->connectToHost();

        m_timer_publish->start(60000);// in order to up again

        return 1;
    }

    return 0;
}

int QHLMqtt::onMqttPublishDataToServer(const QByteArray message)
{
    return m_client->publish(mqttPublishTopic,message);
   // return m_client->publish(QMqttTopicName(QString("plock/dsn_123")),message);

}

int QHLMqtt::onMqttSubscribeDataToServer()
{
    if(m_client->state() == QMqttClient::Connected){
        m_client->subscribe(mqttSubscribeTopic);
        printf("\r\n ---  subscribe ok @---\r\n");
        mysleep(10);
        //TODO first push {"deviceid":2156,"type":"humiture","humidity":number1,"temperature":number2}
        QString pushdata = "{\"deviceid\":";
        pushdata.append(QString::number(deviceId));
        pushdata.append(",\"type\":\"humiture\",\"humidity\":");
        pushdata.append(QString::number(28.5));
        pushdata.append(",\"temperature\":");
        pushdata.append(QString::number(55.4));
        pushdata.append("}");

       // qDebug() << pushdata;
        printf("\r\n ---  publish ok ---\r\n");
        onMqttPublishDataToServer(pushdata.toUtf8());
        m_timer_publish->start(60000);
        printf("\r\n ---  onFtpConnectToServer ---\r\n");
        onFtpConnectToServer();

        return 0;
    }
    printf("\r\n ---  subscribe error ---\r\n");
    return -1;
}

int QHLMqtt::onFtpConnectToServer()
{
    // in order to mkdir
    m_ftp->connectToHost(mqttHostName,21);
    m_ftp->login("lock_test","Hold?fish:palm");
    m_ftp->mkdir(QString::number(deviceId));
    mysleep(1000);
    m_ftp->close();
}

int QHLMqtt::onFtpPutFileToServer(QString fileName)
{


    QUrl url;
    url.setScheme("ftp");
    url.setHost(mqttHostName);
    url.setPort(21);
    url.setUserName("lock_test");
    url.setPassword("Hold?fish:palm");

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    qDebug() << file.isOpen();
    QByteArray data = file.readAll();

    QString filename = "/" +  QString::number(deviceId) + "/" +  QString::number(lockLogId) + "/";
    filename.append(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss.zzz"));
    filename.append(".jpg");
    url.setPath(filename);
    manager->put(QNetworkRequest(url),data);

}

int QHLMqtt::onV4l2GetJgp()
{
   QString qpath = QCoreApplication::applicationDirPath();
   V4l2GetJpg("/dev/video0", qpath.toLatin1().data(),6);
}

int QHLMqtt::onOpenLock(int index)
{

    //表示已经开锁了，不需要再次开锁
    if(m_gpio->Read(1,15) == LOCK_OPENSTATE_LEVEL){
            QString pushData = "{\"deviceid\":";
            pushData.append( QString::number(deviceId) );
            pushData.append( ",\"type\":\"lock\",\"state\":\"opened\",\"index\":" );
            pushData.append( QString::number(index) );
            pushData.append(",\"lockLogId\":");
            pushData.append( QString::number(lockLogId) );
            pushData.append("}");

            onMqttPublishDataToServer(pushData.toUtf8());
            return 0;
      }



    bool isOpened = false;
    //TODO
    int i = 8;
    do{
        m_gpio->Write(1,12,1);
        mysleep(15);
        m_gpio->Write(1,12,0);
        mysleep(250);
        if(m_gpio->Read(1,15) == LOCK_OPENSTATE_LEVEL ){
            // opened
            i = 0;
            isOpened = true;
        }else{
            i--;
        }
    }while(i);

    if(!isOpened){
        QString pushData = "{\"deviceid\":";
        pushData.append( QString::number(deviceId) );
        pushData.append( ",\"type\":\"lock\",\"state\":\"holdClose\",\"index\":" );
        pushData.append( QString::number(lockIndex) );
        pushData.append(",\"lockLogId\":");
        pushData.append( QString::number(lockLogId) );
        pushData.append("}");


          onMqttPublishDataToServer(pushData.toUtf8());


        return -1;
    }

    QString pushData = "{\"deviceid\":";
       pushData.append( QString::number(deviceId) );
       pushData.append( ",\"type\":\"lock\",\"state\":\"alreadyOpen\",\"index\":" );
       pushData.append( QString::number(index) );
       pushData.append(",\"lockLogId\":");
       pushData.append( QString::number(lockLogId) );
       pushData.append("}");
       onMqttPublishDataToServer(pushData.toUtf8());

    // camera timer
       isCameraed = false;
    m_timer_camera->start(500);
}

void QHLMqtt::onSetSerialCom()
{
    m_SerialPort->setPortName("ttyO1");
    if(m_SerialPort->open(QIODevice::ReadWrite))
    {
        printf("-------%s\n", "serial ttyO1 open success");

        m_SerialPort->setBaudRate(115200);
        //设置数据位数
        m_SerialPort->setDataBits(QSerialPort::Data8);
        //设置奇偶校验
        m_SerialPort->setParity(QSerialPort::NoParity);
        //设置停止位
        m_SerialPort->setStopBits(QSerialPort::OneStop);
        //设置流控制
        m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);
    }
    else
    {
        printf("!!!!!!!!!!serial ttyo1 open failed!!!!!!!!!!\n");
    }
}

uint8_t QHLMqtt::AM2302_ReadByte()
{
    uint8_t i, temp=0,j;
    for(i=0;i<8;i++)
        {
            /*每bit以50us低电平标置开始，轮询直到从机发出 的50us 低电平 结束*/
            while( m_gpio->Read(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER)  == 0);

            /*AM2302 以26~28us的高电平表示“0”，以70us高电平表示“1”，
             *通过检测 x us后的电平即可区别这两个状 ，x 即下面的延时
             */
            AM2302_Delay(40); //延时x us 这个延时需要大于数据0持续的时间即可

            if(m_gpio->Read(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER)  == 1)/* x us后仍为高电平表示数据“1” */
            {
                /* 等待数据1的高电平结束 */
                while(m_gpio->Read(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER)==1);

                temp|=(uint8_t)(0x01<<(7-i));  //把第7-i位置1，MSB先行
            }
            else	 // x us后为低电平表示数据“0”
            {
                temp&=(uint8_t)~(0x01<<(7-i)); //把第7-i位置0，MSB先行
            }
        }
        return temp;
}

uint8_t QHLMqtt::AM2302_Read_TempAndHumidity(AM2302_Data_TypeDef *AM2302_Data)
{
    uint8_t temp,j;
    uint16_t humi_temp;

    /*输出模式*/
    m_gpio->AM2302_Mode_OUT();
    /*主机拉低*/
    m_gpio->Write(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER,0);
    /*延时18ms*/
    mysleep(18);
    /*总线拉高 主机延时30us*/
    m_gpio->Write(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER,1);

    AM2302_Delay(30);   //延时30us

    /*主机设为输入 判断从机响应信号*/
    m_gpio->AM2302_Mode_IPU();

    /*判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/
        if(m_gpio->Read(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER)==0)
        {
        /*轮询直到从机发出 的80us 低电平 响应信号结束*/

        while(m_gpio->Read(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER)==0);

        /*轮询直到从机发出的 80us 高电平 标置信号结束*/

        while(m_gpio->Read(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER)==1);

        /*开始接收数据*/
        AM2302_Data->humi_high8bit= AM2302_ReadByte();
        AM2302_Data->humi_low8bit = AM2302_ReadByte();
        AM2302_Data->temp_high8bit= AM2302_ReadByte();
        AM2302_Data->temp_low8bit = AM2302_ReadByte();
        AM2302_Data->check_sum    = AM2302_ReadByte();

        /*读取结束，引脚改为输出模式*/
        m_gpio->AM2302_Mode_OUT();
        /*主机拉高*/
        m_gpio->Write(GPIO_TEMP_GROUNP,GPIO_TEMP_NUMBER,1);

        /* 对数据进行处理 */
        humi_temp=AM2302_Data->humi_high8bit*256+AM2302_Data->humi_low8bit;
        AM2302_Data->humidity =(float)humi_temp/10;
        humi_temp=AM2302_Data->temp_high8bit*256+AM2302_Data->temp_low8bit;
        AM2302_Data->temperature=(float)humi_temp/10;

        /*检查读取的数据是否正确*/
        temp = AM2302_Data->humi_high8bit + AM2302_Data->humi_low8bit +
               AM2302_Data->temp_high8bit+ AM2302_Data->temp_low8bit;
        if(AM2302_Data->check_sum==temp)
        {
          return 1;
        }
        else
          return 0;
        }
        else
            return 0;
}

void QHLMqtt::AM2302_Delay(uint16_t time)
{
    uint8_t i;

      while(time)
      {
          for (i = 0; i < 10; i++)
        {

        }
        time--;
      }
}

void QHLMqtt::onMqttConnected()
{
    state_flag = 1;
    onMqttSubscribeDataToServer();
}

//mqtt disconnected
void QHLMqtt::onMqttDisconnected()
{
    printf("\r\n ---- onMqttDisconnected \r\n  ");
}

void QHLMqtt::showTime()
{
//    AM2302_Data_TypeDef AM2302_Data;
//    if(AM2302_Read_TempAndHumidity(&AM2302_Data)==1)
//   {
//     printf("读取AM2302成功!-->湿度为%.1f ％RH ，温度为 %.1f℃ \n",AM2302_Data.humidity,AM2302_Data.temperature);
//   }
//   else
//   {
//     printf("EEEERRRRRRRRRR读取AM2302失败!-->湿度为%.1f ％RH ，温度为 %.1f℃ \n",AM2302_Data.humidity,AM2302_Data.temperature);
//   }

    m_timer->stop();

    //m_ftp->state()


    if(m_client->state() == QMqttClient::Disconnected){
        // disconnected
        qDebug() << "Disconnected";

        m_timer->stop();
        m_timer_camera->stop();
        m_timer_publish->stop();

        m_client->disconnectFromHost();
        m_ftp->close();
        m_timer->start(3000);

        this->onMqttConnectToServer();

    }

}

void QHLMqtt::timerPublish()
{
    //TODO onMqttPublishDataToServer

   // m_timer_publish->stop();
   // onV4l2GetJgp();
   // qDebug() << "onV4l2GetJgp over";
   // onFtpPutFileToServer("0.jpg");
   // onFtpPutFileToServer("1.jpg");
  //  onFtpPutFileToServer("2.jpg");
//    onFtpPutFileToServer("3.jpg");

    if(m_client->state() == QMqttClient::Disconnected || state_flag == 0){
        // disconnected
        qDebug() << "Disconnected";

        m_timer->stop();
        m_timer_camera->stop();
        m_timer_publish->stop();

        m_client->disconnectFromHost();
        m_ftp->close();

        m_timer->start(3000);

        this->onMqttConnectToServer();

    }else{
        qDebug() << "timerPublish";
       onMqttPublishDataToServer("{\"deviceid\":2156,\"type\":\"humiture\",\"humidity\":26.5,\"temperature\":35.1}");
    }



}

void QHLMqtt::timerCamera()
{


    //timer camera
    m_timer_camera->stop();
    mysleep(100);

    //TODO no man_check

    if( m_gpio->Read(GPIO_MAN_GROUNP,GPIO_MAN_NUMBER) == 1 && !isCameraed){
        onV4l2GetJgp();

        //creat lockLogiD root
        m_ftp->close();
        mysleep(1000);
        m_ftp->connectToHost(mqttHostName,21);
        m_ftp->login("lock_test","Hold?fish:palm");
        m_ftp->cd(QString::number(deviceId));
        mysleep(1000);
        m_ftp->mkdir(QString::number(lockLogId));
        mysleep(1000);
        m_ftp->close();


        onFtpPutFileToServer("./0.jpg");
        mysleep(100);
        onFtpPutFileToServer("./1.jpg");
        mysleep(100);
        onFtpPutFileToServer("./2.jpg");
        mysleep(100);
        onFtpPutFileToServer("./3.jpg");
        mysleep(100);
        onFtpPutFileToServer("./4.jpg");
        mysleep(100);
        onFtpPutFileToServer("./5.jpg");
        mysleep(100);
        isCameraed = true;
        m_timer_camera->start(100);
    }else{
        if( m_gpio->Read(GPIO_LOCK_CHECK_GROUNP,GPIO_LOCK_CHECK_NUMBER) == LOCK_CLOSESTATE_LEVEL){
            // if room is closed
            printf("\r\n close the room\r\n");
            QString pushData = "{\"deviceid\":";
            pushData.append( QString::number(deviceId) );
            pushData.append( ",\"type\":\"lock\",\"state\":\"alreadyClose\",\"index\":" );
            pushData.append( QString::number(lockIndex) );
            pushData.append(",\"lockLogId\":");
            pushData.append( QString::number(lockLogId) );
            pushData.append("}");
            onMqttPublishDataToServer(pushData.toUtf8());

        }else{
            m_timer_camera->start(100);
        }
    }

}

void QHLMqtt::timerFeedDog()
{
    printf("\r\n timerFeedDog \r\n");
    m_watchdog->feedDog();
}

void QHLMqtt::onMqttSubscribeReceive(const QByteArray &message,
                                     const QMqttTopicName &topic)
{
    //TODO
    //qDebug() << "message:" << message;

    const char * str = message.data();
    cJSON *root = cJSON_Parse(str);
    if(root != NULL){
        //type
        QString type =  cJSON_GetObjectItem(root,"type")->valuestring;
        if(type == "lock"){
            int index = cJSON_GetObjectItem(root,"index")->valueint;
            //TODO open the index lock
            qDebug() << "open lock the index = " << index;

            lockIndex = index;
            lockLogId = cJSON_GetObjectItem(root,"lockLogId")->valueint;

            onOpenLock(index);

        }else if(type == "setip"){
            //TODO
        }
    }

}

void QHLMqtt::on_comserial_read()
{
    //
    QByteArray requestData = m_SerialPort->readAll();
    receiveDataappend.append(requestData);
    qDebug() << " on_comserial_read "<< receiveDataappend;

    //TODO check "open"
    if( receiveDataappend.contains("open") ){
        receiveDataappend.clear();
        int i = 8;
        do{
            m_gpio->Write(1,12,1);
            mysleep(15);
            m_gpio->Write(1,12,0);
            mysleep(250);
            if(m_gpio->Read(1,15) == LOCK_OPENSTATE_LEVEL ){
                // opened
                i = 0;
            }else{
                i--;
            }
        }while(i);
    }
    // to be
}


