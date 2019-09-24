/*
 * project.cpp
 *
 *  Created on: 17/08/2011
 *      Author: bsr
 *
 *  Class to handle prjoect contents and settings
 */

#include "project.h"
#include "dskbuild.h"
#include "cprbuild.h"

project_file::project_file() :
	m_filename(""),
	m_load_address(0x8000),
	m_exec_address(0x8000)
{
}

project_file::project_file(QString filename) :
	m_filename(filename),
	m_load_address(0x8000),
	m_exec_address(0x8000),
	m_block(-1)
{
	// base class
}

project_file::project_file(QString filename, int filetype, unsigned int load, unsigned int exec) :
	m_filename(filename),
	m_filetype(filetype),
	m_load_address(load),
	m_exec_address(exec),
	m_block(-1)
{
}

project_file::~project_file()
{

}

void project_file::set_filename(QString filename)
{
	m_filename = filename;
}

project::project(QString name, QString filename, appsettings &stg) :
	m_filename(filename),
	m_name(name),
	m_outfilename(m_name+".dsk"),
	m_buildtype(BUILD_DISK),
	m_built(false),
	m_settings(stg)
{
}

project::~project()
{
	// remove and free all the project_file objects in this project
	QList<project_file*>::iterator it;

	// find file to remove
	for(it=m_filelist.begin();it!=m_filelist.end();it++)
	{
		m_filelist.erase(it);
		//delete(*it);  // TODO: test this
	}
}

bool project::add_file(QString filename)
{
	project_file* pfile = new project_file(filename);
	m_filelist.append(pfile);
	return true;
}

bool project::add_file(project_file* file)
{
	m_filelist.append(file);
	return true;
}

bool project::add_gfx_file(QString filename, int width, int height)
{
	project_file* pfile = new project_file(filename);
	pfile->set_filetype(PROJECT_FILE_GRAPHICS);
	pfile->set_size(width,height);
	m_filelist.append(pfile);
	return true;
}

bool project::add_tileset_file(QString filename, int width, int height)
{
	project_file* pfile = new project_file(filename);
	pfile->set_filetype(PROJECT_FILE_TILESET);
	pfile->set_size(width,height);
	m_filelist.append(pfile);
	return true;
}

bool project::remove_file(QString filename)
{
	QList<project_file*>::iterator it;

	// find file to remove
	for(it=m_filelist.begin();it!=m_filelist.end();it++)
	{
		if((*it)->get_filename() == filename)
		{
			m_filelist.erase(it);
			return true;
		}
	}
	return false;
}

bool project::check_for_file(QString filename)
{
	QList<project_file*>::iterator it;

	// find file
	for(it=m_filelist.begin();it!=m_filelist.end();it++)
	{
		if((*it)->get_filename() == filename)
		{
			return true;
		}
	}
	return false;
}

void project::xml_filelist(QXmlStreamWriter* stream, QDir curr)
{
	QList<project_file*>::iterator it;

	for(it=m_filelist.begin();it!=m_filelist.end();it++)
	{
		stream->writeStartElement("file");
		stream->writeAttribute("name",curr.relativeFilePath((*it)->get_filename()));
		if((*it)->get_filetype() == PROJECT_FILE_SOURCE_ASM)
		{
			stream->writeAttribute("type","asm");
			stream->writeAttribute("load",QString::number((*it)->get_load_address(),16));
			stream->writeAttribute("exec",QString::number((*it)->get_exec_address(),16));
			if(m_buildtype == BUILD_CART)
				stream->writeAttribute("block",QString::number((*it)->get_block(),10));
		}
		if((*it)->get_filetype() == PROJECT_FILE_BINARY)
		{
			stream->writeAttribute("type","binary");
			stream->writeAttribute("load",QString::number((*it)->get_load_address(),16));
			stream->writeAttribute("exec",QString::number((*it)->get_exec_address(),16));
			if(m_buildtype == BUILD_CART)
				stream->writeAttribute("block",QString::number((*it)->get_block(),10));
		}
		if((*it)->get_filetype() == PROJECT_FILE_GRAPHICS)
		{
			stream->writeAttribute("type","gfx");
			stream->writeAttribute("load",QString::number((*it)->get_load_address(),16));
			stream->writeAttribute("exec",QString::number((*it)->get_exec_address(),16));
			stream->writeAttribute("width",QString::number((*it)->get_width(),10));
			stream->writeAttribute("height",QString::number((*it)->get_height(),10));
			if(m_buildtype == BUILD_CART)
				stream->writeAttribute("block",QString::number((*it)->get_block(),10));
		}
		if((*it)->get_filetype() == PROJECT_FILE_TILESET)
		{
			stream->writeAttribute("type","tileset");
			stream->writeAttribute("load",QString::number((*it)->get_load_address(),16));
			stream->writeAttribute("exec",QString::number((*it)->get_exec_address(),16));
			stream->writeAttribute("width",QString::number((*it)->get_width(),10));
			stream->writeAttribute("height",QString::number((*it)->get_height(),10));
			if(m_buildtype == BUILD_CART)
				stream->writeAttribute("block",QString::number((*it)->get_block(),10));
		}
		if((*it)->get_filetype() == PROJECT_FILE_ASCII)
		{
			stream->writeAttribute("type","ascii");
			stream->writeAttribute("load",QString::number((*it)->get_load_address(),16));
			stream->writeAttribute("exec",QString::number((*it)->get_exec_address(),16));
			if(m_buildtype == BUILD_CART)
				stream->writeAttribute("block",QString::number((*it)->get_block(),10));
		}
		stream->writeEndElement();
	}
}

