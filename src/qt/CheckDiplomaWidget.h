//CheckDiplomaWidget.h by Emercoin developers
#pragma once
#include <QLineEdit>
#include <QSpinBox>

class CheckDiplomaWidget: public QWidget {
	public:
		CheckDiplomaWidget();
		static const QString s_checkUniversity;
		static const QString s_checkStudent;
	protected:
		QLineEdit* _name = 0;
		QLineEdit* _university = 0;
		QSpinBox* _yearAdmission = 0;
		QSpinBox* _yearGraduation = 0;
		void onSearch();
};
