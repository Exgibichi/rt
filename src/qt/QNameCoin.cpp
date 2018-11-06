#include "QNameCoin.h"

using CNameVal = std::vector<unsigned char>;
std::vector<CNameVal> NameCoin_myNames();
bool NameActive(const CNameVal& name, int currentBlockHeight = -1);
CNameVal toCNameVal(const std::string & s);
bool NameCoin_isMyName(const CNameVal & name);
CNameVal toCNameVal(const QString& s) {
	CNameVal ret;
	auto arr = s.toUtf8();
	for(char c: arr)
		ret.push_back(c);
	return ret;
}

bool QNameCoin::isMyName(const QString & name) {
	return NameCoin_isMyName(toCNameVal(name));
}
bool QNameCoin::nameActive(const QString & name) {
	return NameActive(toCNameVal(name));
}
QString toQString(const CNameVal & v) {
	const unsigned char* start = &v[0];
	return QString::fromUtf8(reinterpret_cast<const char*>(start), v.size());
}

QStringList QNameCoin::myNames() {
	QStringList ret;
	std::vector<CNameVal> names = NameCoin_myNames();
	for(const auto & name: names) {
		ret << toQString(name);
	}
	return ret;
}
QStringList QNameCoin::myNamesStartingWith(const QString & prefix) {
	QStringList ret;
	for(const auto & s: myNames()) {
		if(s.startsWith(prefix))
			ret << s;
	}
	return ret;
}
QChar QNameCoin::charBy(bool ok) {
	return ok ? charCheckOk : charX;
}
QString QNameCoin::trNameNotFound(const QString & name) {
	return charBy(false) + tr(" This name is not found (%1)").arg(name);
}
QString QNameCoin::trNameIsFree(const QString & name, bool ok) {
	return charBy(ok) + tr(" This name is free (%1)").arg(name);
}
QString QNameCoin::trNameAlreadyRegistered(const QString & name, bool ok) {
	return  charBy(ok) + tr(" This name is already registered in blockchain (%1)").arg(name);
}
QString QNameCoin::labelForNameExistOrError(const QString & name) {
	if(name.isEmpty())
		return {};

	if(nameActive(name))
		return charBy(true) + tr(" This name is registered (%1)").arg(name);

	return trNameNotFound(name);
}
QString QNameCoin::labelForNameExistOrError(const QString & name, const QString & prefix) {
	if(prefix.isEmpty())
		return labelForNameExistOrError(name);

	if(name.isEmpty())
		return {};

	if(!name.startsWith(prefix))
		return charBy(false) + tr(" Name must start with '%1' prefix").arg(prefix);

	if(name==prefix)
		return charBy(false) + tr(" Enter name");

	if(nameActive(name))
		return charBy(true) + tr(" This name is registered (%1)").arg(name);

	return trNameNotFound(name);
}
