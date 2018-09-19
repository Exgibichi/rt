//NameEqValueTextEdit.h by Emercoin developers
#pragma once
#include <QPlainTextEdit>
class QSize;

class NameEqValueTextEdit: public QPlainTextEdit {
	public:
		NameEqValueTextEdit();
		virtual QSize sizeHint()const override;
	protected:
		struct Highlighter;
};
