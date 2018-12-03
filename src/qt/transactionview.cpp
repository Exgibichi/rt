// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "transactionview.h"

#include "addresstablemodel.h"
#include "bitcoinunits.h"
#include "csvmodelwriter.h"
#include "editaddressdialog.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "transactiondescdialog.h"
#include "transactionfilterproxy.h"
#include "transactionrecord.h"
#include "transactiontablemodel.h"
#include "walletmodel.h"

#include "ui_interface.h"

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPoint>
#include <QScrollBar>
#include <QSignalMapper>
#include <QTableView>
#include <QUrl>
#include <QVBoxLayout>

struct TransactionView::TableView: public QTableView{
    TransactionView* _parent = 0;
    WalletModel *_walletModel = 0;
    TableView(TransactionView*parent): _parent(parent) {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setTabKeyNavigation(false);
        setContextMenuPolicy(Qt::CustomContextMenu);

        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setAlternatingRowColors(true);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setSortingEnabled(true);
        sortDefault();
        verticalHeader()->hide();

        setColumnWidth(TransactionTableModel::Status, STATUS_COLUMN_WIDTH);
        setColumnWidth(TransactionTableModel::Watchonly, WATCHONLY_COLUMN_WIDTH);
        setColumnWidth(TransactionTableModel::Date, DATE_COLUMN_WIDTH);
        setColumnWidth(TransactionTableModel::Type, TYPE_COLUMN_WIDTH);
        setColumnWidth(TransactionTableModel::Amount, AMOUNT_MINIMUM_COLUMN_WIDTH);
    }
    void sortDefault() {
        sortByColumn(TransactionTableModel::Date, Qt::DescendingOrder);
    }
    QString format(CAmount amount)const {
        return BitcoinUnits::format(_walletModel->getOptionsModel()->getDisplayUnit(), amount, false, BitcoinUnits::separatorAlways);
    }
    QString formatSelectedAmount()const {
        QMap<int, QModelIndex> rowToIndex;
        for(const QModelIndex& index: selectionModel()->selectedIndexes()) {
            rowToIndex[index.row()] = index;
        }

        CAmount amountPos = 0, amountNeg = 0;
        for(auto index: rowToIndex.values()) {
            CAmount amount = index.data(TransactionTableModel::AmountRole).toLongLong();
            if(amount>0)
                amountPos += amount;
            else
                amountNeg += amount;
        }
        QString s;
        if(!rowToIndex.isEmpty()) {
            s = tr("%1 RNG selected in %2 transactions")
                    .arg(format(amountPos + amountNeg))
                    .arg(rowToIndex.count());
            if(amountNeg!=0) {
                s += tr(" = %1 - %2").arg(format(amountPos)).arg(format(qAbs(amountNeg)));
            }
        }
        return s;
    }
    virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)override {
        QTableView::selectionChanged(selected, deselected);
        if(_walletModel)
            Q_EMIT _parent->amountSelected(formatSelectedAmount());
    }
};

