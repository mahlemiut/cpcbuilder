// Class for handling application settings

#include "appsettings.h"


appsettings::appsettings() :
	m_pasmo_use_syspath(true)
{
	m_includedirs.clear();
	QDir default_incl("."+QString(QDir::separator())+"include");
	m_includedirs.append(default_incl.absolutePath());
	m_pasmo_path.clear();
}


void appsettings::set_includedir(QString dir)
{
	m_includedirs.clear();
	m_includedirs = dir.split(";",QString::SkipEmptyParts);
}

QString appsettings::includedir()
{
	QString str;
	QStringList::const_iterator it;
	for (it = m_includedirs.constBegin(); it != m_includedirs.constEnd(); ++it)
	{
		str.append(QString((*it).toLocal8Bit().constData()));
		if(it != m_includedirs.constEnd())
			str.append(";");
	}
	return str;
}

void appsettings::set_pasmo_path(QString dir)
{
	m_pasmo_path = dir;
}

void appsettings::set_pasmo_use_syspath(bool chk)
{
	m_pasmo_use_syspath = chk;
}

QString appsettings::pasmo_path()
{
	return m_pasmo_path;
}

bool appsettings::pasmo_use_syspath()
{
	return m_pasmo_use_syspath;
}
