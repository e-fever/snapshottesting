#include <QTest>
#include <QtShell>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QJsonDocument>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <aconcurrent.h>
#include <private/qqmldata_p.h>
#include <private/qqmlcontext_p.h>
#include "automator.h"
#include "testcases.h"
#include "testablefunctions.h"
#include "snapshottesting.h"
#include "private/snapshottesting_p.h"

using namespace SnapshotTesting;
using namespace SnapshotTesting::Private;

Testcases::Testcases(QObject *parent) : QObject(parent)
{
    auto ref = [=]() {
        QTest::qExec(this, 0, 0); // Autotest detect available test cases of a QObject by looking for "QTest::qExec" in source code
    };
    Q_UNUSED(ref);
}

void Testcases::init()
{
    {
        // Make sure the QtQuick package is loaded

        QQmlEngine engine;
        createQmlComponent(&engine, "Item", "QtQuick", 2 , 0)->deleteLater();
    }
}

void Testcases::test_obtainQmlPackage()
{
    QQuickItem* item = new QQuickItem();
    QString package = obtainQmlPackage(item);

    QCOMPARE(package, QString("QtQuick"));
    delete item;
}

void Testcases::test_obtainDynamicDefaultValues()
{
    QQuickItem* item = new QQuickItem();

    item->setX(100);
    QVariantMap defaultValues = obtainDynamicDefaultValues(item);

    QVERIFY(defaultValues.contains("x"));
    QCOMPARE(defaultValues["x"].toInt(), 0);

    delete item;

}

void Testcases::test_classNameToComponentName()
{
    QCOMPARE(classNameToComponentName("AnyOtherClass"), QString("AnyOtherClass"));
    QCOMPARE(classNameToComponentName("AnyOtherClass_QML_123"), QString("AnyOtherClass"));
    QCOMPARE(classNameToComponentName("QQuickItem"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickItem_QML_123"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickItem_QML_4523"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickText"), QString("Text"));

    {
        QQmlEngine engine;
        QObject* object = createQmlComponent(&engine, "Canvas", "QtQuick", 2, 0);

        QCOMPARE(classNameToComponentName(object->metaObject()->className()), QString("Canvas"));
        object->deleteLater();
    }

}

void Testcases::test_context()
{
    QQmlApplicationEngine engine;

    auto lsContext = listContextUrls;

    {
        QObject* object = createQmlComponent(&engine, "Button", "QtQuick.Controls", 2, 0);
        QVERIFY(object);
        QVERIFY(lsContext(object).size() > 0);
        object->deleteLater();
    }

    {
        QObject* object = createQmlComponent(&engine, "Item", "QtQuick", 2, 0);
        QVERIFY(object);
        QVERIFY(lsContext(object).size()  == 0);
        object->deleteLater();
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample1.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);
        QQmlContext* context = qmlContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample1"));

        context = SnapshotTesting::Private::obtainCreationContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample1"));

        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Item"));

        QVERIFY(obtainCurrentScopeContext(object) == qmlContext(object));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample5.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);
        QQmlContext* context = qmlContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample5"));

        context = SnapshotTesting::Private::obtainCreationContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample5Form"));

        QVERIFY(obtainCurrentScopeContext(object) == qmlContext(object));
        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Sample5Form"));

    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample2.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QQuickItem* child = object->findChild<QQuickItem*>("item_sample1");

        QVERIFY(obtainCurrentScopeContext(child) != qmlContext(child));
        QCOMPARE(obtainRootComponentName(object), QString("Item"));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample6.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseContext(object), QString("Sample5Form"));

        QCOMPARE(obtainComponentNameByBaseUrl(obtainCurrentScopeContext(object)->baseUrl()), QString("Sample6"));

        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Sample5"));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample7.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QCOMPARE(obtainComponentNameByClass(object), QString("Sample7"));

        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Item"));
        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object, true), QString("Item"));
    }
}

void Testcases::test_loading_config()
{
    {
        QString text = QtShell::cat(":/qt-project.org/imports/SnapshotTesting/config/snapshot-config.json");

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(),&error);
        Q_UNUSED(doc);

        QVERIFY(error.error == QJsonParseError::NoError);
    }
}

void Testcases::test_grabImage()
{
    if (Testable::isCI()) {
        qDebug() << "Skip this test in CI environment";
        return;
    }

    QQuickWindow window;
    QQmlEngine engine;
    QObject* object = createQmlComponent(&engine, "Item", "QtQuick", 2, 0);

    QQuickItem *item = qobject_cast<QQuickItem*>(object);
    QVERIFY(item);
    item->setWidth(200);
    item->setHeight(300);

    item->setParentItem(window.contentItem());
    window.show();

    Automator::wait(2000);

    QFuture<QImage> future = grabImage(item);

    AConcurrent::await(future);

    QImage image = future.result();

    QCOMPARE(image.size(), QSize(200,300));
}

void Testcases::test_render()
{
    if (Testable::isCI()) {
        qDebug() << "Skip this test in CI environment";
        return;
    }

    qDebug() << "test_render";

    QFuture<QImage> future = SnapshotTesting::Private::render(QtShell::realpath_strip(SRCDIR, "sample/Sample2.qml"));

    AConcurrent::await(future, 3000);

    QVERIFY(future.resultCount() > 0);

    QImage image = future.result();

    QCOMPARE(image.size(), QSize(640,480));

    image.save(QtShell::realpath_strip(QTest::currentTestFunction()) + ".jpg");

}

void Testcases::test_SnapshotTesting_diff()
{
    QString text1 = "A\nB\nC";
    QString text2 = "A\nD\nC";

    QString result = SnapshotTesting::diff(text1, text2);

    qDebug().noquote() << result;
}

void Testcases::test_SnapshotTesting_saveSnapshots()
{
    SnapshotTesting::saveSnapshots();
}

void Testcases::test_SnapshotTesting_addClassIgnoredProperty()
{
    QString input = QtShell::realpath_strip(SRCDIR, "sample/Sample1.qml");

    QQmlApplicationEngine engine;
    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    QString name, text;

    name = QString("%1_default").arg(QTest::currentTestFunction());

    text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));

    SnapshotTesting::addClassIgnoredProperty("QQuickRectangle", "width");
    name = QString("%1_set").arg(QTest::currentTestFunction());

    text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));

    SnapshotTesting::removeClassIgnoredProperty("QQuickRectangle", "width");
    name = QString("%1_default").arg(QTest::currentTestFunction());

    text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void Testcases::test_SnapshotTesting_capture_QObject()
{
    QObject object;

    QString snapshot = SnapshotTesting::capture(&object);

    QCOMPARE(snapshot, QString(""));
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;
    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_data()
{
    scanSamples();
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_expandAll()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;

    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    SnapshotTesting::Options options;
    options.expandAll = true;
    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem, options);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_expandAll_data()
{
    scanSamples();
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_hideId()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;

    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    SnapshotTesting::Options options;
    options.hideId = true;
    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem, options);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_hideId_data()
{
    scanSamples();
}

void Testcases::scanSamples()
{
    QTest::addColumn<QString>("input");

    QStringList files = QtShell::find(QtShell::realpath_strip(SRCDIR, "sample"), "*.qml");

    foreach (QString file, files) {
        QTest::newRow(file.toUtf8().constData()) << file;
    }
}