int project::build(QPlainTextEdit* output)
{
	// TODO: Win32 path separator handling
	bool ret;
	QProcess* pasmo = new QProcess();
	QString pasmo_program = "pasmo";
	QStringList arg;
	// TODO add Z80 C cross-compilers (SDCC, Z88DK)?

	QList<project_file*>::iterator it;
	int exitcode;
	dsk_builder dsk(m_outfilename);
	cprbuilder cpr(m_outfilename);

	if(output != nullptr)
		output->clear();

	// set up working directory
	QDir dir(QDir::temp());
	dir.mkdir("cpc");
	dir.cd("cpc");

	// go through each file in the project, and if it's a source file, compile it
	// or if it's data, then copy it to destination, if necessary
	for(it=m_filelist.begin();it!=m_filelist.end();it++)
	{
		if(output != nullptr)
			output->appendPlainText(">>> File: "+(*it)->get_filename()+"\n");
		if((*it)->get_filetype() == PROJECT_FILE_SOURCE_ASM)
		{
			QStringList split_path = (*it)->get_filename().split(QDir::separator());
			QString basefilename = split_path.last();
			QStringList split_base = basefilename.split(".");
			QString basename = split_base.first();
			QStringList::iterator dir_iter;

			arg.clear();
			arg += "-v";  // verbose

			for(dir_iter=m_settings.include_begin();dir_iter!=m_settings.include_end();++dir_iter)
			{
				arg += "-I";
				arg += (*dir_iter);
			}

			arg += (*it)->get_filename();
			arg += (dir.absolutePath()+QDir::separator()+basename+".bin");

			pasmo->setProcessChannelMode(QProcess::MergedChannels);
			pasmo->setReadChannel(QProcess::StandardOutput);
			pasmo->start(pasmo_program,arg);
			if(!pasmo->waitForStarted())
			{
				QMessageBox msg(QMessageBox::Warning,"Build Error","Unable to run Pasmo.  Is it installed and available in your path?",QMessageBox::Ok);
				msg.exec();
				return -1;
			}
			if(output != nullptr)
			{
				while(pasmo->state() != QProcess::NotRunning)
				{
					char buffer[512];
					qint64 count;
					pasmo->waitForReadyRead();
					while(pasmo->canReadLine())
					{
						count = pasmo->readLine(buffer,512);
						if(count > 0)
							output->insertPlainText(buffer);
					}
				}
			}
			pasmo->waitForFinished();
			exitcode = pasmo->exitCode();
			if(exitcode != 0)
			{
				if(output != nullptr)
					output->appendPlainText(QString(">>> Pasmo returned an error (%1). Build not successful.\n").arg(exitcode));
				output->ensureCursorVisible();
				m_built = false;
				return -1;
			}
			// add file to disk image / cart image
			if(get_build_type() == BUILD_DISK)
			{
				dsk.add_file(dir.absolutePath()+QDir::separator()+basename+".bin",
					(*it)->get_load_address(),(*it)->get_exec_address());
			} 
			else if (get_build_type() == BUILD_CART)
			{
				cpr.add_block(dir.absolutePath()+QDir::separator()+basename+".bin", (*it)->get_block());
			}
			if(output != nullptr)
			{
				if(get_build_type() == BUILD_DISK)
				{
					QString l = QString::number((*it)->get_load_address(),16);
					QString e = QString::number((*it)->get_exec_address(),16);
					output->appendPlainText(QString(">>> Adding %1 to virtual disk, Load: 0x%2, Exec: 0x%3\n").arg(dir.absolutePath()+QDir::separator()+basename+".bin").arg(l).arg(e));
				}
				else if(get_build_type() == BUILD_CART)
				{
					QString b = QString::number((*it)->get_block(),10);
					output->appendPlainText(QString(">>> Adding %1 to virtual cartridge, Block: %2\n").arg(dir.absolutePath()+QDir::separator()+basename+".bin").arg(b));
				}
				output->ensureCursorVisible();
			}
		}
		if((*it)->get_filetype() == PROJECT_FILE_BINARY || (*it)->get_filetype() == PROJECT_FILE_GRAPHICS
				|| (*it)->get_filetype() == PROJECT_FILE_TILESET)
		{
			// add file to disk image / cart image
			if(get_build_type() == BUILD_DISK)
				dsk.add_file((*it)->get_filename(), (*it)->get_load_address(),(*it)->get_exec_address());
			else if(get_build_type() == BUILD_CART)
				cpr.add_block((*it)->get_filename(), (*it)->get_block());

			if(output != nullptr)
			{
				if(get_build_type() == BUILD_DISK)
				{
					QString l = QString::number((*it)->get_load_address(),16);
					QString e = QString::number((*it)->get_exec_address(),16);
					output->appendPlainText(QString(">>> Adding %1 to virtual disk, Load: 0x%2, Exec: 0x%3\n").arg((*it)->get_filename()).arg(l).arg(e));
				}
				else if(get_build_type() == BUILD_CART)
				{
					QString b = QString::number((*it)->get_block(),10);
					output->appendPlainText(QString(">>> Adding %1 to virtual cartridge, Block: %2\n").arg((*it)->get_filename()).arg(b));
				}
				output->ensureCursorVisible();
			}
		}
		if((*it)->get_filetype() == PROJECT_FILE_ASCII)
		{
			// add file to disk image
			if(get_build_type() == BUILD_DISK)
			{
				dsk.add_ascii_file((*it)->get_filename());
				if(output != nullptr)
				{
					output->appendPlainText(QString(">>> Adding ASCII file %1 to virtual disk\n").arg((*it)->get_filename()));
					output->ensureCursorVisible();
				}
			}
			else if(get_build_type() == BUILD_CART)
			{
				cpr.add_block((*it)->get_filename(),(*it)->get_block());
				if(output != nullptr)
				{
					QString b = QString::number((*it)->get_block(),10);
					output->appendPlainText(QString(">>> Adding ASCII file %1 to virtual cartridge, Block: %2\n").arg((*it)->get_filename().arg(b)));
					output->ensureCursorVisible();
				}
			}
		}
	}
	m_built = true;

	if(output != nullptr)
	{
		if(get_build_type() == BUILD_DISK)
			output->appendPlainText(QString(">>> Generating DSK image '%1'...").arg(m_outfilename));
		else if(get_build_type() == BUILD_CART)
			output->appendPlainText(QString(">>> Generating CPR image '%1'...").arg(m_outfilename));
	}
	output->ensureCursorVisible();

	if(get_build_type() == BUILD_DISK)
		ret = dsk.generate_dsk();
	else if(get_build_type() == BUILD_CART)
		ret = cpr.generate_cpr();
	else
		ret = false;


	if(ret == true)
		output->appendPlainText("OK\n");
	else
		output->appendPlainText("Failed\n");

	if(output != nullptr)
		output->appendPlainText(">>> Finished.\n");
	output->ensureCursorVisible();
	return 0;
}

