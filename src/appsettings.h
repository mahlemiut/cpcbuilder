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
	QString emu_path();
	void set_emu_path(QString dir);
	int emu_model();
	void set_emu_model(int model);
	int emu_exp();
	void set_emu_exp(int exp);
	QString emu_ospath();
	void set_emu_ospath(QString path);
	QStringList::iterator include_begin() { return m_includedirs.begin(); }
	QStringList::iterator include_end() { return m_includedirs.end(); }
	enum
	{
		EMUMODEL_464,
		EMUMODEL_664,
		EMUMODEL_6128,
		EMUMODEL_464PLUS,
		EMUMODEL_6128PLUS,
		EMUMODEL_GX4000
	};
	enum
	{
		EMUEXP_NONE,
		EMUEXP_DKSPEECH,
		EMUEXP_SSA1,
		EMUEXP_ROMBOARD
	};
private:
	QStringList m_includedirs;
	QString m_pasmo_path;
	bool m_pasmo_use_syspath;  // true if using Pasmo in system path
	QString m_emu_path;
	QString m_emu_ospath;  // path to CPC Plus OS cartridge for disk projects tested on a Plus driver
	int m_emu_model;  // CPC model to run in emulator
	int m_emu_exp;  // Expansion device to connect in emulator
};
#endif // APPSETTINGS_H