TransactionView::TransactionView(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent)
{
    // Build filter row
    setContentsMargins(0,0,0,0);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0,0,0,0);

    if (platformStyle->getUseExtraSpacing()) {
        hlayout->setSpacing(5);
        hlayout->addSpacing(26);
    } else {
        hlayout->setSpacing(0);
        hlayout->addSpacing(23);
    }

    watchOnlyWidget = new QComboBox(this);
    watchOnlyWidget->setFixedWidth(24);
    watchOnlyWidget->addItem("", TransactionFilterProxy::WatchOnlyFilter_All);
    watchOnlyWidget->addItem(platformStyle->SingleColorIcon(":/icons/eye_plus"), "", TransactionFilterProxy::WatchOnlyFilter_Yes);
    watchOnlyWidget->addItem(platformStyle->SingleColorIcon(":/icons/eye_minus"), "", TransactionFilterProxy::WatchOnlyFilter_No);
    hlayout->addWidget(watchOnlyWidget);

    dateWidget = new QComboBox(this);
    if (platformStyle->getUseExtraSpacing()) {
        dateWidget->setFixedWidth(121);
    } else {
        dateWidget->setFixedWidth(120);
    }
    dateWidget->addItem(tr("All"), All);
    dateWidget->addItem(tr("Today"), Today);
    dateWidget->addItem(tr("This week"), ThisWeek);
    dateWidget->addItem(tr("This month"), ThisMonth);
    dateWidget->addItem(tr("Last month"), LastMonth);
    dateWidget->addItem(tr("This year"), ThisYear);
    dateWidget->addItem(tr("Range..."), Range);
    hlayout->addWidget(dateWidget);

    typeWidget = new QComboBox(this);
    if (platformStyle->getUseExtraSpacing()) {
        typeWidget->setFixedWidth(121);
    } else {
        typeWidget->setFixedWidth(120);
    }

    typeWidget->addItem(tr("All"), TransactionFilterProxy::ALL_TYPES);
    typeWidget->addItem(tr("Received with"), TransactionFilterProxy::TYPE(TransactionRecord::RecvWithAddress) |
                                        TransactionFilterProxy::TYPE(TransactionRecord::RecvFromOther));
    typeWidget->addItem(tr("Sent to"), TransactionFilterProxy::TYPE(TransactionRecord::SendToAddress) |
                                  TransactionFilterProxy::TYPE(TransactionRecord::SendToOther));
    typeWidget->addItem(tr("To yourself"), TransactionFilterProxy::TYPE(TransactionRecord::SendToSelf));
    typeWidget->addItem(tr("Mined"), TransactionFilterProxy::TYPE(TransactionRecord::Generated));
    typeWidget->addItem(tr("Name operation"), TransactionFilterProxy::TYPE(TransactionRecord::NameOp));
    typeWidget->addItem(tr("Mint by stake"), TransactionFilterProxy::TYPE(TransactionRecord::StakeMint));
    typeWidget->addItem(tr("Other"), TransactionFilterProxy::TYPE(TransactionRecord::Other));

    hlayout->addWidget(typeWidget);

    addressWidget = new QLineEdit(this);
#if QT_VERSION >= 0x040700
    addressWidget->setPlaceholderText(tr("Enter address or label to search"));
#endif
    hlayout->addWidget(addressWidget);

    amountWidget = new QLineEdit(this);
#if QT_VERSION >= 0x040700
    amountWidget->setPlaceholderText(tr("Min amount"));