project_file* project::find_file(QString file)
{
	QList<project_file*>::iterator it;

	for(it=m_filelist.begin();it!=m_filelist.end();it++)
	{
		if((*it)->get_filename() == file)
			return *it;
	}
	return nullptr;
}

unsigned int project::get_load_address(QString file)
{
	project_file* f = find_file(file);
	if(f != nullptr)
		return f->get_load_address();
	else
		return 0;
}

unsigned int project::get_exec_address(QString file)
{
	project_file* f = find_file(file);
	if(f != nullptr)
		return f->get_exec_address();
	else
		return 0;
}

int project::get_filetype(QString file)
{
	project_file* f = find_file(file);
	if(f != nullptr)
		return f->get_filetype();
	else
		return 0;
}

void project::set_load_address(QString file, unsigned int addr)
{
	project_file* f = find_file(file);
	if(f != nullptr)
		f->set_load_address(addr);
}

void project::set_exec_address(QString file, unsigned int addr)
{
	project_file* f = find_file(file);
	if(f != nullptr)
		f->set_exec_address(addr);
}

void project::set_filetype(QString file, int type)
{
	project_file* f = find_file(file);
	if(f != nullptr)
		return f->set_filetype(type);
}

// Cartridge Block map model implementation

BlockMapModel::BlockMapModel(QList<project_file*>* list, QObject* parent) :
	QAbstractTableModel(parent),
	m_filelist_ptr(list)
{
}

int BlockMapModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return m_filelist_ptr->size();
}

QVariant BlockMapModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	if (index.column() == 0)
		return m_filelist_ptr->at(index.row())->get_filename();

	if (index.column() == 1)
	{
		int block = m_filelist_ptr->at(index.row())->get_block();
		if(block != -1)
			return block;
		else
			return QString("Not Mapped");
	}

	return QVariant();
}

Qt::ItemFlags BlockMapModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flag = Qt::ItemIsEnabled;

	if(index.column() == 1)
		flag |= Qt::ItemIsEditable;

	return flag;
}

bool BlockMapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role != Qt::EditRole)
		return false;

	if(index.column() == 1)
	{
		m_filelist_ptr->at(index.row())->set_block(value.toInt());
		emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
		return true;
	}
	return false;
}
