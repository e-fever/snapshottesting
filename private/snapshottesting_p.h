#pragma once
#include <QQmlContext>
#include <QJSValue>

namespace SnapshotTesting {

    namespace Private {

        QString classNameToComponentName(const QString &className);

        /// Obtain the context of the input object which should it be belonged to, but not its parent's scope
        QQmlContext* obtainCurrentScopeContext(QObject* object);

        QQmlContext* obtainCreationContext(QObject* object);

        QString obtainComponentNameByBaseUrl(const QUrl& baseUrl);

        /// Obtain the name of the component by the creation context (the bottom most component)
        QString obtainComponentNameByBaseContext(QObject* object);

        QString obtainComponentNameByInheritedContext(QObject * object);

        QString obtainComponentNameByClass(QObject* object);

        QString obtainComponentNameByInheritedClass(QObject* object);

        QString obtainComponentNameByQuickClass(QObject* object);

        QString obtainComponentNameByCurrentScopeContext(QObject* object);

        QString obtainRootComponentName(QObject* object, bool expandAll = false);

        QString stringify(QJSEngine*engine, QJSValue value);

        QString stringify(QVariant v);

        QString leftpad(QString text, int pad);

        QString indentText(QString text, int pad);

        QObjectList obtainChildrenObjectList(QObject * object);
    }
}