#endif
    if (platformStyle->getUseExtraSpacing()) {
        amountWidget->setFixedWidth(97);
    } else {
        amountWidget->setFixedWidth(100);
    }
    amountWidget->setValidator(new QDoubleValidator(0, 1e20, 8, this));
    hlayout->addWidget(amountWidget);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setSpacing(0);

    /*QTableView *view = new QTableView(this);
    view->setStyleSheet(
                    "QTableView{"
                    "   gridline-color: rgb(246,246,247);"
                    "   border: 2px solid rgb(246,246,247,200);"
                    "   border-radius : 10px;"
                    "   background: rgb(255,255,255, 200);"
                    "   color: rgb(32,32,32);"
                    "}"
                    );

        view->horizontalHeader()->setStyleSheet(
                    "QHeaderView {        "
                    "    background: rgb(255,255,255);"
                    "    border-radius : 10px;     "
                    "}                             "

                    "QHeaderView::section {        "
                    "    background: rgb(255,255,255); "
                    "    border-right:  2px solid rgb(246,246,247); "
                    "    border-bottom: 2px solid rgb(246,246,247); "
                    "    border-left:   0px;       "
                    "    border-top:    0px;       "
                    "    padding: 4px;             "
                    "    font-size: 10pt;          "
                    "    font: \"Montserrat SemiBold\";"
                    "}                             "

                    "QHeaderView::section:first {        "
                    "    border-top-left-radius: 10px;   "
                    "    border-right: 2px solid rgb(246,246,247);    "
                    "} "

                    "QHeaderView::section:last {        "
                    "    border-top-right-radius: 10px; "
                    "}                                  "

                    );*/


    transactionView = new TableView(this);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(createDateRangeWidget());
    vlayout->addWidget(transactionView);
    vlayout->setSpacing(0);
    int width = transactionView->verticalScrollBar()->sizeHint().width();
    // Cover scroll bar width with spacing
    if (platformStyle->getUseExtraSpacing()) {
        hlayout->addSpacing(width+2);
    } else {
        hlayout->addSpacing(width);
    }

    transactionView->installEventFilter(this);

    // Actions
    abandonAction = new QAction(tr("Abandon transaction"), this);
    QAction *copyAddressAction = new QAction(tr("Copy address"), this);
    QAction *copyLabelAction = new QAction(tr("Copy label"), this);
    QAction *copyAmountAction = new QAction(tr("Copy amount"), this);
    QAction *copyTxIDAction = new QAction(tr("Copy transaction ID"), this);
    QAction *copyTxHexAction = new QAction(tr("Copy raw transaction"), this);
    QAction *copyTxPlainText = new QAction(tr("Copy full transaction details"), this);
    QAction *editLabelAction = new QAction(tr("Edit label"), this);
    QAction *showDetailsAction = new QAction(tr("Show transaction details"), this);

    contextMenu = new QMenu(this);
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    contextMenu->addAction(copyAmountAction);
    contextMenu->addAction(copyTxIDAction);
    contextMenu->addAction(copyTxHexAction);
    contextMenu->addAction(copyTxPlainText);
    contextMenu->addAction(showDetailsAction);
    contextMenu->addSeparator();
    contextMenu->addAction(abandonAction);
    contextMenu->addAction(editLabelAction);

    mapperThirdPartyTxUrls = new QSignalMapper(this);

    // Connect actions
    connect(mapperThirdPartyTxUrls, SIGNAL(mapped(QString)), this, SLOT(openThirdPartyTxUrl(QString)));

    connect(dateWidget, SIGNAL(activated(int)), this, SLOT(chooseDate(int)));
    connect(typeWidget, SIGNAL(activated(int)), this, SLOT(chooseType(int)));
    connect(watchOnlyWidget, SIGNAL(activated(int)), this, SLOT(chooseWatchonly(int)));
    connect(addressWidget, SIGNAL(textChanged(QString)), this, SLOT(changedPrefix(QString)));
    connect(amountWidget, SIGNAL(textChanged(QString)), this, SLOT(changedAmount(QString)));

    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(doubleClicked(QModelIndex)));
    connect(transactionView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));

    connect(abandonAction, SIGNAL(triggered()), this, SLOT(abandonTx()));
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(copyLabel()));
    connect(copyAmountAction, SIGNAL(triggered()), this, SLOT(copyAmount()));
    connect(copyTxIDAction, SIGNAL(triggered()), this, SLOT(copyTxID()));
    connect(copyTxHexAction, SIGNAL(triggered()), this, SLOT(copyTxHex()));
    connect(copyTxPlainText, SIGNAL(triggered()), this, SLOT(copyTxPlainText()));
    connect(editLabelAction, SIGNAL(triggered()), this, SLOT(editLabel()));
    connect(showDetailsAction, SIGNAL(triggered()), this, SLOT(showDetails()));

    //
    //  setup stylesheets
    //
    this->setObjectName("txViewWidget");
    this->setStyleSheet( "#txViewWidget {"
                         "background-color: rgb(246, 246, 247,20);"
                         "}"
                         );

    // QComboBox
    dateWidget->setStyleSheet("QComboBox {                                      "
                              " border: 0px;                                    "
                              " border-radius: 10px;                            "
                              " min-width: 6em;                                 "
                              " font-size: 14px;                                "
                              " font: \"Montserrat\";                           "
                              " color: rgb(136, 136, 136);                      "
                              " background-color: rgb(255, 255, 255);           "
                              " }                                               "
                              "                                                 "
                              " QComboBox::drop-down {                          "
                              "     subcontrol-origin: padding;                 "
                              "     subcontrol-position: top right;             "
                              "     padding-right: 10px;                        "
                              "     image: url(:/icons/res/icons/triangle-down.png);"
                              "     width: 15px;                                "
                              "     border-top-right-radius: 10px;              "
                              "     border-bottom-right-radius: 10px;           "
                              " }                                               "
                              "                                                 "
                              " QComboBox QAbstractItemView {                   "
                              "     background-color: white;                    "
                              "     border: 0px;                                "
                              "     selection-background-color:  #3796a8;       "
                              " }                                               "
                          );

    typeWidget->setStyleSheet( dateWidget->styleSheet() );
    watchOnlyWidget->setStyleSheet( dateWidget->styleSheet() );

    // QLineEdit
    amountWidget->setStyleSheet(
                "border-radius: 10px;                  "
                "background-color: rgb(255, 255, 255); "
                "font-size: 14px;                      "
                "font: \"Montserrat\";                 "
                "color: rgb(136, 136, 136);            "
                "padding-left: 8px;                    "
                "padding-right: px;                    "
                );
}

