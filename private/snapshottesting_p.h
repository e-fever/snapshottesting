#pragma once
#include <QQmlContext>

namespace SnapshotTesting {

    namespace Private {

        QQmlContext* obtainCreationContext(QObject* object);

        /// Obtain the name of the component offer the context for display by the base url from QQmlContext
        QString obtainComponentNameByBaseUrl(const QUrl& baseUrl);

        QString obtainComponentNameByContext(QObject* object);

        QString obtainComponentNameByClass(QObject* object);

        QString obtainRootComponentName(QObject* object);
    }
}
