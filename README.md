Snapshot Testing
================

This project is a library to offer snapshot testing as a tool to make sure your UI does not change unexpectedly. It is inspired by Jest snapshot testing module but it is designed for QML.

Quoted from [Snapshot Testing · Jest](https://facebook.github.io/jest/docs/snapshot-testing.html) :

>> A typical snapshot test case for a mobile app renders a UI component, takes a screenshot, then compares it to a reference image stored alongside the test. The test will fail if the two images do not match: either the change is unexpected, or the screenshot needs to be updated to the new version of the UI component.
>> A similar approach can be taken when it comes to testing your React components. Instead of rendering the graphical UI, which would require building the entire app, you can use a test renderer to quickly generate a serializable value for your React tree.

The concept of this project is similar, but it replaces React component by a QObject/QQuickItem instance then convert to a text representation looks similar to QML. Let’s see a demonstration:

```QML
import QtQuick 2.0
import QtTest 1.0
import SnapshotTesting 1.0

Item {
    id: root
    width: 640
    height: 480

    CustomItem {
        // Don't place this under TestCase object.
        id: item1
        width: 320
        height: 240
        anchors.centerIn: parent
    }

    TestCase {
        name: "Demo1"
        when: windowShown

        function test_demo1() {
            var snapshot = SnapshotTesting.capture(item1); // Capture "item1" into a text representation
            snapshot = snapshot.replace(Qt.resolvedUrl(".."), "");
            SnapshotTesting.matchStoredSnapshot("test_demo1", snapshot); // Compare with previously stored snapshot
        }
    }
}

```

CustomItem.qml

```QML
import QtQuick 2.0
import QtQuick.Layouts 1.3

Item {
    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#000000"
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#FF0000"
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#FFCC00"
        }
    }
}
```

In the first time execution, it has no any previously saved snapshot. It will prompt a UI and ask for confirmation of applying the changes to snapshotsFile by `SnapshotTesting.matchStoredSnapshot`.

![snapshottesting-1.png (1159×552)](https://raw.githubusercontent.com/benlau/junkcode/master/docs/snapshottesting-1.png)

Once the snapshots file is created, the UI will not prompt again unless there have changed. For example, if it changes item height from 320 to 180. And run the porgramme again. It will show:

![snapshottesting-2.png (655×549)](https://raw.githubusercontent.com/benlau/junkcode/master/docs/snapshottesting-2.png)

Text Representation
-------------------

SnapshotTesting converts a QObject/QQuickItem to a text representation like QML. But it will remove all the data binding/anchors and show the real coordination information. It will also remove non-visible items by default. In case you wish to show non-visual component in your snapshot. You should pass captureVisibleItemOnly to false in the capture() call.

```
    var snapshot = SnapshotTesting.capture(item, {captureVisibleItemOnly: false });
```


UI Gallery
----------

Quoted from :[The Five Key Mindsets to Master If You Want to Be a Successful Programmer](https://www.effectiveengineer.com/blog/five-key-skills-of-successful-programmers)

>> Or suppose that you’re fixing a bug that requires you to start the app and then navigate through five screens to set up the right conditions to trigger the bug. Could you spend 10 minutes to wire it up so that it goes to the buggy screen on startup?

The debugging technique mentioned above is very useful. But there has a problem. How do you manage the code of short cut? If it is not saved in version control, then you have to patch your code for every time you found a new bug.

However, it is quite difficult to put the short cut code in your application. It will add a lot of `#ifdef` condition that you don’t want to handle it.

The best place to hold the code is the unit test problem with using SnapshotTesting. You could simulate a specific condition with chosen UI component. The “SnapshotTesting.matchStoredSnapshot()” will only pause the test case but not the UI. So you could evaluate your UI until you have pressed “Yes” to confirm the behaviour.

This kind of technique has no name. Personally, I would call it as gallery tests. As it will collects a set of UI in different condition finally.

Installation
------------

```
    qpm install net.efever.snapshottesting
```

Examples
--------

An example program is available in the [examples](https://github.com/e-fever/snapshottesting/tree/master/examples/example1) folder within the source code.

QML API
---

**SnapshotTesting.snapshotsFile[Property]**

It is a property to holder which file to save/load snapshots. It is recommended to set this property in main.cpp by the C++ API

**String SnapshotTesting.capture(object, options)**


**SnapshotTesting.matchStoredSnapshot(name, snapshot)**

C++ API
-------

TODO

FAQ
----

**Should snapshots file be committed?**

Yes.

**Does snapshot testing substitute unit testing?**

**How do I resolve the conflict within snapshots file?**
