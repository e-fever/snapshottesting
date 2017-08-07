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

    Sample2 {
        id: sample2
    }

    Sample5 {
        id: sample5
    }

    Sample6 {
        id: sample6
    }

    TestCase {
        name: "SnapshotTesting"
        when: windowShown

        function test_capture_sample1() {
            var snapshot = SnapshotTesting.capture(sample1);
            snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_sample1", snapshot);
        }

        function test_capture_sample2() {
            var snapshot;
            snapshot = SnapshotTesting.capture(sample2);
            snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_sample2", snapshot);

            snapshot = SnapshotTesting.capture(sample2 , {expandAll: true});
            snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_expandAll_sample2", snapshot);
        }

        function test_capture_sample5() {
            var snapshot = SnapshotTesting.capture(sample5);
            snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_sample5", snapshot);
        }

        function test_capture_sample6() {
            var snapshot;
            snapshot = SnapshotTesting.capture(sample6);
            snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_sample6", snapshot);

            snapshot = SnapshotTesting.capture(sample6 , {expandAll: true});
            snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
            SnapshotTesting.matchStoredSnapshot("qml_test_capture_expandAll_sample6", snapshot);
        }

    }

}
