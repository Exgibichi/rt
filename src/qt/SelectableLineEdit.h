﻿//SelectableLineEdit.h by Rngcoin developers
#pragma once
#include <QPointer>
#include <QLineEdit>

class SelectableLineEdit: public QLineEdit {
	public:
		SelectableLineEdit() {}
		virtual void mousePressEvent(QMouseEvent *e)override;
		void copyAndShowTooltip(QLineEdit*toDeselect = 0);
		
		QPointer<QWidget> _buddyToShowToolip;//use this window if not set
};
