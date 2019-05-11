import QtQuick 2.0

Grid {
    Item {
        id: item
        width: 100
        height: 100
    }

    Image {
        id: image
        source: "../sample/red-100x100.png"
    }

    MouseArea {
        width: 100
        height: 100
    }
}
