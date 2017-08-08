#include <QDebug>
#include <QtShell>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStack>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QBuffer>
#include "snapshottesting.h"
#include <private/qqmldata_p.h>
#include <private/qqmlcontext_p.h>
#include <private/snapshottesting_p.h>
#include <functional>

/* For dtl */
using namespace std;
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include "dtl/Sequence.hpp"
#include "dtl/Lcs.hpp"
#include "dtl/variables.hpp"
#include "dtl/functors.hpp"
#include "dtl/Ses.hpp"
#include "dtl/Diff.hpp"

static QString m_snapshotFile;
static QVariantMap m_snapshots;
static bool m_snapshotsDirty = false;
static bool m_interactiveEnabled = true;
static bool m_ignoreAll = false;
static bool m_acceptAll = false;

static QStringList knownComponentList;
static QMap<QString,QString> classNameToItemNameTable;

static QMap<QString, QVariantMap> defaultValueMap;
static QMap<QString, QStringList> ignoreListMap;

#define DEHYDRATE_FONT(dest, property, original, current, field) \
    if (original.field() != current.field()) { \
        dest[property + "." + #field] = current.field(); \
    }

static QString removeDynamicClassSuffix(const QString &name) {
    QString res = name;
    QStringList list;

    list << "_QML_[0-9]+$" << "_QMLTYPE_[0-9]+$";

    foreach (QString pattern, list) {
        QRegExp rx(pattern);

        if (rx.indexIn(res) >= 0) {
            res = res.replace(rx, "");
        }
    }

    return res;
}

static QString obtainClassName(QObject* object) {
    const QMetaObject* meta = object->metaObject();
    return meta->className();
}

/// Obtain the class name of QObject which is known to the system
static QString obtainKnownClassName(QObject* object) {
  const QMetaObject* meta = object->metaObject();
  QString res;

  while (!classNameToItemNameTable.contains(res) && meta != 0) {
      res =  SnapshotTesting::Private::classNameToComponentName(meta->className());
      meta = meta->superClass();
  }

  return res;
}

QString SnapshotTesting::Private::classNameToComponentName(const QString &className)
{
    QString res = className;
    if (res.indexOf("QQuick") == 0) {
        res = res.replace("QQuick", "");
    }

    res = removeDynamicClassSuffix(res);

    return res;
}

QQmlContext* SnapshotTesting::Private::obtainCurrentScopeContext(QObject *object)
{
    QList<QQmlContext*> list;

    QQmlContext* context = obtainCreationContext(object);

    QQmlContext* result = 0;

    while (context != 0 && !context->baseUrl().isEmpty()) {
        list << context;
        context = context->parentContext();
    }

    if (list.size() == 0) {
        return 0;
    }

    if (object->parent() && list.size() > 0) {
        if (qmlContext(object->parent()) == list.last()) {
            list.takeLast();
        }
    }

    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    if (item && item->parentItem() && list.size() > 0) {
        if (qmlContext(item->parentItem()) == list.last()) {
            list.takeLast();
        }
    }

    if (list.size() > 0) {
        result = list.last();
    }

    return result;
}

QQmlContext *SnapshotTesting::Private::obtainCreationContext(QObject *object)
{
    QQmlContext* result = 0;
    QQmlData *ddata = QQmlData::get(object, false);
    if (ddata && ddata->context) {
        // obtain the inner context name
        result  = ddata->context->asQQmlContext();
    }
    return result;
}

QString SnapshotTesting::Private::obtainComponentNameByBaseUrl(const QUrl &baseUrl)
{
    QString path = QtShell::realpath_strip(baseUrl.toString());
    QFileInfo info(path);

    return info.baseName();
}

QString SnapshotTesting::Private::obtainComponentNameByBaseContext(QObject *object)
{
    QQmlContext* creationContext = SnapshotTesting::Private::obtainCreationContext(object);

    return SnapshotTesting::Private::obtainComponentNameByBaseUrl(creationContext->baseUrl());
}

QString SnapshotTesting::Private::obtainComponentNameByInheritedContext(QObject *object)
{
    QList<QUrl> urls;
    QQmlContext* creationContext = SnapshotTesting::Private::obtainCreationContext(object);
    QQmlContext* currentScopeContext = obtainCurrentScopeContext(object);
    QQmlContext* context = creationContext;

    while (context && context != currentScopeContext) {
        urls << context->baseUrl();
        context = context->parentContext();
    }

    QString res;

    if (urls.size() <= 0) {
        res = SnapshotTesting::Private::obtainComponentNameByClass(object);

        QString currentScopeContextName = obtainComponentNameByBaseUrl(currentScopeContext->baseUrl());

        if (res == currentScopeContextName) {
            res = obtainComponentNameByInheritedClass(object);
        }

    } else {
        res = SnapshotTesting::Private::obtainComponentNameByBaseUrl(urls.last());
    }
    return res;
}


QString SnapshotTesting::Private::obtainComponentNameByClass(QObject *object)
{
    QString result;
    QString className = obtainClassName(object);

    result = classNameToComponentName(className);

    if (result.isNull()) {
        QString knownClassName = obtainKnownClassName(object);

        result = classNameToItemNameTable[knownClassName];
    }
    return result;
}

QString SnapshotTesting::Private::obtainComponentNameByInheritedClass(QObject *object)
{
    QString result;

    const QMetaObject* meta = object->metaObject();
    if (meta->superClass()) {
        const QMetaObject* superClass = meta->superClass();
        result = classNameToComponentName(superClass->className());
    }

    return result;
}

QString SnapshotTesting::Private::obtainComponentNameByQuickClass(QObject *object)
{
    QString result;

    const QMetaObject* meta = object->metaObject();

    while (meta && !(result.indexOf("QQuick") == 0 || result.indexOf("QObject")  == 0 )  ) {
        result = meta->className();
        meta = meta->superClass();
    }

    result = classNameToComponentName(result);
    return result;
}


QString SnapshotTesting::Private::obtainRootComponentName(QObject *object, bool expandAll)
{
    QString res;
    if (expandAll) {
        res = obtainComponentNameByQuickClass(object);
    } else {
        res = SnapshotTesting::Private::obtainComponentNameByInheritedContext(object);
    }

    return res;
}

static bool inherited(QObject *object, QString className) {
    bool res = false;

    const QMetaObject *metaObject = object->metaObject();

    while (metaObject) {
        if (metaObject->className() == className) {
            res = true;
            break;
        }
        metaObject = metaObject->superClass();
    }

    return res;
}

static QVariantMap dehydrate(QObject* source, const SnapshotTesting::Options& options) {
    QString topLevelContextName;
    bool captureVisibleItemOnly = options.captureVisibleItemOnly;
    bool expandAll = options.expandAll;
    QList<QQmlContext*> topLevelContexts;
    QList<QUrl> topLevelBaseUrlList;

    {
        QQmlContext* context = qmlContext(source);
        if (context) {
            topLevelContexts << context;
            topLevelBaseUrlList<< context->baseUrl();
        }
        context = SnapshotTesting::Private::obtainCreationContext(source);
        if (context) {
            topLevelContexts << context;
            topLevelBaseUrlList<< context->baseUrl();
        }
    }

    auto obtainContextName = [=](QObject *object) {
        QString result;
        QQmlData *ddata = QQmlData::get(object, false);
        if (ddata && ddata->context) {
            // obtain the inner context name
            QUrl fileUrl = ddata->context->url();

            if (!fileUrl.isEmpty()) {
                result = SnapshotTesting::Private::obtainComponentNameByBaseUrl(fileUrl.toString());
            }
        }
        return result;
    };

    topLevelContextName = obtainContextName(source);

    auto obtainId = [=](QObject* object) -> QString {
        if (topLevelContexts.size() == 0) {
            return "";
        }
        QString res = topLevelContexts.first()->nameForObject(object);
        if (res.isEmpty()) {
            QQmlContext* context = qmlContext(object);
            if (context) {
                res = context->nameForObject(object);
            }
        }
        return res;
    };

    /// Obtain the item name in QML
    auto obtainItemName = [=,&topLevelContextName](QObject* object) {
        QString result;

        if (object == source) {
            return SnapshotTesting::Private::obtainRootComponentName(object, options.expandAll);
        }

        result = SnapshotTesting::Private::obtainComponentNameByQuickClass(object);

        if (!expandAll && object != source) {
            QString contextName = obtainContextName(object);
            if (contextName != topLevelContextName && contextName != "") {
                result = contextName;
            }
        }

        return result;
    };

    auto obtainDynamicGeneratedDefaultValuesMapByClassName = [=](QString className) {
        static QMap<QString, QVariantMap> autoDefaultValueMap;

        if (autoDefaultValueMap.contains(className)) {
            return autoDefaultValueMap[className];
        }

        QVariantMap res;

        if (className.indexOf("QQuick") != 0) {
            return res;
        }

        QString itemName = SnapshotTesting::Private::classNameToComponentName(className);

        QQmlApplicationEngine engine;

        QStringList packages;
        packages << "import QtQuick 2.0";
        packages << "import QtQuick.Layouts 1.1";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
        packages << "import QtQuick.Controls 2.0";
#endif

        QString qml  = QString("%2\n %1 {}").arg(itemName).arg(packages.join("\n"));

        QQmlComponent comp (&engine);
        comp.setData(qml.toUtf8(),QUrl());
        QObject* holder = comp.create();

        if (holder) {
            const QMetaObject* meta = holder->metaObject();

            for (int i = 0 ; i < meta->propertyCount(); i++) {

                const QMetaProperty property = meta->property(i);
                const char* name = property.name();

                QVariant value = holder->property(name);
                if (value.canConvert<QObject*>()) {
                    continue;
                }
                res[name] = value;
            }
            delete holder;
        } else {
            qDebug() << comp.errorString();
        }

        autoDefaultValueMap[className] = res;
        return res;
    };

    auto obtainDynamicGeneratedDefaultValuesMap = [=](QObject* object) {

        QString className = removeDynamicClassSuffix(obtainClassName(object));
        return obtainDynamicGeneratedDefaultValuesMapByClassName(className);
    };

    auto obtainDefaultValuesMap = [=](QObject* object) {
        const QMetaObject* meta = object->metaObject();
        QVariantMap result = obtainDynamicGeneratedDefaultValuesMap(object);

        QStringList classes;

        while (meta != 0) {
            QString className = meta->className();
            classes << removeDynamicClassSuffix(className);
            meta = meta->superClass();
        }

        while (classes.size() > 0) {
            QString className = classes.takeLast();
            QList<QVariantMap> pending;
            pending << obtainDynamicGeneratedDefaultValuesMapByClassName(className);
            if (defaultValueMap.contains(className)) {
                pending << defaultValueMap[className];
            }

            for (int i = 0 ; i < pending.size(); i++) {
                QVariantMap map = pending[i];
                QStringList keys = map.keys();
                foreach (QString key, keys) {
                    result[key] = map[key];
                }
            }
        }

        return result;
    };

    auto obtainIgnoreList = [=](QObject* object) {
        const QMetaObject* meta = object->metaObject();
        QStringList result;
        while (meta != 0) {
            QString className = meta->className();
            if (ignoreListMap.contains(className)) {
                QStringList list = ignoreListMap[className];
                result.append(list);
            }

            meta = meta->superClass();
        }

        return result;
    };

    auto _dehydrateFont = [=](QVariantMap& dest, QString property, QFont original , QFont current) {
        DEHYDRATE_FONT(dest,property,original,current, pixelSize);
        DEHYDRATE_FONT(dest,property,original,current, bold);
        DEHYDRATE_FONT(dest,property,original,current, capitalization);
        DEHYDRATE_FONT(dest,property,original,current, family);
        DEHYDRATE_FONT(dest,property,original,current, hintingPreference);
        DEHYDRATE_FONT(dest,property,original,current, italic);
        DEHYDRATE_FONT(dest,property,original,current, letterSpacing);

        DEHYDRATE_FONT(dest,property,original,current, pointSize);
        DEHYDRATE_FONT(dest,property,original,current, strikeOut);
        DEHYDRATE_FONT(dest,property,original,current, styleName);
        DEHYDRATE_FONT(dest,property,original,current, underline);
        DEHYDRATE_FONT(dest,property,original,current, weight);
        DEHYDRATE_FONT(dest,property,original,current, wordSpacing);
    };

    auto _dehydrate = [=](QObject* object) {

        QVariantMap dest;
        QVariantMap defaultValues = obtainDefaultValuesMap(object);
        QStringList ignoreList = obtainIgnoreList(object);

        const QMetaObject* meta = object->metaObject();

        QString id = obtainId(object);
        if (!id.isNull() && !options.hideId) {
            dest["id"] = id;
        }

        for (int i = 0 ; i < meta->propertyCount(); i++) {
            const QMetaProperty property = meta->property(i);
            const char* name = property.name();
            QString stringName = name;

            if (ignoreList.indexOf(stringName) >= 0) {
                continue;
            }

            QVariant value = object->property(name);

            if (value == defaultValues[stringName]) {
                // ignore default value
                continue;
            }

            if (value.type() == QVariant::Font) {
                _dehydrateFont(dest, stringName, defaultValues[stringName].value<QFont>(), value.value<QFont>());
                continue;
            } else if (value.canConvert<QObject*>()) {
                // ignore object value
                continue;
            }

            dest[stringName] = value;
        }
        return dest;
    };

    auto isVisible = [=](QObject* object) {
        QQuickItem* item = qobject_cast<QQuickItem*>(object);

        if (!item) {
            return false;
        }

        if (item->opacity() == 0 ||
            !item->isVisible()) {
            return false;
        }

        return true;
    };

    auto allowTravel = [=](QObject* object) {
        if (!captureVisibleItemOnly) {
            return true;
        }

        return isVisible(object);
    };

    std::function<QVariantMap(QObject*)> travel;

    travel = [=, &travel](QObject* object) {
        if (!allowTravel(object)) {
            return QVariantMap();
        }

        QVariantMap dest;

        dest = _dehydrate(object);

        QObjectList children = object->children();
        QVariantList childrenDataList;
        for (int i = 0 ; i < children.size() ; i++) {
            QObject* child = children[i];
            QVariantMap childData = travel(child);
            if (!childData.isEmpty()) {
                childrenDataList << childData;
            }
        }

        QString className = removeDynamicClassSuffix(obtainClassName(object));

        if (className == "QQuickRepeater") {
            int count = object->property("count").toInt();
            for (int i = 0 ; i < count; i++) {
                QQuickItem* child;
                QMetaObject::invokeMethod(object,"itemAt",Qt::DirectConnection,
                                          Q_RETURN_ARG(QQuickItem*,child),
                                          Q_ARG(int,i));
                QVariantMap childData = travel(child);
                if (!childData.isEmpty()) {
                    childrenDataList << childData;
                }
            }
        } else if (inherited(object, "QQuickFlickable")) {

            QQuickItem* contentItem = object->property("contentItem").value<QQuickItem*>();

            if (contentItem) {
                QList<QQuickItem *>items = contentItem->childItems() ;

                for (int i = 0 ;  i < items.size() ; i++) {
                    QVariantMap childData = travel(items.at(i));
                    if (!childData.isEmpty()) {
                        childrenDataList << childData;
                    }
                }
            }
        }

        if (childrenDataList.size() > 0) {
            dest["$children"] = childrenDataList;
        }

        dest["$class"] = obtainKnownClassName(object);
        dest["$name"] = obtainItemName(object);

        QUrl baseUrl;
        QQmlContext *context = qmlContext(object);
        if (context) {
            baseUrl = context->baseUrl();
        }

        if (!expandAll && topLevelBaseUrlList.indexOf(baseUrl) < 0) {
            dest["$skip"] = true;
        }

        return dest;
    };

    if (captureVisibleItemOnly && !isVisible(source)) {
        qDebug() << "SnapshotTesting::capture(): The capture target is not visible";
    }

    return travel(source);
}

static QString prettyText(QVariantMap snapshot) {
    QStringList priorityFields;

    priorityFields << "objectName" << "x" << "y" << "width" << "height";

    auto _prettyField = [=](QString field, QVariant v, int indent) {
        QString res;
        QString format = "%1: %2";
        QString quotedFormat = "%1: \"%2\"";

        if (v.type() == QVariant::Bool) {
            res = QString(format).arg(field).arg(v.toBool() ? "true" : "false");
        } else if (v.type() == QVariant::Double) {
            double dv = v.toDouble();
            double intpart;
            double fractpart = modf(dv, &intpart);
            if (fractpart != 0) {
                res = QString(format).arg(field).arg(dv,0,'f',2,'0');
            } else {
                res = QString(format).arg(field).arg(v.toInt());
            }
        } else if (v.type() == QVariant::String) {
            res = QString(quotedFormat).arg(field).arg(v.toString());
        } else if (v.type() == QVariant::Int) {
            res = QString(format).arg(field).arg(v.toInt());
        } else if (v.type() == QVariant::Color) {
            res = QString(quotedFormat).arg(field).arg(v.value<QColor>().name());
        } else if (v.type() == QVariant::Url) {
            res = QString(quotedFormat).arg(field).arg(v.toUrl().toString());
        } else if (v.type() == QVariant::Size) {
            QSize size = v.toSize();
            res = QString("%1: Qt.size(%2,%3)").arg(field).arg(size.width()).arg(size.height());
        } else {
            qDebug() << "Non-supported type" << v.typeName() << " Field :" << field;
            return QString("");
        }

        res = QString("").fill(' ', indent) + res;

        return res;
    };

    std::function<QString(QVariantMap, int)> _prettyText;

    _prettyText = [=, &_prettyText](QVariantMap snapshot, int indent) {
        if (snapshot.isEmpty()) {
            return QString("");
        }

        QStringList lines;

        if (snapshot.contains("$skip")) {
            QVariantList children = snapshot["$children"].toList();
            for (int i = 0 ; i < children.size() ;i++) {
                QVariantMap data = children[i].toMap();
                QString line = _prettyText(data, indent);
                if (!line.isEmpty()) {
                    lines << "" << line;
                }
            }
            return lines.join("\n");
        }

        int currentIndent = indent + 4;

        lines << QString().fill(' ',indent) + snapshot["$name"].toString() + " {";

        QStringList keys = snapshot.keys();

        if (keys.indexOf("id") >= 0) {
            lines << _prettyField("id", snapshot["id"], currentIndent).replace("\"","");
            keys.removeOne("id");
        }

        for (int i = 0 ; i < priorityFields.size() ; i++) {
            QString key = priorityFields[i];
            if (keys.indexOf(key) >= 0) {
                QString line = _prettyField(key, snapshot[key], currentIndent);
                if (!line.isEmpty()) {
                    lines << line;
                }
                keys.removeOne(key);
            }
        }

        for (int i = 0 ; i < keys.size();i++) {
            QString key = keys[i];
            if (key.indexOf("$") == 0) {
                continue;
            }
            QString line = _prettyField(key, snapshot[key], currentIndent);
            if (!line.isEmpty())
                lines << line;
        }

        QVariantList children = snapshot["$children"].toList();
        for (int i = 0 ; i < children.size() ;i++) {
            QVariantMap data = children[i].toMap();
            QString line = _prettyText(data, currentIndent);
            if (!line.isEmpty()) {
                lines << "" << line;
            }
        }

        lines << QString().fill(' ',indent) +  QString("}");

        return lines.join("\n");
    };

    return _prettyText(snapshot, 0);
}


void SnapshotTesting::setSnapshotsFile(const QString &file)
{
    m_snapshotFile = QtShell::realpath_strip(file);
}

QString SnapshotTesting::snapshotsFile()
{
    return m_snapshotFile;
}

QVariantMap SnapshotTesting::loadStoredSnapshots()
{
    if (!m_snapshots.isEmpty()) {
        return m_snapshots;
    }

    QVariantMap result;
    if (!QFile::exists(m_snapshotFile)) {
        return result;
    }

    QString content = QtShell::cat(m_snapshotFile);

    if (content.isNull()) {
        return result;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(),&error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << QString("SnapshotTesting::loadSnapshots: Failed to parse file: %1: %2").arg(m_snapshotFile).arg(error.errorString());
    }

    QVariantMap data = doc.object().toVariantMap();

    m_snapshots = data["content"].toMap();

    return m_snapshots;
}


void SnapshotTesting::saveSnapshots()
{
    if (m_snapshots.isEmpty()) {
        loadStoredSnapshots();
    }

    if (!m_snapshotsDirty) {
        return;
    }

    m_snapshotsDirty = false;

    QVariantMap data;

    data["content"] = m_snapshots;

    QJsonObject object = QJsonObject::fromVariantMap(data);

    QJsonDocument doc;
    doc.setObject(object);
    QByteArray bytes = doc.toJson(QJsonDocument::Indented);

    QFile file;
    file.setFileName(m_snapshotFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << QString("SnapshotTesting::saveSnapshots: Failed to save snapshos file: %1").arg(file.errorString());
        return;
    }

    file.write(bytes);
    file.close();
}

void SnapshotTesting::setSnapshot(const QString &name, const QString &content)
{
    m_snapshots[name] = content;
    m_snapshotsDirty = true;
}

void SnapshotTesting::setInteractiveEnabled(bool value)
{
    m_interactiveEnabled = value;
}

bool SnapshotTesting::interactiveEnabled()
{
    return m_interactiveEnabled;
}

void SnapshotTesting::setIgnoreAll(bool value)
{
    m_ignoreAll = value;
}

bool SnapshotTesting::ignoreAll()
{
    return m_ignoreAll;
}


QString SnapshotTesting::capture(QObject *object, SnapshotTesting::Options options)
{
    QVariantMap data = dehydrate(object, options);
    return prettyText(data);
}

bool SnapshotTesting::matchStoredSnapshot(const QString &name, const QString &snapshot)
{
    QVariantMap snapshots = SnapshotTesting::loadStoredSnapshots();

    QString originalVersion = snapshots[name].toString();

    if (originalVersion == snapshot) {
        return true;
    }

    QString diff = SnapshotTesting::diff(originalVersion, snapshot);

    qDebug().noquote() << "matchStoredSnapshot: The snapshot is different:";
    qDebug().noquote() << diff;

    if (m_acceptAll) {
        SnapshotTesting::setSnapshot(name, snapshot);
        SnapshotTesting::saveSnapshots();
        return true;
    }

    if (SnapshotTesting::interactiveEnabled() && !SnapshotTesting::ignoreAll()) {
        QQmlApplicationEngine engine;
        engine.addImportPath("qrc:///");
        engine.load(QUrl("qrc:///qt-project.org/imports/SnapshotTesting/Matcher.qml"));

        QObject* dialog = engine.rootObjects()[0];
        Q_ASSERT(dialog);

        dialog->setProperty("diff", diff);
        dialog->setProperty("previousSnapshot", originalVersion);
        dialog->setProperty("snapshot", snapshot);
        dialog->setProperty("title", name);

        QMetaObject::invokeMethod(dialog, "open");
        QCoreApplication::exec();

        int button = dialog->property("clickedButton").value<int>();
        switch (button) {
        // Use hex code to avoid the dependence to QtWidget
        case 0x00020000: // No to all
            SnapshotTesting::setIgnoreAll(true);
            break;
        case 0x00008000: // Yes to all
            m_acceptAll = true;
        case 0x00004000: // Yes
        case 0x02000000:
            SnapshotTesting::setSnapshot(name, snapshot);
            SnapshotTesting::saveSnapshots();
            return true;
            break;
        }
    }

    return false;
}

static void init() {
    if (m_snapshotFile.isNull()) {
        m_snapshotFile = QtShell::realpath_strip(QtShell::pwd(), "snapshots.json");
    }

    QString text = QtShell::cat(":/qt-project.org/imports/SnapshotTesting/config/snapshot-config.json");

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(),&error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON::parse() error: "<< error.errorString();
    }

    QVariantMap map = doc.object().toVariantMap();
    knownComponentList = map.keys();
    for (int i = 0 ; i < knownComponentList.size() ; i++) {
        QString key = knownComponentList[i];
        QVariantMap record =  map[key].toMap();
        classNameToItemNameTable[key] = record["name"].toString();
        defaultValueMap[key] = record["defaultValues"].toMap();
        ignoreListMap[key] = record["ignoreList"].toStringList();
    }
}


QString SnapshotTesting::diff(QString original, QString current)
{
    auto toVector = [=](QString text) {
        vector<string> res;

        QStringList lines = text.split("\n");
        for (int i = 0 ; i < lines.size() ;i++) {
            res.push_back(lines[i].toStdString());
        }

        return res;
    };

    std::vector<string> text1, text2;

    text1 = toVector(original);
    text2 = toVector(current);
    dtl::Diff<std::string> diff(text1, text2);

    diff.onHuge();
    diff.compose();
    diff.composeUnifiedHunks();

    std::stringstream stream;

    diff.printUnifiedFormat(stream);

    return QString::fromStdString(stream.str());
}

Q_COREAPP_STARTUP_FUNCTION(init)
