/*******************************************************************************
* @brief    App for receiving BLE advertising data and drawing them on the map
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 23, 2019
*******************************************************************************/
#include "AdvReceiver.h"
#include <QDebug>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

/**
 * @brief
 *
 */
enum : uint8_t {
    EIR_FLAGS = 0x01, /* flags */
    EIR_UUID16_SOME = 0x02, /* 16-bit UUID, more available */
    EIR_UUID16_ALL = 0x03, /* 16-bit UUID, all listed */
    EIR_UUID32_SOME = 0x04, /* 32-bit UUID, more available */
    EIR_UUID32_ALL = 0x05, /* 32-bit UUID, all listed */
    EIR_UUID128_SOME = 0x06, /* 128-bit UUID, more available */
    EIR_UUID128_ALL = 0x07, /* 128-bit UUID, all listed */
    EIR_NAME_SHORT = 0x08, /* shortened local name */
    EIR_NAME_COMPLETE = 0x09, /* complete local name */
    EIR_TX_POWER = 0x0A, /* transmit power level */
    EIR_DEVICE_ID = 0x10, /* device ID */
};

/**
 * @brief
 *
 * @param parent
 */
AdvReceiver::AdvReceiver(QObject* parent)
    : QObject(parent)
{
    uint8_t scan_type = 0x00; /* Passive */

    uint8_t own_type = 0x00;
    uint8_t filter_policy = 0x00;
    uint16_t interval = htobs(0x0010);
    uint16_t window = htobs(0x0010);

    int dev_id = hci_get_route(nullptr);

    m_dd = hci_open_dev(dev_id);
    if (m_dd < 0) {
        throw std::system_error(errno, std::system_category(), "Could not open HCI device");
    }

    int retval = hci_le_set_scan_parameters(m_dd, scan_type, interval, window, own_type, filter_policy, 1000);
    if (retval < 0) {
        throw std::system_error(errno, std::system_category(), "Set scan parameters failed");
    }

    retval = hci_le_set_scan_enable(m_dd, 0x01, 1, 1000);
    if (retval < 0) {
        throw std::system_error(errno, std::system_category(), "Enable scan failed");
    }

    m_advReveiver = std::thread(&AdvReceiver::advReveiver, this);
}

/**
 * @brief
 *
 */
AdvReceiver::~AdvReceiver()
{

    m_terminate = true;
    m_advReveiver.join();

    int retval = hci_le_set_scan_enable(m_dd, 0x00, 1, 1000);
    if (retval < 0) {
        qDebug() << "Disable scan failed";
    }

    hci_close_dev(m_dd);
}

/**
 * @brief Thread function for receiving
 *
 */
void AdvReceiver::advReveiver()
{
    struct hci_filter nf, of;

    socklen_t olen = sizeof(of);
    if (getsockopt(m_dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
        qDebug() << "Could not get socket options\n";
        return;
    }

    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(m_dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
        qDebug() << "Could not set socket options!";
        return;
    }

    while (!m_terminate) {
        ssize_t len;
        uint8_t buf[HCI_MAX_EVENT_SIZE];
        while ((len = read(m_dd, buf, sizeof(buf))) < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            break;
        }

        uint8_t* ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
        len -= (1 + HCI_EVENT_HDR_SIZE);

        // Ignoring
        auto meta = reinterpret_cast<evt_le_meta_event*>(ptr);
        if (meta->subevent != 0x02) {
            break;
        }

        // Ignoring multiple reports
        auto info = reinterpret_cast<le_advertising_info*>(meta->data + 1);
        // We also can retrieve MAC address: ba2str(&info->bdaddr, addr);
        auto name = getEirName(info->data, info->length);

        // Filter by NAME
        if ("tracker" == name) {
            struct PositionDataStr {
                int32_t latitude;
                int32_t longitude;
            } __attribute__((packed));

            auto adv = reinterpret_cast<PositionDataStr*>(&info->data[7]);
            double latitude = convertToDeg(adv->latitude);
            double longitude = convertToDeg(adv->longitude);
            emit positionReveived(latitude, longitude);
        }
    }

    setsockopt(m_dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));
}

/**
 * @brief Convert a raw coordinate to a floating point DD.DDD... value.
 *
 * @param value
 * @return double
 */
double AdvReceiver::convertToDeg(int32_t value)
{
    int32_t degrees = value / (100 * 1000);
    int32_t minutes = value % (100 * 1000);
    return static_cast<double>(degrees) + static_cast<double>(minutes) / (60 * 1000);
}

/**
 * @brief Finds and returns name in advertising packet
 *
 * @param eir pointer to advertising packet
 * @param eir_len packet len
 * @return std::string
 */
std::string AdvReceiver::getEirName(const uint8_t* eir, size_t eir_len)
{
    size_t offset = 0;

    while (offset < eir_len) {
        uint8_t field_len = eir[0];

        /* Check for the end of EIR */
        if (0 == field_len) {
            break;
        }

        if (offset + field_len > eir_len) {
            break;
        }

        switch (eir[1]) {
        case EIR_NAME_SHORT:
        case EIR_NAME_COMPLETE:
            return std::string(&eir[2], &eir[2] + field_len - 1);
        }

        offset += field_len + 1;
        eir += field_len + 1;
    }
    return "(unknown)";
}
