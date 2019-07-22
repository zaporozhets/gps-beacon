/*******************************************************************************
* @brief    App for receiving BLE advertising data and drawing them on the map
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 23, 2019
*******************************************************************************/
import QtQuick 2.12
import QtLocation 5.12

MapQuickItem {
    id: marker
    anchorPoint.x: image.width / 4
    anchorPoint.y: image.height

    sourceItem: Image {

        id: image
        source: "qrc:/marker.png"
    }
}
