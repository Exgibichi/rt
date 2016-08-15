// Copyright (c) 2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_QT_NETWORKSTYLE_H
#define BITCOIN_QT_NETWORKSTYLE_H

#include <QIcon>
#include <QPixmap>
#include <QString>

/* Coin network-specific GUI style information */
class NetworkStyle
{
public:
    /** Get style associated with provided BIP70 network id, or 0 if not known */
    static const NetworkStyle *instantiate(const QString &networkId);

    const QString &getAppName() const { return appName; }
    const QIcon &getAppIcon() const { return appIcon; }
    const QString &getTitleAddText() const { return titleAddText; }
    const QPixmap &getSplashImage() const { return splashImage; }

private:
    NetworkStyle(const QString &appName, const QString &appIcon, const char *titleAddText, const QString &splashImage);

    QString appName;
    QIcon appIcon;
    QString titleAddText;
    QPixmap splashImage;
};

#endif // BITCOIN_QT_NETWORKSTYLE_H
