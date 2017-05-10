#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore>

class appsettings
{
public:
	appsettings();
	void set_includedir(QString dir);
	void set_pasmo_path(QString dir);
	void set_pasmo_use_syspath(bool chk);
	QString includedir();
	QString pasmo_path();
	bool pasmo_use_syspath();
	QStringList::iterator include_begin() { return m_includedirs.begin(); }
	QStringList::iterator include_end() { return m_includedirs.end(); }
private:
	QStringList m_includedirs;
	QString m_pasmo_path;
	bool m_pasmo_use_syspath;  // true is using Pasmo in system path
};
#endif // APPSETTINGS_H
