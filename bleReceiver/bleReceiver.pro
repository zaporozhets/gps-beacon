QT += quick network positioning location widgets


SOURCES += \
    main.cpp \
    AdvReceiver.cpp

HEADERS += \
    AdvReceiver.h

LIBS += \
    -lbluetooth

RESOURCES += \
    BleReceiver.qrc


target.path = /usr/sbin
INSTALLS += target




