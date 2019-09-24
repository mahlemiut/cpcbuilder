#ifndef CPRBUILDER_H
#define CPRBUILDER_H

#include <QString>
#include <QFile>
#include <QtWidgets>
#include <vector>
#include <stdint.h>

struct riff_header
{
	char header[4];
	uint32_t data_length;
	std::vector<uint8_t> data;
};

class cprbuilder
{
public:
	cprbuilder(QString fname);
	void init();
	bool generate_cpr();
	QString get_filename() { return m_filename; }
	void set_filename(QString file) { m_filename = file; }
	long add_block(QString file, char blocknum);
private:
	QString m_filename;
	riff_header main_header;
	std::vector<riff_header> m_blocks;
	bool m_blocks_used[32];  // map which blocks have been used up
};

#endif // CPRBUILDER_H
