//RegisterDiplomaWidget.cpp by Emercoin developers
#include "RegisterDiplomaWidget.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>

RegisterDiplomaWidget::RegisterDiplomaWidget() {
	setWindowTitle(tr("Register diploma"));
	setWindowIcon(QIcon(":/icons/Trusted-diploma-16-monochrome.png"));

	auto lay = new QVBoxLayout(this);
	auto label = new QLabel(tr("Coming soon. Meanwhile you can make these steps manually as described in the manual above"));
	lay->addWidget(label);
	lay->addStretch();
}
