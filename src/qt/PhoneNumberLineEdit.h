//PhoneNumberLineEdit.h by Emercoin developers
#include <QLineEdit>

struct PhoneNumberLineEdit: public QLineEdit {
	PhoneNumberLineEdit();
	QString toPhoneNumber()const;
};
