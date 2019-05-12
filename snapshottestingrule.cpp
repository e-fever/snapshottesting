#include <private/snapshottesting_p.h>
#include "snapshottesting.h"

bool SnapshotTesting::Private::Rule::isIgnoredProperty(QObject *object, const QString &property, const QStringList &rules)
{
    QMap<QString,bool> properties = findIgnorePropertyList(object, rules);
    return properties.contains(property);
}
