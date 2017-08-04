QT       += testlib qml

TARGET = snapshottesting
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp     tests.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$PWD/\\\"

ROOTDIR = $$PWD/../../

include(vendor/vendor.pri)
include($$ROOTDIR/snapshottesting.pri)

DISTFILES +=     qpm.json     qmltests/tst_QmlTests.qml

HEADERS +=     tests.h
