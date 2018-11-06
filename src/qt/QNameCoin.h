#pragma once
#include <QObject>

//Qt NameCoin interface
class QNameCoin: public QObject {//for tr()
    public:
        static bool isMyName(const QString & name);
        static bool nameActive(const QString & name);
        static QStringList myNames();
        static QStringList myNamesStartingWith(const QString & prefix);

        static QString labelForNameExistOrError(const QString & name);
        static QString labelForNameExistOrError(const QString & name, const QString & prefix);
        static QString trNameNotFound(const QString & name);
        static QString trNameIsFree(const QString & name, bool ok = true);
        static QString trNameAlreadyRegistered(const QString & name, bool ok);
        static const int charCheckOk = 0x2705;
        static const int charX = 0x274C;
        static QChar charBy(bool ok);
};
