/*******************************************************************************
* @brief    App for receiving BLE advertising data and drawing them on the map
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 23, 2019
*******************************************************************************/
#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>

#include "AdvReceiver.h"

int main(int argc, char** argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    QString extraImportPath(QStringLiteral("%1/../../../../%2"));

    engine.addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(),
        QString::fromLatin1("qml")));

    qmlRegisterType<AdvReceiver>("AdvReceiver", 1, 0, "AdvReceiver");

    engine.load(QUrl(QLatin1String("qrc:/BleReceiver.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