void TransactionView::setModel(WalletModel *_model)
{
    this->model = _model;
    if(!_model)
        return;
    transactionView->_walletModel = _model;
    transactionProxyModel = new TransactionFilterProxy(this);
    transactionProxyModel->setSourceModel(_model->getTransactionTableModel());
    transactionProxyModel->setDynamicSortFilter(true);
    transactionProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    transactionProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    transactionProxyModel->setSortRole(Qt::EditRole);

    transactionView->setModel(transactionProxyModel);
    transactionView->sortDefault();

    columnResizingFixer = new GUIUtil::TableViewLastColumnResizingFixer(transactionView, AMOUNT_MINIMUM_COLUMN_WIDTH, MINIMUM_COLUMN_WIDTH, this);

    if (_model->getOptionsModel())
    {
        // Add third party transaction URLs to context menu
        QStringList listUrls = _model->getOptionsModel()->getThirdPartyTxUrls().split('|', QString::SkipEmptyParts);
        for (int i = 0; i < listUrls.size(); ++i)
        {
            QString host = QUrl(listUrls[i].trimmed(), QUrl::StrictMode).host();
            if (!host.isEmpty())
            {
                QAction *thirdPartyTxUrlAction = new QAction(host, this); // use host as menu item label
                if (i == 0)
                    contextMenu->addSeparator();
                contextMenu->addAction(thirdPartyTxUrlAction);
                connect(thirdPartyTxUrlAction, SIGNAL(triggered()), mapperThirdPartyTxUrls, SLOT(map()));
                mapperThirdPartyTxUrls->setMapping(thirdPartyTxUrlAction, listUrls[i].trimmed());
            }
        }
    }

    // show/hide column Watch-only
    updateWatchOnlyColumn(_model->haveWatchOnly());

    // Watch-only signal
    connect(_model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyColumn(bool)));
}

void TransactionView::chooseDate(int idx)
{
    if(!transactionProxyModel)
        return;
    QDate current = QDate::currentDate();
    dateRangeWidget->setVisible(false);
    switch(dateWidget->itemData(idx).toInt())
    {
    case All:
        transactionProxyModel->setDateRange(
                TransactionFilterProxy::MIN_DATE,
                TransactionFilterProxy::MAX_DATE);
        break;
    case Today:
        transactionProxyModel->setDateRange(
                QDateTime(current),
                TransactionFilterProxy::MAX_DATE);
        break;
    case ThisWeek: {
        // Find last Monday
        QDate startOfWeek = current.addDays(-(current.dayOfWeek()-1));
        transactionProxyModel->setDateRange(
                QDateTime(startOfWeek),
                TransactionFilterProxy::MAX_DATE);

        } break;
    case ThisMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month(), 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case LastMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month(), 1).addMonths(-1)),
                QDateTime(QDate(current.year(), current.month(), 1)));
        break;
    case ThisYear:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), 1, 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case Range:
        dateRangeWidget->setVisible(true);
        dateRangeChanged();
        break;
    }
}

