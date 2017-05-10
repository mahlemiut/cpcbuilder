/*
 * project.h
 *
 *  Created on: 17/08/2011
 *      Author: bsr
 */

#ifndef PROJECT_H_
#define PROJECT_H_

#include <QtWidgets>
#include "appsettings.h"

enum
{
	PROJECT_FILE_BINARY = 0,  // will just be copied to destination
	PROJECT_FILE_SOURCE_ASM, // will be compiled by Pasmo
	PROJECT_FILE_GRAPHICS,  // graphics data, will be treated the same as binary data
	PROJECT_FILE_ASCII,  // Will be copied without an AMSDOS header (unless protected)
	PROJECT_FILE_TILESET  // also graphics data, but stored linearly
};

class project_file
{
public:
	project_file();
	project_file(QString filename);
	project_file(QString filename, int filetype, unsigned int load, unsigned int exec);
	~project_file();
	void set_filename(QString filename);
	QString get_filename() { return m_filename; }
	int get_filetype() { return m_filetype; }
	void set_filetype(int type) { m_filetype = type; }
	unsigned int get_load_address() { return m_load_address; }
	unsigned int get_exec_address() { return m_exec_address; }
	void set_load_address(unsigned int addr) { m_load_address = addr; }
	void set_exec_address(unsigned int addr) { m_exec_address = addr; }
	void set_size(int width, int height) { m_width = width; m_height = height; }
	int get_width() { return m_width; }
	int get_height() { return m_height; }
private:
	QString m_filename;
	int m_filetype;
	unsigned int m_load_address;
	unsigned int m_exec_address;
	int m_width;  // gfx only
	int m_height; // gfx only
};


class project
{
public:
	project(QString name, QString filename, appsettings& stg);
	~project();
	QString get_filename() { return m_filename; }
	QString get_name() { return m_name; }
	void set_name(QString name) { m_name = name; }
	QList<project_file*> get_filelist() { return m_filelist; }
	void xml_filelist(QXmlStreamWriter* stream, QDir curr);
	bool add_file(QString filename);
	bool add_file(project_file* file);
	bool add_gfx_file(QString filename, int width, int height);
	bool add_tileset_file(QString filename, int width, int height);
	bool remove_file(QString filename);
	bool check_for_file(QString filename);
	project_file* find_file(QString file);
	unsigned int get_load_address(QString file);
	unsigned int get_exec_address(QString file);
	void set_load_address(QString file, unsigned int addr);
	void set_exec_address(QString file, unsigned int addr);
	int get_filetype(QString file);
	void set_filetype(QString file, int type);
	QString get_output_filename() { return m_outfilename; }
	void set_output_filename(QString fname) { m_outfilename = fname; }
	int build(QPlainTextEdit* output = 0);
private:
	QString m_filename;  // project file name
	QString m_name;  // name of project
	QString m_outfilename;  // filename of DSK to output to.
	QList<project_file*> m_filelist;
	bool m_built;  // build status (true if a build has been successful)
	appsettings m_settings;
};

#endif /* PROJECT_H_ */
