#pragma once
#include <QObject>
class UniValue;

//Qt NameCoin interface
class QNameCoin: public QObject {//for tr()
    public:
        static bool isMyName(const QString & name);
        static bool nameActive(const QString & name);
        static QStringList myNames();
        static QStringList myNamesStartingWith(const QString & prefix);

        static UniValue signMessage(const QString& address, const QString& message);
        static UniValue nameShow(const QString& name);

        static QString labelForNameExistOrError(const QString & name);
        static QString labelForNameExistOrError(const QString & name, const QString & prefix);
        static QString trNameNotFound(const QString & name);
        static QString trNameIsFree(const QString & name, bool ok = true);
        static QString trNameAlreadyRegistered(const QString & name, bool ok);
        static const int charCheckOk = 0x2705;
        static const int charX = 0x274C;
        static QChar charBy(bool ok);

        static QString numberLikeBase64(quint64 n);
        static QString currentSecondsPseudoBase64();
};
