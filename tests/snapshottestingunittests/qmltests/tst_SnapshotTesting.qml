import QtQuick 2.0
import QtTest 1.0
import SnapshotTesting 1.0
import "../sample"

Item {
    width: 640
    height: 480

    Sample1 {
        id: item1
    }

    TestCase {
        name: "SnapshotTesting"
        when: windowShown

        function test_capture() {
            var snapshot = SnapshotTesting.capture(item1);
            console.log(snapshot);
            snapshot = snapshot.replace(Qt.resolvedUrl(".."), "");
            SnapshotTesting.matchStoredSnapshot("test_capture", snapshot);
        }

    }

}