void TransactionView::chooseType(int idx)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setTypeFilter(
        typeWidget->itemData(idx).toInt());
}

void TransactionView::chooseWatchonly(int idx)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setWatchOnlyFilter(
        (TransactionFilterProxy::WatchOnlyFilter)watchOnlyWidget->itemData(idx).toInt());
}

void TransactionView::changedPrefix(const QString &prefix)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setAddressPrefix(prefix);
}

void TransactionView::changedAmount(const QString &amount)
{
    if(!transactionProxyModel)
        return;
    CAmount amount_parsed = 0;
    if(BitcoinUnits::parse(model->getOptionsModel()->getDisplayUnit(), amount, &amount_parsed))
    {
        transactionProxyModel->setMinAmount(amount_parsed);
    }
    else
    {
        transactionProxyModel->setMinAmount(0);
    }
}

void TransactionView::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(this,
        tr("Export Transaction History"), QString(),
        tr("Comma separated file (*.csv)"), NULL);

    if (filename.isNull())
        return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(transactionProxyModel);
    writer.addColumn(tr("Confirmed"), 0, TransactionTableModel::ConfirmedRole);
    if (model && model->haveWatchOnly())
        writer.addColumn(tr("Watch-only"), TransactionTableModel::Watchonly);
    writer.addColumn(tr("Date"), 0, TransactionTableModel::DateRole);
    writer.addColumn(tr("Type"), TransactionTableModel::Type, Qt::EditRole);
    writer.addColumn(tr("Label"), 0, TransactionTableModel::LabelRole);
    writer.addColumn(tr("Address"), 0, TransactionTableModel::AddressRole);
    writer.addColumn(BitcoinUnits::getAmountColumnTitle(model->getOptionsModel()->getDisplayUnit()), 0, TransactionTableModel::FormattedAmountRole);
    writer.addColumn(tr("ID"), 0, TransactionTableModel::TxIDRole);

    if(!writer.write()) {
        Q_EMIT message(tr("Exporting Failed"), tr("There was an error trying to save the transaction history to %1.").arg(filename),
            CClientUIInterface::MSG_ERROR);
    }
    else {
        Q_EMIT message(tr("Exporting Successful"), tr("The transaction history was successfully saved to %1.").arg(filename),
            CClientUIInterface::MSG_INFORMATION);
    }
}

void TransactionView::contextualMenu(const QPoint &point)
{
    QModelIndex index = transactionView->indexAt(point);
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);
    if (selection.empty())
        return;

    // check if transaction can be abandoned, disable context menu action in case it doesn't
    uint256 hash;
    hash.SetHex(selection.at(0).data(TransactionTableModel::TxHashRole).toString().toStdString());
    abandonAction->setEnabled(model->transactionCanBeAbandoned(hash));

    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void TransactionView::abandonTx()
{
    if(!transactionView || !transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);

    // get the hash from the TxHashRole (QVariant / QString)
    uint256 hash;
    QString hashQStr = selection.at(0).data(TransactionTableModel::TxHashRole).toString();
    hash.SetHex(hashQStr.toStdString());

    // Abandon the wallet transaction over the walletModel
    model->abandonTransaction(hash);

    // Update the table
    model->getTransactionTableModel()->updateTransaction(hashQStr, CT_UPDATED, false);
}

void TransactionView::copyAddress()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::AddressRole);
}

void TransactionView::copyLabel()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::LabelRole);
}

void TransactionView::copyAmount()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::FormattedAmountRole);
}

void TransactionView::copyTxID()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxIDRole);
}

void TransactionView::copyTxHex()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxHexRole);
}

void TransactionView::copyTxPlainText()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxPlainTextRole);
}

