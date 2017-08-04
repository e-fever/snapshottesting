import QtQuick 2.0
import QtTest 1.0
import SnapshotTesting 1.0

Item {
    id: root
    width: 640
    height: 480

    Item {
        // Don't place this under TestCase object.
        id: item1
    }

    TestCase {
        name: "Demo1"
        when: windowShown

        function test_demo1() {
            var snapshot = SnapshotTesting.capture(item1);
            snapshot = snapshot.replace(Qt.resolvedUrl(".."), "");
            SnapshotTesting.matchStoredSnapshot("test_demo1", snapshot);
        }

    }

}
