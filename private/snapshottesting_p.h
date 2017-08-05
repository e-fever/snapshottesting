#pragma once
#include <QQmlContext>

namespace SnapshotTesting {

    namespace Private {

        QQmlContext* obtainCreationContext(QObject* object);

        QString obtainComponentNameByBaseUrl(const QUrl& baseUrl);

        /// Obtain the name of the component by the creation context (the bottom most component
        QString obtainComponentNameByBaseContext(QObject* object);

        QString obtainComponentNameByTopContext(QObject *object);

        QString obtainComponentNameByInheritedContext(QObject * object);

        QString obtainComponentNameByClass(QObject* object);

        QString obtainRootComponentName(QObject* object);
    }
}
