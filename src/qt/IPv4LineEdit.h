﻿//IPv4LineEdit.h by rngcoin developers
#pragma once
#include <QLineEdit>
#include <QRegExpValidator>

class IPv4LineEdit: public QLineEdit {
	public:
		IPv4LineEdit();
		struct Validator: public QRegExpValidator {
			Validator(QObject*parent);
		};
};
