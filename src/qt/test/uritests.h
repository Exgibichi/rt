// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_QT_TEST_URITESTS_H
#define BITCOIN_QT_TEST_URITESTS_H

#include <QObject>
#include <QTest>

class URITests : public QObject
{
    Q_OBJECT

private slots:
    void uriTests();
};

#endif // BITCOIN_QT_TEST_URITESTS_H
