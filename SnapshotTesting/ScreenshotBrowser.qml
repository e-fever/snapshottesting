import QtQuick 2.0
import QtQuick.Layouts 1.0

Item {
    id: component

    property string screenshot: ""

    property string previousScreenshot: ""

    property string combinedScreenshot: ""

    ScaleToFitImage {
        id: singleImage
        anchors.fill: parent
        source: "data:image/png;base64," + screenshot
    }

    Item {
        id: dualImage
        anchors.fill: parent
        visible: false

        RowLayout {
            anchors.fill: parent

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                ScaleToFitImage {
                    anchors.fill: parent
                    anchors.margins: 4
                    source: "data:image/png;base64," + screenshot
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ScaleToFitImage {
                    anchors.fill: parent
                    anchors.margins: 4
                    source: {
                        if (previousScreenshot === "") {
                            return "";
                        }

                        return "data:image/png;base64," + previousScreenshot;
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "SingleMode"
        },
        State {
            name: "DualMode"
            when: previousScreenshot !== ""

            PropertyChanges {
                target: singleImage
                visible: false
            }

            PropertyChanges {
                target: dualImage
                visible: true
            }
        },
        State {
            name: "CombinedMode"
        }
    ]

}
