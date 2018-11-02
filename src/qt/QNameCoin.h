#pragma once
#include <QObject>

//Qt NameCoin interface
class QNameCoin {
    public:
        static bool isMyName(const QString & name);
        static bool nameActive(const QString & name);
        static QStringList myNames();
        static QStringList myNamesStartingWith(const QString & prefix);
};