void TransactionView::editLabel()
{
    if(!transactionView->selectionModel() ||!model)
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        AddressTableModel *addressBook = model->getAddressTableModel();
        if(!addressBook)
            return;
        QString address = selection.at(0).data(TransactionTableModel::AddressRole).toString();
        if(address.isEmpty())
        {
            // If this transaction has no associated address, exit
            return;
        }
        // Is address in address book? Address book can miss address when a transaction is
        // sent from outside the UI.
        int idx = addressBook->lookupAddress(address);
        if(idx != -1)
        {
            // Edit sending / receiving address
            QModelIndex modelIdx = addressBook->index(idx, 0, QModelIndex());
            // Determine type of address, launch appropriate editor dialog type
            QString type = modelIdx.data(AddressTableModel::TypeRole).toString();

            EditAddressDialog dlg(
                type == AddressTableModel::Receive
                ? EditAddressDialog::EditReceivingAddress
                : EditAddressDialog::EditSendingAddress, this);
            dlg.setModel(addressBook);
            dlg.loadRow(idx);
            dlg.exec();
        }
        else
        {
            // Add sending address
            EditAddressDialog dlg(EditAddressDialog::NewSendingAddress,
                this);
            dlg.setModel(addressBook);
            dlg.setAddress(address);
            dlg.exec();
        }
    }
}

void TransactionView::showDetails()
{
    if(!transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        TransactionDescDialog *dlg = new TransactionDescDialog(selection.at(0));
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    }
}

void TransactionView::openThirdPartyTxUrl(QString url)
{
    if(!transactionView || !transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);
    if(!selection.isEmpty())
         QDesktopServices::openUrl(QUrl::fromUserInput(url.replace("%s", selection.at(0).data(TransactionTableModel::TxHashRole).toString())));
}

QWidget *TransactionView::createDateRangeWidget()
{
    dateRangeWidget = new QFrame();
    dateRangeWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    dateRangeWidget->setContentsMargins(1,1,1,1);
    QHBoxLayout *layout = new QHBoxLayout(dateRangeWidget);
    layout->setContentsMargins(0,0,0,0);
    layout->addSpacing(23);
    layout->addWidget(new QLabel(tr("Range:")));

    dateFrom = new QDateTimeEdit(this);
    dateFrom->setDisplayFormat("dd/MM/yy");
    dateFrom->setCalendarPopup(true);
    dateFrom->setMinimumWidth(100);
    dateFrom->setDate(QDate::currentDate().addDays(-7));
    layout->addWidget(dateFrom);
    layout->addWidget(new QLabel(tr("to")));

    dateTo = new QDateTimeEdit(this);
    dateTo->setDisplayFormat("dd/MM/yy");
    dateTo->setCalendarPopup(true);
    dateTo->setMinimumWidth(100);
    dateTo->setDate(QDate::currentDate());
    layout->addWidget(dateTo);
    layout->addStretch();

    // Hide by default
    dateRangeWidget->setVisible(false);

    // Notify on change
    connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));
    connect(dateTo, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));

    return dateRangeWidget;
}

void TransactionView::dateRangeChanged()
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setDateRange(
            QDateTime(dateFrom->date()),
            QDateTime(dateTo->date()).addDays(1));
}

void TransactionView::focusTransaction(const QModelIndex &idx)
{
    if(!transactionProxyModel)
        return;
    QModelIndex targetIdx = transactionProxyModel->mapFromSource(idx);
    transactionView->scrollTo(targetIdx);
    transactionView->setCurrentIndex(targetIdx);
    transactionView->setFocus();
}

// We override the virtual resizeEvent of the QWidget to adjust tables column
// sizes as the tables width is proportional to the dialogs width.
void TransactionView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    //columnResizingFixer->stretchColumnWidth(TransactionTableModel::ToAddress);
    columnResizingFixer->stretchColumnWidth(TransactionTableModel::TxComment);
}

// Need to override default Ctrl+C action for amount as default behaviour is just to copy DisplayRole text
bool TransactionView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_C && ke->modifiers().testFlag(Qt::ControlModifier))
        {
             GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxPlainTextRole);
             return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// show/hide column Watch-only
void TransactionView::updateWatchOnlyColumn(bool fHaveWatchOnly)
{
    watchOnlyWidget->setVisible(fHaveWatchOnly);
    transactionView->setColumnHidden(TransactionTableModel::Watchonly, !fHaveWatchOnly);
}
