/*******************************************************************************
* @brief    App for receiving BLE advertising data and drawing them on the map
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 23, 2019
*******************************************************************************/
import QtQuick 2.9
import QtPositioning 5.12
import QtLocation 5.12
import QtQuick.Controls 2.3

import AdvReceiver 1.0

ApplicationWindow {
    visible: true
    title: qsTr("Coordinates Receiver")
    width: 640
    height: 640
    id: window

    AdvReceiver {
        id: receiver
        onPositionReveived: {
            console.debug("latitude: " + latitude)
            console.debug("longitude: " + longitude)
            map.addMarker(latitude, longitude)
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button & Qt.RightButton) {
                map.clearMarkers()
            }
        }
    }

    Plugin {
        id: osmPlugin
        name: "osm" // "mapboxgl", "esri", ...
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: osmPlugin
        center: centerLocation
        zoomLevel: 14

        property variant centerLocation: QtPositioning.coordinate(0, 0)
        property bool updateCenterLocation: true

        function addMarker(latitude, longitude) {
            var marker = Qt.createQmlObject('Marker {}', map)
            map.addMapItem(marker)
            marker.z = map.z + 1
            marker.coordinate = QtPositioning.coordinate(latitude, longitude)

            if (updateCenterLocation) {
                centerLocation = QtPositioning.coordinate(latitude, longitude)
                updateCenterLocation = false
            }
        }
        function clearMarkers() {
            map.clearMapItems()
            map.updateCenterLocation = true
        }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:600;width:1024}
}
 ##^##*/

