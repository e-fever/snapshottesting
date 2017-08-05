#pragma once
#include <QQmlContext>

namespace SnapshotTesting {

    namespace Private {

        QString classNameToComponentName(const QString &className);

        /// Obtain the context of the input object which should it be belonged to, but not its parent's scope
        QQmlContext* obtainCurrentScopeContext(QObject* object);

        QQmlContext* obtainCreationContext(QObject* object);

        QString obtainComponentNameByBaseUrl(const QUrl& baseUrl);

        /// Obtain the name of the component by the creation context (the bottom most component
        QString obtainComponentNameByBaseContext(QObject* object);

        QString obtainComponentNameByInheritedContext(QObject * object);

        QString obtainComponentNameByClass(QObject* object);

        QString obtainRootComponentName(QObject* object);
    }
}
