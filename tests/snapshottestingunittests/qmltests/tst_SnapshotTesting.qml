import QtQuick 2.0
import QtTest 1.0
import SnapshotTesting 1.0
import "../sample"

Item {
    width: 640
    height: 480

    Sample1 {
        id: sample1
    }

    Sample5 {
        id: sample5
    }

    TestCase {
        name: "SnapshotTesting"
        when: windowShown

        function test_capture_sample1() {
            var snapshot = SnapshotTesting.capture(sample1);
            snapshot = snapshot.replace(Qt.resolvedUrl(".."), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_sample1", snapshot);
        }

        function test_capture_sample5() {
            var snapshot = SnapshotTesting.capture(sample5);
            snapshot = snapshot.replace(Qt.resolvedUrl(".."), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_sample5", snapshot);
        }

    }

}
