import QtQuick 2.0

Item {
    id: sampleLoaderAsync

    Loader {
        id: loader

        asynchronous: true

        source: Qt.resolvedUrl("./Sample1.qml")
    }

}
