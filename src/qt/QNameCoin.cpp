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
