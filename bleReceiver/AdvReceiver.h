/*******************************************************************************
* @brief    App for receiving BLE advertising data and drawing them on the map
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 23, 2019
*******************************************************************************/
#pragma once

#include <QObject>
#include <thread>
class AdvReceiver : public QObject {
    Q_OBJECT
public:
    explicit AdvReceiver(QObject* parent = nullptr);
    ~AdvReceiver();

signals:
    void positionReveived(double latitude, double longitude);

private:
    static std::string getEirName(const uint8_t* eir, size_t eir_len);
    double convertToDeg(uint32_t value);
    void advReveiver(void);

    bool m_terminate = false;
    std::thread m_advReveiver;
    int m_dd = -1;
};
