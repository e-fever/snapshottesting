import QtQuick 2.0
import QtQuick.Controls 1.2
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

    ColumnLayout {
        id: dualImage
        anchors.fill: parent
        visible: false
        enabled: false

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: 40

             ExclusiveGroup { id: displayMode }
             RadioButton {
                 text: "Side by Side"
                 checked: true
                 exclusiveGroup: displayMode
             }

             RadioButton {
                 text: "Combined"
                 checked: false
                 exclusiveGroup: displayMode
             }
        }

        RowLayout {
            anchors.fill: parent

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

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ScaleToFitImage {
                    anchors.fill: parent
                    anchors.margins: 4
                    source: "data:image/png;base64," + screenshot
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
                enabled: true
            }
        },
        State {
            name: "CombinedMode"
        }
    ]

}
