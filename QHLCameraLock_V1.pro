QT += core network
QT -= gui
#QT += script
CONFIG += c++11

TARGET = QHLCameraLock_V1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    QHLMqtt.cpp \
    cJSON.c \
    qftp.cpp \
    qurlinfo.cpp \
    V4l2CameraControl.cpp \
    gpio.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    QHLMqtt.h \
    cJSON.h \
    qftp.h \
    qurlinfo.h \
    V4l2CameraControl.h \
    gpio.h







#INCLUDEPATH += $$PWD/../Mqtt
#DEPENDPATH += $$PWD/../Mqtt


unix:!macx: LIBS += -L$$PWD/../../../QtMqttCode/mqtt_pc_lib/lib/ -lQt5Mqtt

INCLUDEPATH += /home/qhl/QtMqttCode/qtmqtt_arm/include
DEPENDPATH += $$PWD/../../../QtMqttCode/mqtt_pc_lib/include

DISTFILES += \
    Readme.txt


