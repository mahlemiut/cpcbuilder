/*
 * ui.cpp
 *
 *  Created on: 10/08/2011
 *      Author: bsr
 */

#include "ui.h"
#include "bineditor.h"
#include "gfxeditor.h"
#include "imgconvert.h"
#include "project.h"
#include <typeinfo>
#include <filesystem>

ui_main::ui_main(QWidget* parent)
	: QMainWindow(parent),
	  m_current_project(nullptr)
{
	QWidget* form;
	setupUi(this);
	setWindowTitle("CPC Builder");

	// initialise include directory(ies)  TODO: read settings in the settings class
    QSettings settings("cpcbuild.ini",QSettings::IniFormat);
    if(settings.contains("assembler/include_dir"))
		m_app_settings.set_includedir(settings.value("assembler/include_dir").toString());
    else
    {
        QDir default_incl("."+QString(QDir::separator())+"include");
		m_app_settings.set_includedir(default_incl.absolutePath());
    }
	if(settings.contains("emulator/emu_path"))
		m_app_settings.set_emu_path(settings.value("emulator/emu_path").toString());
	if(settings.contains("emulator/emu_ospath"))
		m_app_settings.set_emu_ospath(settings.value("emulator/emu_ospath").toString());
	if(settings.contains("emulator/model"))
		m_app_settings.set_emu_model(settings.value("emulator/model").toInt());
	if(settings.contains("emulator/exp"))
		m_app_settings.set_emu_exp(settings.value("emulator/exp").toInt());

	// load firmware function descriptions into memory
	load_descriptions();

    // pre-load some dialogs
	QAction* act_boot = new QAction("Bootable",this);
	connect(act_boot,SIGNAL(triggered()),this,SLOT(SetBootable()));
	tree_files->addAction(act_boot);
	QAction* act1 = new QAction("Properties...",this);
	connect(act1,SIGNAL(triggered()),this,SLOT(EditProperties()));
	tree_files->addAction(act1);
	QFile f1(":/forms/ide_newproject.ui");
	f1.open(QFile::ReadOnly);
	form = loader.load(&f1, this);
	m_dlg_newproject = dynamic_cast<QDialog*>(form);
	f1.close();
	QFile f2(":/forms/ide_fileprop.ui");
	f2.open(QFile::ReadOnly);
	form = loader.load(&f2, this);
	m_dlg_fileprop = dynamic_cast<QDialog*>(form);
	f2.close();
	QFile f3(":/forms/ide_buildoptions.ui");
	f3.open(QFile::ReadOnly);
	form = loader.load(&f3, this);
	m_dlg_buildoptions = dynamic_cast<QDialog*>(form);
	f3.close();
    QFile f4(":/forms/compiler_setup_dialog.ui");
    f4.open(QFile::ReadOnly);
    form = loader.load(&f4, this);
    m_dlg_compileoptions = dynamic_cast<QDialog*>(form);
	f4.close();
	QFile f5(":/forms/ide_emuprop.ui");
	f5.open(QFile::ReadOnly);
	form = loader.load(&f5, this);
	m_dlg_emuoptions = dynamic_cast<QDialog *>(form);
	f5.close();
}

ui_main::~ui_main()
{
}

void ui_main::load_descriptions()
{
	// make sure the list is empty
	m_firmware_desc.clear();

	// find firmware.inc
	QString fname;
	appsettings s = settings();

	// go through include directories
	for(QStringList::iterator it = s.include_begin();it != s.include_end();++it)
	{
		fname = (*it);
		fname.append(QDir::separator());
		fname.append("firmware.inc");
		QFile qf(fname);
		if(qf.open(QIODevice::ReadOnly))
		{
			// file exists in this include directory
			read_descriptions_from_file(qf);
			qf.close();
			return;
		}
	}
}

void ui_main::read_descriptions_from_file(QFile& qf)
{
	char buffer[512];
	QString desc,key;
	int64_t len;

	while((len = qf.readLine(buffer,512)) != -1)
	{
		QString str = buffer;
		if(!str.startsWith(QString("; *")) && len != 0)  // ignore these lines
		{
			if(str.startsWith(QChar(';')))
				desc.append(str);
			else
			{
				key = str.split(QString("\t")).at(0);
				m_firmware_desc[key.toStdString()] = desc.toStdString();
				desc.clear();
				key.clear();
			}
		}
	}
}

void ui_main::NewProject()
{
	int ret = QDialog::Rejected;

	QString name, fname;
	fname = QFileDialog::getSaveFileName(this,tr("New Project..."),"~/",tr("Project files (*.cpc);;All files (*.*)"));
	ret = m_dlg_newproject->exec();

	if(ret == QDialog::Accepted && !fname.isEmpty())
	{
        QLineEdit* dlg_name = m_dlg_newproject->findChild<QLineEdit*>("dlg_project_name");

		name = dlg_name->text();
		// new project
		m_current_project = new project(name,fname,m_app_settings);
		if(m_current_project == nullptr)
			return;
		m_project_filename = fname;
		QString fpath(m_project_filename.section(QDir::separator(),0,-2));
		m_oldpath = QDir::current();
		QDir::setCurrent(fpath);
		tree_files->clear();
		QTreeWidgetItem* item = new QTreeWidgetItem(QTreeWidgetItem::Type);
		QFont font("Sans",14,QFont::Bold);
		item->setText(0,name+" files");
		item->setToolTip(0,fname);
		item->setFont(0,font);
		tree_files->addTopLevelItem(item);
		menu_project_close->setEnabled(true);
		menu_project_build->setEnabled(true);
		menu_project_buildoptions->setEnabled(true);
		menu_project_test->setEnabled(true);
	}
}

void ui_main::OpenProject()
{
	QString name;
	QList<QString> filelist;
	QList<QString>::iterator it;
	m_project_filename = QFileDialog::getOpenFileName(this,tr("Open Project..."),"~/",tr("Projects (*.cpc);;All Files (*.*)"));
	QString fname;
	QString fpath(m_project_filename.section(QDir::separator(),0,-2));
	int buildtype = BUILD_DISK;

	if(m_project_filename.isEmpty())
		return;

	if(m_current_project != nullptr)
		CloseProject();

	QFile f(m_project_filename);
	f.open(QFile::ReadOnly);
	m_oldpath = QDir::current();
	QDir::setCurrent(fpath);
	QXmlStreamReader stream(&f);
	stream.readNextStartElement();
	if(stream.name() != "project")  // first element must be project name
	{
		f.close();
		return;
	}
	else
	{
		QXmlStreamAttributes attr = stream.attributes();
		if(attr.hasAttribute("name"))
			name = attr.value("name").toString();
		if(attr.hasAttribute("buildtype"))
		{
			if(attr.value("buildtype").toString() == "disk")
				buildtype = BUILD_DISK;
			else if(attr.value("buildtype").toString() == "cart")
				buildtype = BUILD_CART;
		}
	}

	m_current_project = new project(name,m_project_filename,m_app_settings);
	filelist.clear();
	m_current_project->set_build_type(buildtype);
	while(stream.readNextStartElement())
	{
		if(stream.name() == "file")
		{
			QXmlStreamAttributes attr = stream.attributes();
			if(attr.hasAttribute("name"))
			{
				filelist.append(attr.value("name").toString());
				fname = attr.value("name").toString();
			}
			project_file* pfile = new project_file(fname);
			if(attr.hasAttribute("type"))
			{
				if(attr.value("type").toString() == "asm")
				{
					pfile->set_filetype(PROJECT_FILE_SOURCE_ASM);
					if(!check_for_open_file(fname))
					{
						QStringList split = pfile->get_filename().split(QDir::separator());
						ui_QMdiSubWindow* subwin = CreateWindowAsm();
						subwin->load_text(pfile->get_filename());
						subwin->setWindowTitle("Z80 Assembly - " + split.last());
						if(attr.hasAttribute("load"))
							pfile->set_load_address(attr.value("load").toString().toUInt(nullptr,16));
						if(attr.hasAttribute("exec"))
							pfile->set_exec_address(attr.value("exec").toString().toUInt(nullptr,16));
						pfile->set_filename(attr.value("name").toString());
						if(buildtype == BUILD_CART)
							pfile->set_block(attr.value("block").toInt());
						m_current_project->add_file(pfile);
					}
				}
				if(attr.value("type").toString() == "gfx")
				{
					pfile->set_filetype(PROJECT_FILE_GRAPHICS);
					if(!check_for_open_file(pfile->get_filename()))
					{
						QStringList split = pfile->get_filename().split(QDir::separator());
						ui_QMdiSubWindow* subwin = CreateWindowGraphics();
						if(attr.hasAttribute("width") && attr.hasAttribute("height"))
						{
							subwin->load_gfx(pfile->get_filename(),attr.value("width").toString().toInt(),attr.value("height").toString().toInt());
							pfile->set_size(attr.value("width").toString().toInt(),attr.value("height").toString().toInt());
						}
						else
							subwin->load_gfx(pfile->get_filename());
						subwin->setWindowTitle("Graphics Editor (Screen) - " + split.last());
						if(attr.hasAttribute("load"))
							pfile->set_load_address(attr.value("load").toString().toUInt(nullptr,16));
						if(attr.hasAttribute("exec"))
							pfile->set_exec_address(attr.value("exec").toString().toUInt(nullptr,16));
						pfile->set_filename(attr.value("name").toString());
						if(buildtype == BUILD_CART)
							pfile->set_block(attr.value("block").toInt());
						m_current_project->add_file(pfile);
					}
				}
				if(attr.value("type").toString() == "binary")
				{
					pfile->set_filetype(PROJECT_FILE_BINARY);
					if(!check_for_open_file(pfile->get_filename()))
					{
						QStringList split = pfile->get_filename().split(QDir::separator());
						ui_QMdiSubWindow* subwin = CreateWindowBinary();
						subwin->load_binary(pfile->get_filename());
						subwin->setWindowTitle("Binary File - " + split.last());
					}
					if(attr.hasAttribute("load"))
						pfile->set_load_address(attr.value("load").toString().toUInt(nullptr,16));
					if(attr.hasAttribute("exec"))
						pfile->set_exec_address(attr.value("exec").toString().toUInt(nullptr,16));
					pfile->set_filename(attr.value("name").toString());menu_project_test->setEnabled(true);
					if(buildtype == BUILD_CART)
						pfile->set_block(attr.value("block").toInt());
					m_current_project->add_file(pfile);
				}
				if(attr.value("type").toString() == "tileset")
				{
					pfile->set_filetype(PROJECT_FILE_TILESET);
					if(!check_for_open_file(pfile->get_filename()))
					{
						QStringList split = pfile->get_filename().split(QDir::separator());
						ui_QMdiSubWindow* subwin = CreateWindowTileset();
						subwin->load_tileset(pfile->get_filename(),attr.value("width").toString().toInt(),attr.value("height").toString().toInt());
						pfile->set_size(attr.value("width").toString().toInt(),attr.value("height").toString().toInt());
						subwin->setWindowTitle("Tileset Editor - " + split.last());
					}
					if(attr.hasAttribute("load"))
						pfile->set_load_address(attr.value("load").toString().toUInt(nullptr,16));
					if(attr.hasAttribute("exec"))
						pfile->set_exec_address(attr.value("exec").toString().toUInt(nullptr,16));
					pfile->set_filename(attr.value("name").toString());
					if(buildtype == BUILD_CART)
						pfile->set_block(attr.value("block").toInt());
					m_current_project->add_file(pfile);
				}
				if(attr.value("type").toString() == "ascii")
				{
					pfile->set_filetype(PROJECT_FILE_ASCII);
					if(!check_for_open_file(pfile->get_filename()))
					{
						QStringList split = pfile->get_filename().split(QDir::separator());
						ui_QMdiSubWindow* subwin = CreateWindowASCII();
						subwin->load_text(pfile->get_filename());
						subwin->setWindowTitle("ASCII file - " + split.last());
						if(attr.hasAttribute("load"))
							pfile->set_load_address(attr.value("load").toString().toUInt(nullptr,16));
						if(attr.hasAttribute("exec"))
							pfile->set_exec_address(attr.value("exec").toString().toUInt(nullptr,16));
						pfile->set_filename(attr.value("name").toString());
						if(buildtype == BUILD_CART)
							pfile->set_block(attr.value("block").toInt());
						m_current_project->add_file(pfile);
					}
				}
			}
		}menu_project_test->setEnabled(true);
		if(stream.name() == "output")
		{
			QXmlStreamAttributes attr = stream.attributes();
			if(attr.hasAttribute("filename"))
			{
				m_current_project->set_output_filename(attr.value("filename").toString());
			}
		}
		stream.skipCurrentElement();
	}
	f.close();
	if(stream.hasError())
	{
		QMessageBox msg(QMessageBox::Warning,"XML Error",stream.errorString(),QMessageBox::Ok);
		msg.exec();
	}
	tree_files->clear();
	QTreeWidgetItem* item = new QTreeWidgetItem(QTreeWidgetItem::Type);
	QFont font("Sans",14,QFont::Bold);
	item->setText(0,name+" files");
	item->setToolTip(0,m_project_filename);
	item->setFont(0,font);
	tree_files->addTopLevelItem(item);
	menu_project_close->setEnabled(true);
	menu_project_build->setEnabled(true);
	menu_project_buildoptions->setEnabled(true);

	for(it=filelist.begin();it!=filelist.end();it++)
	{
		if(QFile::exists(*it))
		{
			QStringList split = (*it).split(QDir::separator());
			QString shortname = split.last();
			// update project file list
			QTreeWidgetItem* item = tree_files->topLevelItem(0);
			if(item == nullptr)
				return;  // this shouldn't happen
			QTreeWidgetItem* child = new QTreeWidgetItem(item,QTreeWidgetItem::Type);
			child->setText(0,shortname);
			child->setToolTip(0,*it);
			item->addChild(child);
		}
	}

	setWindowTitle("[" + m_project_filename + "] - CPC Builder");
	menu_project_close->setEnabled(true);
	menu_project_build->setEnabled(true);
	menu_project_buildoptions->setEnabled(true);
}

void ui_main::SaveProject()
{
	QString fname = m_current_project->get_filename();
	QFile f(fname);
	QDir curr(fname.section(QDir::separator(),0,-2));
	f.open(QFile::WriteOnly);
	QXmlStreamWriter stream(&f);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();

	stream.writeStartElement("project");
	stream.writeAttribute("name",m_current_project->get_name());
	if(m_current_project->get_build_type() == BUILD_DISK)
		stream.writeAttribute("buildtype","disk");
	else if(m_current_project->get_build_type() == BUILD_CART)
		stream.writeAttribute("buildtype","cart");

	m_current_project->xml_filelist(&stream, curr);

	stream.writeStartElement("output");
	stream.writeAttribute("filename",m_current_project->get_output_filename());

	stream.writeEndElement();

	stream.writeEndElement();

	stream.writeEndDocument();
	f.close();
}

void ui_main::CloseProject()
{
	if(m_current_project == nullptr)
		return;
	SaveProject();
	tree_files->clear();
	delete(m_current_project);
	QDir::setCurrent(m_oldpath.path());  // restore previous working directory
	m_current_project = nullptr;
	menu_project_close->setEnabled(false);
	menu_project_build->setEnabled(false);
	menu_project_buildoptions->setEnabled(false);
	menu_project_test->setEnabled(false);
}

void ui_main::AddToProject()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(m_current_project == nullptr)
		return;
	if(subwin == nullptr)
		return;
	QString fname = subwin->get_filename();
	QStringList split = fname.split(QDir::separator());
	QString shortname = split.last();
	if(fname.isEmpty())
		return;
	if(m_current_project->check_for_file(fname) == true)
		return;  // file is already in project
	if(subwin->get_doctype() == PROJECT_FILE_GRAPHICS)
	{
		gfxeditor* gfx = dynamic_cast<gfxeditor*>(subwin->widget());
		if(gfx != nullptr)
			m_current_project->add_gfx_file(fname,gfx->get_width(),gfx->get_height());
	}
	else if(subwin->get_doctype() == PROJECT_FILE_TILESET)
	{
		tileeditor* gfx = dynamic_cast<tileeditor*>(subwin->widget());
		if(gfx != nullptr)
			m_current_project->add_tileset_file(fname,gfx->get_width(),gfx->get_height());
	}
	else
		m_current_project->add_file(fname);
	// update project file list
	QTreeWidgetItem* item = tree_files->topLevelItem(0);
	if(item == nullptr)
		return;  // this shouldn't happen
	QTreeWidgetItem* child = new QTreeWidgetItem(item,QTreeWidgetItem::Type);
	child->setText(0,shortname);
	child->setToolTip(0,fname);
	item->addChild(child);
	SaveProject();
}

void ui_main::BuildProject()
{
	if(m_current_project != nullptr)
		m_current_project->build(text_console);
}

ui_QMdiSubWindow* ui_main::CreateWindow(int doctype)
{
	switch(doctype)
	{
	case PROJECT_FILE_SOURCE_ASM:
		return CreateWindowAsm();
	case PROJECT_FILE_BINARY:
		return CreateWindowBinary();
	case PROJECT_FILE_GRAPHICS:
		return CreateWindowGraphics();
	case PROJECT_FILE_ASCII:
		return CreateWindowASCII();
	}
	return nullptr;
}

ui_QMdiSubWindow* ui_main::CreateWindowAsm()
{
	ui_QMdiSubWindow* subwin = new ui_QMdiSubWindow(PROJECT_FILE_SOURCE_ASM,this);
	QTextEdit* widget = new QTextEdit;
	QFont font("Courier New",14);
	widget->setFont(font);
	widget->setLineWrapMode(QTextEdit::NoWrap);
	widget->setContextMenuPolicy(Qt::ActionsContextMenu);
	QAction* act1 = new QAction("Cut",this);
	connect(act1,SIGNAL(triggered()),widget,SLOT(cut()));
	QAction* act2 = new QAction("Copy",this);
	connect(act2,SIGNAL(triggered()),widget,SLOT(copy()));
	QAction* act3 = new QAction("Paste",this);
	connect(act3,SIGNAL(triggered()),widget,SLOT(paste()));
	QAction* sep = new QAction(this);
	sep->setSeparator(true);
	QAction* act4 = new QAction("Add file to current project",this);
	connect(act4,SIGNAL(triggered()),this,SLOT(AddToProject()));
	widget->addAction(act1);
	widget->addAction(act2);
	widget->addAction(act3);
	widget->addAction(sep);
	widget->addAction(act4);
	subwin->setWidget(widget);
	subwin->setMinimumSize(400,400);
	subwin->setAttribute(Qt::WA_DeleteOnClose);
	connect(widget,SIGNAL(textChanged()),subwin,SLOT(contents_changed())); // connect modification signal
	connect(widget,SIGNAL(cursorPositionChanged()),subwin,SLOT(cursor_changed()));
	syntax = new highlighter(widget->document());
	mdi_main->addSubWindow(subwin);
	subwin->show();
	m_doclist.append(subwin);
	return subwin;
}

ui_QMdiSubWindow* ui_main::CreateWindowBinary()
{
	ui_QMdiSubWindow* subwin = new ui_QMdiSubWindow(PROJECT_FILE_BINARY,this);
	QScrollArea* scroll = new QScrollArea(subwin);
	bineditor* widget = new bineditor(subwin);

	widget->setContextMenuPolicy(Qt::ActionsContextMenu);
	QAction* act4 = new QAction("Add file to current project",this);
	connect(act4,SIGNAL(triggered()),this,SLOT(AddToProject()));
	widget->addAction(act4);
	scroll->setWidget(widget);
	subwin->setWidget(scroll);
	subwin->setMinimumSize(400,400);
	subwin->setAttribute(Qt::WA_DeleteOnClose);
	//connect(widget,SIGNAL(textChanged()),subwin,SLOT(contents_changed()));
	mdi_main->addSubWindow(subwin);
	subwin->show();
	m_doclist.append(subwin);
	return subwin;
}

ui_QMdiSubWindow* ui_main::CreateWindowGraphics()
{
	ui_QMdiSubWindow* subwin = new ui_QMdiSubWindow(PROJECT_FILE_GRAPHICS,this);
	gfxeditor* widget = new gfxeditor(subwin);

	widget->setContextMenuPolicy(Qt::ActionsContextMenu);
	palmenu = new QAction("12-bit palette",this);
	palmenu->setCheckable(true);
	palmenu->setChecked(false);
	connect(palmenu,SIGNAL(triggered()),this,SLOT(TogglePal()));
	widget->addAction(palmenu);
	QAction* act2 = new QAction("Import CPC palette...",this);
	connect(act2,SIGNAL(triggered()),this,SLOT(ImportPal()));
	widget->addAction(act2);
	QAction* act3 = new QAction("Import 12-bit CPC+ palette...",this);
	connect(act3,SIGNAL(triggered()),this,SLOT(ImportPalPlus()));
	widget->addAction(act3);
	QAction* act5 = new QAction("Export CPC palette to clipboard",this);
	connect(act5,SIGNAL(triggered()),this,SLOT(ExportPalClip()));
	widget->addAction(act5);
	QAction* sep = new QAction(this);
	sep->setSeparator(true);
	widget->addAction(sep);
	QAction* act4 = new QAction("Add file to current project",this);
	connect(act4,SIGNAL(triggered()),this,SLOT(AddToProject()));
	widget->addAction(act4);
	subwin->setWidget(widget);
	subwin->setMinimumSize(400,400);
	subwin->setAttribute(Qt::WA_DeleteOnClose);
	//connect(widget,SIGNAL(textChanged()),subwin,SLOT(contents_changed()));
	mdi_main->addSubWindow(subwin);
	subwin->show();
	m_doclist.append(subwin);
	return subwin;
}

ui_QMdiSubWindow* ui_main::CreateWindowTileset()
{
	ui_QMdiSubWindow* subwin = new ui_QMdiSubWindow(PROJECT_FILE_TILESET,this);
	tileeditor* widget = new tileeditor(subwin);

	widget->setContextMenuPolicy(Qt::ActionsContextMenu);
	palmenu = new QAction("12-bit palette",this);
	palmenu->setCheckable(true);
	palmenu->setChecked(false);
	connect(palmenu,SIGNAL(triggered()),this,SLOT(TogglePal()));
	widget->addAction(palmenu);
	QAction* act2 = new QAction("Import CPC palette...",this);
	connect(act2,SIGNAL(triggered()),this,SLOT(ImportPal()));
	widget->addAction(act2);
	QAction* act3 = new QAction("Import 12-bit CPC+ palette...",this);
	connect(act3,SIGNAL(triggered()),this,SLOT(ImportPalPlus()));
	widget->addAction(act3);
	QAction* act5 = new QAction("Export CPC palette to clipboard",this);
	connect(act5,SIGNAL(triggered()),this,SLOT(ExportPalClip()));
	widget->addAction(act5);
	QAction* sep = new QAction(this);
	sep->setSeparator(true);
	widget->addAction(sep);
	QAction* act4 = new QAction("Add file to current project",this);
	connect(act4,SIGNAL(triggered()),this,SLOT(AddToProject()));
	widget->addAction(act4);
	subwin->setWidget(widget);
	subwin->setMinimumSize(400,400);
	subwin->setAttribute(Qt::WA_DeleteOnClose);
	//connect(widget,SIGNAL(textChanged()),subwin,SLOT(contents_changed()));
	mdi_main->addSubWindow(subwin);
	subwin->show();
	m_doclist.append(subwin);
	return subwin;
}

ui_QMdiSubWindow* ui_main::CreateWindowASCII()
{
	ui_QMdiSubWindow* subwin = new ui_QMdiSubWindow(PROJECT_FILE_ASCII,this);
	QTextEdit* widget = new QTextEdit;
	QFont font("Courier New",14);
	widget->setFont(font);
	widget->setLineWrapMode(QTextEdit::NoWrap);
	widget->setContextMenuPolicy(Qt::ActionsContextMenu);
	QAction* act1 = new QAction("Cut",this);
	connect(act1,SIGNAL(triggered()),widget,SLOT(cut()));
	QAction* act2 = new QAction("Copy",this);
	connect(act2,SIGNAL(triggered()),widget,SLOT(copy()));
	QAction* act3 = new QAction("Paste",this);
	connect(act3,SIGNAL(triggered()),widget,SLOT(paste()));
	QAction* sep = new QAction(this);
	sep->setSeparator(true);
	QAction* act4 = new QAction("Add file to current project",this);
	connect(act4,SIGNAL(triggered()),this,SLOT(AddToProject()));
	widget->addAction(act1);
	widget->addAction(act2);
	widget->addAction(act3);
	widget->addAction(sep);
	widget->addAction(act4);
	subwin->setWidget(widget);
	subwin->setMinimumSize(400,400);
	subwin->setAttribute(Qt::WA_DeleteOnClose);
	connect(widget,SIGNAL(textChanged()),subwin,SLOT(contents_changed()));
	mdi_main->addSubWindow(subwin);
	subwin->show();
	m_doclist.append(subwin);
	return subwin;
}

bool ui_main::check_for_open_file(QString filename)
{
	QList<ui_QMdiSubWindow*>::iterator it;
	// step through each open file
	for(it=m_doclist.begin();it!=m_doclist.end();it++)
	{
		if((*it)->get_filename() == filename)
			return true;
	}
	return false;
}

void ui_main::NewFile()
{
	CreateWindowAsm();
}

void ui_main::NewGfxFile()
{
	int w,h;

	unsigned char* data;
	ui_QMdiSubWindow* subwin = CreateWindowGraphics();
	gfxeditor* widget = dynamic_cast<gfxeditor*>(subwin->widget());

	// TODO: add support for other sizes, in particular overscan
	w = 80;  // normal screen size
	h = 200;
	widget->set_size(w,h);
	data = reinterpret_cast<unsigned char*>(malloc(16384));  // screen size is 16kB (not all is visible)
	memset(data,0,16384);
	widget->set_data(data,16384);
	widget->draw_scene();
}

void ui_main::NewTilesetFile()
{
	int w,h;
	QUiLoader loader;
	QFile uif(":/forms/ide_tilesize.ui");
	QWidget* form;

	uif.open(QFile::ReadOnly);
	form = loader.load(&uif, this);
	uif.close();

	if(form)
	{
		QDialog* dlg = dynamic_cast<QDialog*>(form);
		if(dlg)
		{
			if(dlg->exec() == QDialog::Rejected)
				return;
            QLineEdit* dlg_width = dlg->findChild<QLineEdit*>("dlg_tile_width");
            QLineEdit* dlg_height = dlg->findChild<QLineEdit*>("dlg_tile_height");
			w = dlg_width->text().toUInt(nullptr,10);
			h = dlg_height->text().toUInt(nullptr,10);
		}
		else
			return;
	}
	else
		return;

	unsigned char* data;
	ui_QMdiSubWindow* subwin = CreateWindowTileset();
	tileeditor* widget = dynamic_cast<tileeditor*>(subwin->widget());

	widget->set_size(w,h);
	data = reinterpret_cast<unsigned char*>(malloc(w*h));
	memset(data,0,w*h);
	widget->set_data(data,w*h);
	widget->draw_scene();
}

void ui_main::OpenFile()
{
	QString filename;
	QStringList filelist;
	QFileDialog dlg(this);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    QLayout* dlg_layout = dlg.layout();
	QLabel* dlg_label = new QLabel;
	dlg_label->setText(tr("Open As:"));
	dlg_layout->addWidget(dlg_label);
	QComboBox* dlg_combo = new QComboBox;
	dlg_combo->addItem(tr("ASM source"));
	dlg_combo->addItem(tr("Binary data"));
	dlg_combo->addItem(tr("Graphics"));
	dlg_combo->addItem(tr("ASCII"));
	dlg_combo->addItem(tr("Tileset"));
	dlg_layout->addWidget(dlg_combo);
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setNameFilter(tr("Source files (*.asm);;Binary files (*.bin);;Graphical data (*.gfx *.bin);;ASCII data (*.txt);;All Files (*.*)"));
	if(dlg.exec())
	{
		if(dlg_combo->currentIndex() == 0)
		{
			filelist = dlg.selectedFiles();
			filename = filelist.first();
			if(filename.isEmpty())
				return;
			if(check_for_open_file(filename))
				return;
			ui_QMdiSubWindow* subwin = CreateWindowAsm();
			QStringList split = filename.split(QDir::separator());
			QString shortname = split.last();

			subwin->load_text(filename);
			subwin->setWindowTitle("Z80 Assembly - " + shortname);
		}
		else if(dlg_combo->currentIndex() == 1)  // Binary
		{
			filelist = dlg.selectedFiles();
			filename = filelist.first();
			if(filename.isEmpty())
				return;
			if(check_for_open_file(filename))
				return;

			ui_QMdiSubWindow* subwin = CreateWindowBinary();
			QStringList split = filename.split(QDir::separator());
			QString shortname = split.last();

			subwin->load_binary(filename);
			subwin->setWindowTitle("Binary file - " + shortname);
		}
		else if(dlg_combo->currentIndex() == 2)  // Graphics
		{
			filelist = dlg.selectedFiles();
			filename = filelist.first();
			if(filename.isEmpty())
				return;
			if(check_for_open_file(filename))
				return;

			ui_QMdiSubWindow* subwin = CreateWindowGraphics();
			QStringList split = filename.split(QDir::separator());
			QString shortname = split.last();

			if(subwin->load_gfx(filename) == false)
			{
				subwin->close();  // close window if load fails.
				return;
			}
			subwin->setWindowTitle("Graphics Editor (Screen) - " + shortname);
		}
		else if(dlg_combo->currentIndex() == 3)  // ASCII
		{
			filelist = dlg.selectedFiles();
			filename = filelist.first();
			if(filename.isEmpty())
				return;
			if(check_for_open_file(filename))
				return;

			ui_QMdiSubWindow* subwin = CreateWindowASCII();
			QStringList split = filename.split(QDir::separator());
			QString shortname = split.last();

			if(subwin->load_text(filename) == false)
			{
				subwin->close();  // close window if load fails.
				return;
			}
			subwin->setWindowTitle("ASCII file - " + shortname);
		}
		else if(dlg_combo->currentIndex() == 4)  // Tileset
		{
			filelist = dlg.selectedFiles();
			filename = filelist.first();
			if(filename.isEmpty())
				return;
			if(check_for_open_file(filename))
				return;

			ui_QMdiSubWindow* subwin = CreateWindowTileset();
			QStringList split = filename.split(QDir::separator());
			QString shortname = split.last();

			if(subwin->load_tileset(filename) == false)
			{
				subwin->close();  // close window if load fails.
				return;
			}
			subwin->setWindowTitle("Tileset Editor - " + shortname);
		}
	}
}

void ui_main::SaveFile()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(subwin == nullptr)
		return;
	QString filename = subwin->get_filename();
	QString typestr;
	if(filename.isEmpty())
		SaveFileAs();
	// if filename is still empty, then the Save As dialog must have been cancelled.
	if(filename.isEmpty())
		return;
	if(subwin->get_doctype() == PROJECT_FILE_SOURCE_ASM)
	{
		subwin->save_text(filename);
		// we have a set filename now, so enable the AddToProject menuitem on the current subwindow widget
		QTextEdit* widget = dynamic_cast<QTextEdit*>(subwin->widget());
		QList<QAction*> actlist = widget->actions();
		QAction* act = actlist[3];  // TODO: maybe there's a better way to actually search for a list item
		act->setEnabled(true);
		typestr = "Z80 Assembly - ";
	}
	else if(subwin->get_doctype() == PROJECT_FILE_GRAPHICS)
	{
		subwin->save_gfx(filename);
		// we have a set filename now, so enable the AddToProject menuitem on the current subwindow widget
		gfxeditor* widget = dynamic_cast<gfxeditor*>(subwin->widget());
		QList<QAction*> actlist = widget->actions();
		QAction* act = actlist[0];  // TODO: maybe there's a better way to actually search for a list item
		act->setEnabled(true);
		typestr = "Graphics Editor (Screen) - ";
	}
	else if(subwin->get_doctype() == PROJECT_FILE_TILESET)
	{
		subwin->save_tileset(filename);
		// we have a set filename now, so enable the AddToProject menuitem on the current subwindow widget
		tileeditor* widget = dynamic_cast<tileeditor*>(subwin->widget());
		QList<QAction*> actlist = widget->actions();
		QAction* act = actlist[0];  // TODO: maybe there's a better way to actually search for a list item
		act->setEnabled(true);
		typestr = "Tileset Editor - ";
	}
	QStringList split = filename.split(QDir::separator());
	subwin->setWindowTitle(typestr + split.last());
}

void ui_main::SaveFileAs()
{
	QString filename;
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(subwin == nullptr)
		return;
	filename = QFileDialog::getSaveFileName(this,tr("Save File As..."),"~/",tr("Source files (*.asm);;All Files (*.*)"));
	if(filename.isEmpty())
		return;
	rename_project_file(subwin->get_filename(),filename);  // update project is one is loaded
	subwin->set_filename(filename);
	SaveFile();
}

void ui_main::CloseFile()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(subwin == nullptr)
		return;
	subwin->close();
}

void ui_main::Find()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(subwin == nullptr)
		return;
	if(subwin->get_doctype() != PROJECT_FILE_SOURCE_ASM)
		return;
	QDialog dlg(this);
	dlg.setModal(true);
	QHBoxLayout* layout = new QHBoxLayout;
	QLabel* label = new QLabel("Find:");
	layout->addWidget(label);
	QLineEdit* searchstr = new QLineEdit();
	layout->addWidget(searchstr);
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
	layout->addWidget(buttonBox);
	dlg.setLayout(layout);
	if(dlg.exec())
	{
		QTextEdit* widget = dynamic_cast<QTextEdit*>(subwin->widget());
		widget->find(searchstr->text());
		m_searchstring = searchstr->text();
	}
}

void ui_main::FindNext()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(m_searchstring.isEmpty())
		return;
	if(subwin == nullptr)
		return;
	if(subwin->get_doctype() != PROJECT_FILE_SOURCE_ASM)
		return;

	QTextEdit* widget = dynamic_cast<QTextEdit*>(subwin->widget());
	widget->find(m_searchstring);
}

void ui_main::About()
{
	QUiLoader loader;
	QFile f(":/forms/ide_about.ui");
	QWidget* form;

	f.open(QFile::ReadOnly);
	form = loader.load(&f, this);
	f.close();
	if(form)
	{
		QDialog* dlg = dynamic_cast<QDialog*>(form);
		if(dlg)
		{
			dlg->setWindowTitle(VER);
			dlg->exec();
		}
	}
}

void ui_main::Exit()
{
	close();
}

void ui_main::CloseAll()
{
	// close all open windows
	foreach(QMdiSubWindow* win,mdi_main->subWindowList())
	{
		if(win != nullptr)
		{
			win->close();
		}
	}
}

void ui_main::ImportScr()
{
	QString filename;
	QStringList filelist;
	QFileDialog dlg(this);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setNameFilter(tr("CPC screen data (*.scr);;All Files (*.*)"));
	if(dlg.exec())
	{
		filelist = dlg.selectedFiles();
		filename = filelist.first();
		if(filename.isEmpty())
			return;
		if(check_for_open_file(filename))
			return;

		ui_QMdiSubWindow* subwin = CreateWindowGraphics();
		QStringList split = filename.split(QDir::separator());
		QString shortname = split.last();

		if(subwin->import_scr(filename) == false)
		{
			subwin->close();  // close window if import fails.
			return;
		}
		subwin->setWindowTitle("Graphics Editor (Screen) - " + shortname);
	}
}

void ui_main::TestEmu()
{
	QString cmd;
	QString drv;
	QStringList args;

	switch(m_app_settings.emu_model())
	{
	case appsettings::EMUMODEL_464:
		drv = "cpc464";
		break;
	case appsettings::EMUMODEL_664:
		drv = "cpc664";
		break;
	case appsettings::EMUMODEL_6128:
		drv = "cpc6128";
		break;
	case appsettings::EMUMODEL_464PLUS:
		drv = "cpc464p";
		break;
	case appsettings::EMUMODEL_6128PLUS:
		drv = "cpc6128p";
		break;
	}
	cmd = m_app_settings.emu_path();
	args << drv << "-window";
	if(m_app_settings.emu_model() == appsettings::EMUMODEL_464)
		args << "-exp" << "ddi1" << "-flop";
	else if(m_current_project->get_build_type() == BUILD_CART)
		args << "-cart";
	else
		args << "-flop1";
	args << QString(std::filesystem::current_path().c_str()) + QDir::separator() + m_current_project->get_output_filename();
	if(m_current_project->get_build_type() == BUILD_DISK)
		if(m_app_settings.emu_model() == appsettings::EMUMODEL_464PLUS || m_app_settings.emu_model() == appsettings::EMUMODEL_6128PLUS)
			args << "-cart" << m_app_settings.emu_ospath();
	// autoboot
	QStringList basename = m_current_project->get_bootable_filename()->get_filename().split(".");
	QString outfile = basename.first() + ".bin";
	if(m_current_project->get_bootable_filename() != nullptr)
		args << "-autoboot_command"  << QString("memory %1:load\\\"%2\\\":call %3\\n").arg(m_current_project->get_bootable_filename()->get_load_address() - 1) \
			 .arg(outfile).arg(m_current_project->get_bootable_filename()->get_exec_address());
	args << "-verbose";
//	text_console->appendPlainText(args.join(" "));
//	text_console->appendPlainText("\n");

	Qt::WindowStates wnd = windowState();
	setWindowState(Qt::WindowMinimized);

	QProcess* proc = new QProcess();
	proc->setWorkingDirectory(cmd.section('/',0,-2));
	proc->setProcessChannelMode(QProcess::MergedChannels);
	proc->setReadChannel(QProcess::StandardOutput);
	proc->start(cmd, args);
	if(text_console != nullptr)
	{
		while(proc->state() != QProcess::NotRunning)
		{
			char buffer[512];
			qint64 count;
			proc->waitForReadyRead();
			while(proc->canReadLine())
			{
				count = proc->readLine(buffer,512);
				if(count > 0)
					text_console->insertPlainText(buffer);
			}
		}
	}

	//QMessageBox::information(this,"cmdline",cmd+" "+args.join(" "));
	//QMessageBox::information(this,"cwd",cmd.section('/',0,-2));
	setWindowState(wnd);
}

void ui_main::closeEvent(QCloseEvent* event)
{
	if(m_current_project != nullptr)
		CloseProject();
	CloseAll();
    // settings save
    QSettings settings("cpcbuild.ini",QSettings::IniFormat);
	settings.setValue("assembler/include_dir",m_app_settings.includedir());
	settings.setValue("emulator/emu_path",m_app_settings.emu_path());
	settings.setValue("emulator/emu_ospath",m_app_settings.emu_ospath());
	settings.setValue("emulator/model",m_app_settings.emu_model());
	settings.setValue("emulator/exp",m_app_settings.emu_exp());
	event->accept();
}

void ui_main::DockOpenFile(QTreeWidgetItem* widget, int col)
{
	QString filename;
	QString typestr;
	filename = widget->toolTip(col);
	if(widget->parent() == nullptr)
		return;
	if(filename.isEmpty())
		return;
	if(check_for_open_file(filename))
		return;

	ui_QMdiSubWindow* subwin;
	project_file* f = m_current_project->find_file(filename);

	switch(m_current_project->get_filetype(filename))
	{
	case PROJECT_FILE_SOURCE_ASM:
		subwin = CreateWindowAsm();
		subwin->load_text(filename);
		typestr = "Z80 Assembly - ";
		break;
	case PROJECT_FILE_BINARY:
		subwin = CreateWindowBinary();
		subwin->load_binary(filename);
		typestr = "Binary file - ";
		break;
	case PROJECT_FILE_GRAPHICS:
		subwin = CreateWindowGraphics();
		subwin->load_gfx(filename,f->get_width(),f->get_height());
		typestr = "Graphics Editor (Screen) - ";
		break;
	case PROJECT_FILE_ASCII:
	default:
		subwin = CreateWindowASCII();
		subwin->load_text(filename);
		typestr = "ASCII file - ";
		break;
	}

	QStringList split = filename.split(QDir::separator());
	QString shortname = split.last();

	subwin->setWindowTitle(typestr + shortname);
}

void ui_main::EditProperties()
{
	QString filename = tree_files->currentItem()->toolTip(0);
	int ret;
	if(filename.isEmpty())
		return;
	if(!m_current_project->check_for_file(filename))
		return;
    QLineEdit* dlg_load = m_dlg_fileprop->findChild<QLineEdit*>("dlg_load_address");
    QLineEdit* dlg_exec = m_dlg_fileprop->findChild<QLineEdit*>("dlg_exec_address");
    QComboBox* dlg_type = m_dlg_fileprop->findChild<QComboBox*>("dlg_filetype");

	unsigned int load_addr,exec_addr;
	int filetype;
	filetype = m_current_project->get_filetype(filename);
	load_addr = m_current_project->get_load_address(filename);
	exec_addr = m_current_project->get_exec_address(filename);
	dlg_load->clear();
	dlg_exec->clear();
	dlg_type->setCurrentIndex(filetype);
	dlg_load->insert(QString::number(load_addr,16));
	dlg_exec->insert(QString::number(exec_addr,16));
	ret = m_dlg_fileprop->exec();
	if(ret == QDialog::Accepted)
	{
		m_current_project->set_filetype(filename,dlg_type->currentIndex());
		m_current_project->set_load_address(filename,dlg_load->text().toUInt(nullptr,16));
		m_current_project->set_exec_address(filename,dlg_exec->text().toUInt(nullptr,16));
	}
}

void ui_main::SetBootable()
{
	QString filename = tree_files->currentItem()->toolTip(0);

	if(filename.isEmpty())
		return;

	m_current_project->set_bootable_filename(m_current_project->find_file(filename));
}

void ui_main::remove_file(QString filename)
{
	int idx;

	for(idx=0;idx<m_doclist.count();idx++)
	{
		if(m_doclist.at(idx)->get_filename() == filename)
			m_doclist.takeAt(idx);
	}
}

void ui_main::ImportPal()
{
	QString filename;
	QStringList filelist;
	QFileDialog dlg(this);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setNameFilter(tr("Palette files (*.pal);;All Files (*.*)"));
	if(dlg.exec())
	{
		filelist = dlg.selectedFiles();
		filename = filelist.first();
		if(filename.isEmpty())
			return;

		ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
		subwin->import_pal(filename);
	}
}

void ui_main::ImportPalPlus()
{
	QString filename;
	QStringList filelist;
	QFileDialog dlg(this);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setNameFilter(tr("Palette files (*.pal);;All Files (*.*)"));
	if(dlg.exec())
	{
		filelist = dlg.selectedFiles();
		filename = filelist.first();
		if(filename.isEmpty())
			return;

		ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
		subwin->import_pal_12bit(filename);
	}
}

void ui_main::ImportImage()
{
	// Import Qt supported image and convert to CPC data
	QString filename;
	QStringList filelist;

	// create file dialog and add mode selection to it
	QFileDialog dlg(this);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setNameFilter(tr("Image files (*.bmp *.png *.gif *.jpg *.jpeg *.pbm *.pgm *.ppm *.tiff *.tif *.xbm *.xpm *.scr);;All Files (*.*)"));
	QLayout* dlg_layout = dlg.layout();
	QLabel* dlg_label = new QLabel;
	dlg_label->setText(tr("Import graphics as:"));
	dlg_layout->addWidget(dlg_label);
	QComboBox* dlg_combo = new QComboBox;
	dlg_combo->addItem(tr("Mode 0"));
	dlg_combo->addItem(tr("Mode 1"));
	dlg_combo->addItem(tr("Mode 2"));
	dlg_layout->addWidget(dlg_combo);

	// show file dialog
	if(dlg.exec())
	{
		int mode;
		filelist = dlg.selectedFiles();
		filename = filelist.first();
		if(filename.isEmpty())
			return;
		if(check_for_open_file(filename))
			return;

		mode = dlg_combo->currentIndex();
		ui_QMdiSubWindow* subwin = CreateWindowGraphics();
		QStringList split = filename.split(QDir::separator());
		QString shortname = split.last();

		if(subwin->import_image(filename,mode) == false)
		{
			subwin->close();  // close window if import fails.
			return;
		}
		subwin->setWindowTitle("Graphics Editor (Linear) - " + shortname);
	}
}

void ui_main::ImportTileset()
{
	// Import Qt supported image and convert to a tileset
	QString filename;
	QStringList filelist;

	// create file dialog and add mode selection to it
	QFileDialog dlg(this);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setNameFilter(tr("Image files (*.bmp *.png *.gif *.jpg *.jpeg *.pbm *.pgm *.ppm *.tiff *.tif *.xbm *.xpm *.scr);;All Files (*.*)"));
	QLayout* dlg_layout = dlg.layout();
	QLabel* dlg_label = new QLabel;
	dlg_label->setText(tr("Import tileset as:"));
	dlg_layout->addWidget(dlg_label);
	QComboBox* dlg_combo = new QComboBox;
	dlg_combo->addItem(tr("Mode 0"));
	dlg_combo->addItem(tr("Mode 1"));
	dlg_combo->addItem(tr("Mode 2"));
	dlg_layout->addWidget(dlg_combo);

	// create a quick and dirty tile dimension dialog, since it doesn't fit well on the file dialog's layout
	QDialog dlg_size(this);
	QHBoxLayout* dlg_size_layout = new QHBoxLayout;
	QLabel* dlg_label2 = new QLabel;
	dlg_label2->setText(tr("Tile size:"));
	dlg_size_layout->addWidget(dlg_label2);
	QSpinBox* dlg_spin_width = new QSpinBox;
	dlg_spin_width->setMinimum(2);
	dlg_spin_width->setMaximum(64);
	dlg_spin_width->setSingleStep(2);
	dlg_spin_width->setValue(8);
	dlg_size_layout->addWidget(dlg_spin_width);
	QLabel* dlg_label3 = new QLabel;
	dlg_label3->setText(tr("x"));
	dlg_size_layout->addWidget(dlg_label3);
	QSpinBox* dlg_spin_height = new QSpinBox;
	dlg_spin_height->setMinimum(1);
	dlg_spin_height->setMaximum(64);
	dlg_spin_height->setSingleStep(1);
	dlg_spin_height->setValue(8);
	dlg_size_layout->addWidget(dlg_spin_height);
	QDialogButtonBox* dlg_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(dlg_buttons, SIGNAL(accepted()), &dlg_size, SLOT(accept()));
	connect(dlg_buttons, SIGNAL(rejected()), &dlg_size, SLOT(reject()));
	dlg_size_layout->addWidget(dlg_buttons);
	dlg_size.setLayout(dlg_size_layout);

	// show file dialog
	if(dlg.exec())
	{
		int mode;


		filelist = dlg.selectedFiles();
		filename = filelist.first();
		if(filename.isEmpty())
			return;
		if(check_for_open_file(filename))
			return;

		mode = dlg_combo->currentIndex();
		switch(mode)
		{
		default:
		case 3:
		case 0:	dlg_spin_width->setMinimum(2); dlg_spin_width->setSingleStep(2); break;
		case 1:	dlg_spin_width->setMinimum(4); dlg_spin_width->setSingleStep(4); break;
		case 2:	dlg_spin_width->setMinimum(8); dlg_spin_width->setSingleStep(8); break;
		}
		// show tile dimension dialog, stop here if cancelled
		if(!dlg_size.exec())
			return;

		ui_QMdiSubWindow* subwin = CreateWindowTileset();
		QStringList split = filename.split(QDir::separator());
		QString shortname = split.last();

		QMessageBox msg(QMessageBox::Critical,"Unimplemented","Importing a tileset is currently unimplemented",QMessageBox::Ok);
		msg.exec();

		if(subwin->import_tileset(filename,mode,dlg_spin_width->value(),dlg_spin_height->value()) == false)
		{
			subwin->close();  // close window if import fails.
			return;
		}
		subwin->setWindowTitle("Tile editor - " + shortname);
	}
}

void ui_main::TogglePal()
{
	bool plus;
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	plus = subwin->toggle_pal();
	palmenu->setChecked(plus);
}

void ui_main::ExportPalClip()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	subwin->export_pal_to_clipboard();
}

void ui_main::BuildOptions()
{
    QLineEdit* dlg_name = m_dlg_buildoptions->findChild<QLineEdit*>("dlg_build_filename");
	dlg_name->clear();
	dlg_name->insert(m_current_project->get_output_filename());
	QRadioButton* dlg_build_disk = m_dlg_buildoptions->findChild<QRadioButton*>("dlg_build_disk");
	QRadioButton* dlg_build_cart = m_dlg_buildoptions->findChild<QRadioButton*>("dlg_build_cart");

	if(m_current_project->get_build_type() == BUILD_CART)
		dlg_build_cart->setChecked(true);
	else
		dlg_build_disk->setChecked(true);

	QList<project_file*> filelist = m_current_project->get_filelist();
	BlockMapModel block_model(&filelist);
	QTableView* dlg_table = m_dlg_buildoptions->findChild<QTableView*>("dlg_block_map");
	dlg_table->setModel(&block_model);

	if(m_dlg_buildoptions->exec() == QDialog::Accepted)
	{
		m_current_project->set_output_filename(dlg_name->text());
		if(!dlg_build_cart->isChecked())
		{
			for(int x=0;x<filelist.size();x++)
				filelist.at(x)->clear_block();
			m_current_project->set_build_type(BUILD_DISK);
		}
		else
			m_current_project->set_build_type(BUILD_CART);
	}
}

void ui_main::CompileOptions()
{
    QLineEdit* dlg_name = m_dlg_compileoptions->findChild<QLineEdit*>("dlg_compile_options_includedir");
    dlg_name->clear();
	dlg_name->insert(m_app_settings.includedir());
    if(m_dlg_compileoptions->exec() == QDialog::Accepted)
		m_app_settings.set_includedir(dlg_name->text());
}

void ui_main::EmulatorSettings()
{
	QLineEdit* dlg_name = m_dlg_emuoptions->findChild<QLineEdit*>("dlg_emupath");
	QLineEdit* dlg_os = m_dlg_emuoptions->findChild<QLineEdit*>("dlg_opt_ospath");
	dlg_name->clear();
	dlg_name->insert(m_app_settings.emu_path());
	dlg_os->clear();
	dlg_os->insert(m_app_settings.emu_ospath());
	switch(m_app_settings.emu_model())
	{
	case appsettings::EMUMODEL_464:
		m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_464")->setChecked(true);
		break;
	case appsettings::EMUMODEL_664:
		m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_664")->setChecked(true);
		break;
	case appsettings::EMUMODEL_6128:
		m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_6128")->setChecked(true);
		break;
	case appsettings::EMUMODEL_464PLUS:
		m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_464plus")->setChecked(true);
		break;
	case appsettings::EMUMODEL_6128PLUS:
		m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_6128plus")->setChecked(true);
		break;
	}

	if(m_dlg_emuoptions->exec() == QDialog::Accepted)
	{
		// todo: get model/expansion values
		int m=appsettings::EMUMODEL_464,e=appsettings::EMUEXP_NONE;
		if(m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_464")->isChecked())
			m = appsettings::EMUMODEL_464;
		else if(m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_664")->isChecked())
			m = appsettings::EMUMODEL_664;
		else if(m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_6128")->isChecked())
			m = appsettings::EMUMODEL_6128;
		else if(m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_464plus")->isChecked())
			m = appsettings::EMUMODEL_464PLUS;
		else if(m_dlg_emuoptions->findChild<QRadioButton*>("dlg_opt_6128plus")->isChecked())
			m = appsettings::EMUMODEL_6128PLUS;
		m_app_settings.set_emu_path(dlg_name->text());
		m_app_settings.set_emu_ospath(dlg_os->text());
		m_app_settings.set_emu_model(m);
		m_app_settings.set_emu_exp(e);
	}
}

// if file is part of the current project, then rename it.
void ui_main::rename_project_file(QString oldname, QString filename)
{
	if(m_current_project != nullptr)  // check if there is an open project first
	{
		project_file* pf = m_current_project->find_file(oldname);
		if(pf != nullptr)
			pf->set_filename(filename);
		redraw_project_tree();
	}
}

void ui_main::redraw_project_tree()
{
	QList<project_file*>::iterator it;
	QList<project_file*> lst = m_current_project->get_filelist();
	tree_files->clear();
	QTreeWidgetItem* item = new QTreeWidgetItem(QTreeWidgetItem::Type);
	QFont font("Sans",14,QFont::Bold);
	item->setText(0,m_current_project->get_name() + " files");
	item->setToolTip(0,m_project_filename);
	item->setFont(0,font);
	tree_files->addTopLevelItem(item);
	for(it=lst.begin();it!=lst.end();it++)
	{
		QString str = (*it)->get_filename();
		QStringList split = str.split(QDir::separator());
		QString shortname = split.last();
		// update project file list
		QTreeWidgetItem* item = tree_files->topLevelItem(0);
		if(item == nullptr)
			return;  // this shouldn't happen
		QTreeWidgetItem* child = new QTreeWidgetItem(item,QTreeWidgetItem::Type);
		child->setText(0,shortname);
		child->setToolTip(0,str);
		item->addChild(child);
	}
}

/*
 *  subclassed QMdiSubWindow, for document display
 */
ui_QMdiSubWindow::ui_QMdiSubWindow(int doctype, QWidget* parent, Qt::WindowFlags flags) :
		QMdiSubWindow(parent,flags),
		m_parent(parent),
		m_modified(false),
		m_project(nullptr),
		m_doctype(doctype)
{
	setMouseTracking(true);
}

ui_QMdiSubWindow::~ui_QMdiSubWindow()
{
}

bool ui_QMdiSubWindow::load_text(QString filename)
{
	if(m_doctype != PROJECT_FILE_SOURCE_ASM && m_doctype != PROJECT_FILE_ASCII)
		return false;

	QFile f(filename);
	QString data;
	QTextEdit* txt = dynamic_cast<QTextEdit*>(widget());

	set_filename(filename);
	f.open(QFile::ReadOnly);
	QTextStream stream(&f);
	data.clear();
	while(!stream.atEnd())
	{
		data += stream.readLine();
		data += "\n";
	}
	f.close();
	txt->setPlainText(data);
	m_modified = false;

	return true;  // TODO: error testing
}

bool ui_QMdiSubWindow::save_text(QString filename)
{
	if(m_doctype != PROJECT_FILE_SOURCE_ASM && m_doctype != PROJECT_FILE_ASCII)
		return false;

	QFile f(filename);
	QString data;
	QTextEdit* txt = dynamic_cast<QTextEdit*>(widget());

	data = txt->toPlainText();
	f.open(QFile::WriteOnly);
	QTextStream stream(&f);
	stream << data;
	f.close();
	m_modified = false;

	return true;  // TODO: error testing
}

bool ui_QMdiSubWindow::load_binary(QString filename)
{
	if(m_doctype != PROJECT_FILE_BINARY)
		return false;

	QFile f(filename);
	unsigned char* data;
	QScrollArea* area = dynamic_cast<QScrollArea*>(widget());
	bineditor* bin = dynamic_cast<bineditor*>(area->widget());
	int fsize;

	set_filename(filename);
	f.open(QFile::ReadOnly);
	fsize = f.size();
	data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed when the widget is closed, or when set_data is called again
	f.read(reinterpret_cast<char*>(data),fsize);
	f.close();

	bin->set_data(data,fsize);

	m_modified = false;

	return true;  // TODO: error testing
}

bool ui_QMdiSubWindow::load_gfx(QString filename)
{
	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return false;

	QFile f(filename);
	QUiLoader loader;
	QFile uif(":/forms/ide_tilesize.ui");
	QWidget* form;
	unsigned char* data;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());
	int fsize;
	bool has_header;

	uif.open(QFile::ReadOnly);
	form = loader.load(&uif, this);
	uif.close();

	if(form)
	{
		QDialog* dlg = dynamic_cast<QDialog*>(form);
		if(dlg)
		{
			// add option to skip AMSDOS header
			QGridLayout* dlg_layout = dynamic_cast<QGridLayout*>(dlg->layout());
			QCheckBox* dlg_header = new QCheckBox(dlg);
			dlg_header->setText("File has AMSDOS header");
			dlg_header->setChecked(false);
			dlg_layout->addWidget(dlg_header,2,0,1,2);
			// run dialog
			if(dlg->exec() == QDialog::Rejected)
				return false;
            QLineEdit* dlg_width = dlg->findChild<QLineEdit*>("dlg_tile_width");
            QLineEdit* dlg_height = dlg->findChild<QLineEdit*>("dlg_tile_height");
			gfx->set_size(dlg_width->text().toUInt(nullptr,10),dlg_height->text().toUInt(nullptr,10));
			if(dlg_header->isChecked())
				has_header = true;
			else
				has_header = false;
		}
		else
			return false;
	}
	else
		return false;

	m_modified = false;

	set_filename(filename);
	f.open(QFile::ReadOnly);
	if(has_header)
	{
		fsize = f.size() - 0x80;
		data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed when the widget is closed, or when set_data is called again
		f.seek(0x80);  // skip AMSDOS header
		f.read(reinterpret_cast<char*>(data),fsize);
		m_modified = true;  // files are not saved with AMSDOS header, so it is effectively modified.
	}
	else
	{
		fsize = f.size();
		data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed when the widget is closed, or when set_data is called again
		f.read(reinterpret_cast<char*>(data),fsize);
	}
	f.close();

	gfx->set_data(data,fsize);
	gfx->draw_scene();

	return true;  // TODO: error testing
}

bool ui_QMdiSubWindow::load_gfx(QString filename, int width, int height)
{
	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return false;

	QFile f(filename);
	unsigned char* data;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());
	int fsize;

	gfx->set_size(width,height);

	set_filename(filename);
	f.open(QFile::ReadOnly);
	fsize = f.size();
	data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed when the widget is closed, or when set_data is called again
	f.read(reinterpret_cast<char*>(data),fsize);
	f.close();

	gfx->set_data(data,fsize);
	gfx->draw_scene();

	m_modified = false;

	return true;  // TODO: error testing
}

bool ui_QMdiSubWindow::load_tileset(QString filename)
{
	if(m_doctype != PROJECT_FILE_TILESET)
		return false;

	QFile f(filename);
	QUiLoader loader;
	QFile uif(":/forms/ide_tilesize.ui");
	QWidget* form;
	unsigned char* data;

	tileeditor* gfx = dynamic_cast<tileeditor*>(widget());
	int fsize;
	bool has_header;

	uif.open(QFile::ReadOnly);
	form = loader.load(&uif, this);
	uif.close();

	if(form)
	{
		QDialog* dlg = dynamic_cast<QDialog*>(form);
		if(dlg)
		{
			// add option to skip AMSDOS header
			QGridLayout* dlg_layout = dynamic_cast<QGridLayout*>(dlg->layout());
			QCheckBox* dlg_header = new QCheckBox(dlg);
			dlg_header->setText("File has AMSDOS header");
			dlg_header->setChecked(false);
			dlg_layout->addWidget(dlg_header,2,0,1,2);
			// run dialog
			if(dlg->exec() == QDialog::Rejected)
				return false;
            QLineEdit* dlg_width = dlg->findChild<QLineEdit*>("dlg_tile_width");
            QLineEdit* dlg_height = dlg->findChild<QLineEdit*>("dlg_tile_height");
			gfx->set_size(dlg_width->text().toUInt(nullptr,10),dlg_height->text().toUInt(nullptr,10));
			if(dlg_header->isChecked())
				has_header = true;
			else
				has_header = false;
		}
		else
			return false;
	}
	else
		return false;

	m_modified = false;

	set_filename(filename);
	f.open(QFile::ReadOnly);
	if(has_header)
	{
		fsize = f.size() - 0x80;
		data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed when the widget is closed, or when set_data is called again
		f.seek(0x80);  // skip AMSDOS header
		f.read(reinterpret_cast<char*>(data),fsize);
		m_modified = true;  // files are not saved with AMSDOS header, so it is effectively modified.
	}
	else
	{
		fsize = f.size();
		data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed when the widget is closed, or when set_data is called again
		f.read(reinterpret_cast<char*>(data),fsize);
	}
	f.close();

	gfx->set_data(data,fsize);
	gfx->draw_scene();

	return true;  // TODO: error testing
}

bool ui_QMdiSubWindow::load_tileset(QString filename, int width, int height)
{
	if(m_doctype != PROJECT_FILE_TILESET)
		return false;

	QFile f(filename);
	unsigned char* data;

	tileeditor* gfx = dynamic_cast<tileeditor*>(widget());
	int fsize;

	gfx->set_size(width,height);
	set_filename(filename);
	f.open(QFile::ReadOnly);
	fsize = f.size();
	data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed when the widget is closed, or when set_data is called again
	f.read(reinterpret_cast<char*>(data),fsize);
	f.close();

	gfx->set_data(data,fsize);
	gfx->calculate_tiles();
	gfx->draw_scene();

	m_modified = false;

	return true;  // TODO: error testing
}

bool ui_QMdiSubWindow::save_gfx(QString filename)
{
	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return false;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());
	uint size = gfx->get_datasize();

	QFile f(filename);
	f.open(QFile::WriteOnly);
	QDataStream out(&f);
	out.writeRawData(reinterpret_cast<char*>(gfx->get_data()),size);
	f.close();

	return true;
}

bool ui_QMdiSubWindow::save_tileset(QString filename)
{
	if(m_doctype != PROJECT_FILE_TILESET)
		return false;

	tileeditor* gfx = dynamic_cast<tileeditor*>(widget());
	uint size = gfx->get_datasize();

	QFile f(filename);
	f.open(QFile::WriteOnly);
	QDataStream out(&f);
	out.writeRawData(reinterpret_cast<char*>(gfx->get_data()),size);
	f.close();

	return true;
}

bool ui_QMdiSubWindow::import_pal(QString filename)
{
	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return false;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());
	unsigned char* data;
	int fsize;

	QFile f(filename);
	f.open(QFile::ReadOnly);
	fsize = f.size();
	data = reinterpret_cast<unsigned char*>(malloc(fsize));  // will be freed once the data is copied
	f.read(reinterpret_cast<char*>(data),fsize);
	f.close();

	gfx->load_pal_normal((data+0x88));  // skip file header and AMSDOS header

	free(data);
	repaint();
	return true;
}

bool ui_QMdiSubWindow::import_pal_12bit(QString filename)
{
	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return false;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());
	unsigned short* data;
	int fsize;

	QFile f(filename);
	f.open(QFile::ReadOnly);
	fsize = f.size();
	data = reinterpret_cast<unsigned short*>(malloc(fsize));  // will be freed once the data is copied
	f.read(reinterpret_cast<char*>(data),fsize);
	f.close();

	gfx->load_pal_12bit((data+0x44));  // skip file header and AMSDOS header

	free(data);
	repaint();
	return true;
}

bool ui_QMdiSubWindow::import_scr(QString filename)
{
	int x,width, height;
	int inptr,outptr;
	bool has_header;

	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return false;

	QUiLoader loader;
	QFile uif(":/forms/ide_tilesize.ui");
	QWidget* form;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());
	unsigned char* data;
	unsigned char* output;
//	int fsize;

	uif.open(QFile::ReadOnly);
	form = loader.load(&uif, this);
	uif.close();

	if(form)
	{
		QDialog* dlg = dynamic_cast<QDialog*>(form);
		if(dlg)
		{
			// add option to skip AMSDOS header
            QLineEdit* dlg_width = dlg->findChild<QLineEdit*>("dlg_tile_width");
            QLineEdit* dlg_height = dlg->findChild<QLineEdit*>("dlg_tile_height");
			QGridLayout* dlg_layout = dynamic_cast<QGridLayout*>(dlg->layout());
			QCheckBox* dlg_header = new QCheckBox(dlg);
			dlg_header->setText("File has AMSDOS header");
			dlg_header->setChecked(false);
			dlg_layout->addWidget(dlg_header,2,0,1,2);
			dlg_width->setText("80");
			dlg_height->setText("200");
			dlg_width->setEnabled(false);
			dlg_height->setEnabled(false);
			// run dialog
			if(dlg->exec() == QDialog::Rejected)
				return false;
			gfx->set_size(dlg_width->text().toUInt(nullptr,10),dlg_height->text().toUInt(nullptr,10));
			if(dlg_header->isChecked())
				has_header = true;
			else
				has_header = false;
		}
		else
			return false;
	}
	else
		return false;

	QFile f(filename);
	f.open(QFile::ReadOnly);
//	fsize = f.size();
	data = reinterpret_cast<unsigned char*>(malloc(16384));
	output = reinterpret_cast<unsigned char*>(malloc(16384));  // will be freed when the widget is closed, or when set_data is called again
	m_modified = false;
	if(has_header)
	{
		f.seek(0x80);  // skip AMSDOS header
		m_modified = true;
	}
//	fsize = f.read(reinterpret_cast<char*>(data),16384);  // limit to 16kB
	f.close();

	width = gfx->get_width();
	height = gfx->get_height();

	inptr = 0;
	outptr = 0;
	for(x=0;x<height;x+=8)  // for each character line
	{
		memcpy(output+outptr,data+inptr,width);
		outptr+=width;
		memcpy(output+outptr,data+inptr+0x800,width);
		outptr+=width;
		memcpy(output+outptr,data+inptr+0x1000,width);
		outptr+=width;
		memcpy(output+outptr,data+inptr+0x1800,width);
		outptr+=width;
		memcpy(output+outptr,data+inptr+0x2000,width);
		outptr+=width;
		memcpy(output+outptr,data+inptr+0x2800,width);
		outptr+=width;
		memcpy(output+outptr,data+inptr+0x3000,width);
		outptr+=width;
		memcpy(output+outptr,data+inptr+0x3800,width);
		outptr+=width;
		inptr+=width;
	}

	gfx->set_data(output,16384);

	free(data);  // no longer needed
	repaint();

	m_filename = filename;

	return true;
}

bool ui_QMdiSubWindow::import_image(QString filename, int mode)
{
	imageconvert conv(filename,mode);
	unsigned char* buffer;
	unsigned int size;
	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());

	// TODO: add options

	size = conv.calculate_size(mode);
	buffer = reinterpret_cast<unsigned char*>(malloc(size));
	memset(buffer,0,size);
	if(!conv.convert(buffer,size,mode))
	{
		// failed
		free(buffer);
		return false;
	}
	else
	{
		// success
		gfx->set_size(80,200);//conv.get_width(),conv.get_height());
		gfx->set_format_screen();
		gfx->set_data(buffer,size);
		for(int x=0;x<16;x++)
			gfx->set_pen(x,conv.get_colour(x));
		m_filename = filename;
		return true;
	}

	return false;
}

bool ui_QMdiSubWindow::import_tileset(QString filename, int mode, int width, int height)
{
	tileconvert conv(filename,mode);
	unsigned char* buffer;
	unsigned int size;
	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());

	// TODO: add options

	size = conv.calculate_size(mode,width,height); // TODO
	buffer = reinterpret_cast<unsigned char*>(malloc(size));
	memset(buffer,0,size);
	if(!conv.convert(buffer,size,mode,width,height)) // TODO
	{
		// failed
		free(buffer);
		return false;
	}
	else
	{
		// success
		gfx->set_size(conv.get_tile_width(),conv.get_tile_height());
		gfx->set_format_linear();
		gfx->set_data(buffer,size);
		for(int x=0;x<16;x++)
			gfx->set_pen(x,conv.get_colour(x));
		m_filename = filename;
		return true;
	}

	return false;
}

bool ui_QMdiSubWindow::toggle_pal()
{
	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return false;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());

	return gfx->toggle_pal();
}

void ui_QMdiSubWindow::export_pal_to_clipboard()
{
	if(m_doctype != PROJECT_FILE_GRAPHICS)
		return;

	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());

	gfx->export_palette_to_clipboard();
}

void ui_QMdiSubWindow::contents_changed()
{
	m_modified = true;
}

void ui_QMdiSubWindow::cursor_changed()
{
	if(m_doctype != PROJECT_FILE_SOURCE_ASM && m_doctype != PROJECT_FILE_ASCII)
		return;
	ui_main* p = dynamic_cast<ui_main*>(this->parent());
	QTextEdit* widget = dynamic_cast<QTextEdit*>(this->widget());
	p->set_status(QString("Line %1").arg(widget->textCursor().blockNumber() + 1));
}

bool ui_QMdiSubWindow::event(QEvent *event)
{
	if(event->type() == QEvent::ToolTip)
	{
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
		ui_main* p = dynamic_cast<ui_main*>(this->parent());
		QTextEdit* widget = dynamic_cast<QTextEdit*>(this->widget());
		if(widget == nullptr)
		{
			QToolTip::hideText();
			return QMdiSubWindow::event(event);
		}
		QTextCursor cursor = widget->cursorForPosition(helpEvent->pos());
		cursor.select(QTextCursor::WordUnderCursor);
		if (!cursor.selectedText().isEmpty())
		{
			try
			{
				std::string text = p->fw_desc().at(cursor.selectedText().toStdString());
				QToolTip::showText(helpEvent->globalPos(),QString(text.c_str()),widget);
			}
			catch (const std::out_of_range&)
			{
				QToolTip::hideText();
			}
		}
	}
	else
		QToolTip::hideText();
	return QMdiSubWindow::event(event);
}

void ui_QMdiSubWindow::closeEvent(QCloseEvent* event)
{
	ui_main* main = dynamic_cast<ui_main*>(parent());

	if(is_modified() == true)
	{
		QMessageBox msg;
		int ret;
		msg.setWindowTitle(m_filename);
		msg.setText(tr("Do you wish to save the changes?"));
		msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		msg.setDefaultButton(QMessageBox::Save);
		msg.setIcon(QMessageBox::Question);
		ret = msg.exec();
		switch(ret)
		{
		case QMessageBox::Cancel:
			event->ignore();
			break;  // Do nothing, stop here, window will remain open
		case QMessageBox::Discard:
			main->remove_file(m_filename);
			event->accept();
			break;  // Do nothing, continue to close window
		case QMessageBox::Save:
			save_text(m_filename);  // Save file, then continue to close window
			main->remove_file(m_filename);
			event->accept();
			break;
		}
	}
	else
	{
		main->remove_file(m_filename);
		event->accept();
	}
}

// subclassed syntax highlighter object
highlighter::highlighter(QTextDocument* parent) :
	QSyntaxHighlighter(parent)
{
	syntax_rule rule;

	fmt_opcode.setFontWeight(QFont::Bold);

	QStringList opcode_list;
	QStringList register_list;
	QStringList keyword_list;

	// todo: add more opcodes
	opcode_list << "\\badc\\b" << "\\badd\\b" << "\\band\\b" << "\\bbit\\b" << "\\bcall\\b" << "\\bccf\\b"
			<< "\\bcp\\b" << "\\bcpd\\b"<< "\\bcpdr\\b"<< "\\bcpi\\b"<< "\\bcpir\\b"<< "\\bcpl\\b" << "\\bdaa\\b"
			<< "\\bdec\\b"<< "\\bdi\\b" << "\\bdjnz\\b" << "\\bei\\b" << "\\bex\\b" << "\\bexx\\b" << "\\bhalt\\b"
			<< "\\bim\\b" << "\\bin\\b" << "\\binc\\b" << "\\bind\\b" << "\\bindr\\b" << "\\bini\\b" << "\\binir\\b"
			<< "\\bjp\\b" << "\\bjr\\b" << "\\bld\\b" << "\\bldd\\b" << "\\blddr\\b" << "\\bldi\\b" << "\\bldir\\b"
			<< "\\bneg\\b" << "\\bnop\\b" << "\\bor\\b" << "\\botdr\\b" << "\\botir\\b" << "\\bout\\b" << "\\boutd\\b"
			<< "\\bouti\\b" << "\\bpop\\b" << "\\bpush\\b" << "\\bres\\b" << "\\bret\\b" << "\\breti\\b" << "\\bretn\\b"
			<< "\\brl\\b" << "\\brla\\b" << "\\brlc\\b" << "\\brld\\b" << "\\brr\\b" << "\\brra\\b" << "\\brrc\\b"
			<< "\\brrd\\b" << "\\brst\\b" << "\\bsbc\\b" << "\\bscf\\b" << "\\bset\\b" << "\\bsla\\b" << "\\bsra\\b"
			<< "\\bsra\\b" << "\\bsrl\\b" << "\\bsub\\b" << "\\bxor\\b";

	foreach(const QString& pattern, opcode_list)
	{
		rule.pattern = QRegExp(pattern,Qt::CaseInsensitive);
		rule.format = fmt_opcode;
		rules.append(rule);
	}

	fmt_keywords.setForeground(Qt::magenta);
	fmt_keywords.setFontWeight(QFont::Bold);

	keyword_list << "\\b.error\\b" << "\\b.shift\\b" << "\\b.warning\\b" << "\\bdefb\\b" << "\\bdb\\b" << "\\bdefl\\b"
			<< "\\bdefm\\b" << "\\bdefs\\b" << "\\bdefw\\b" << "\\bds\\b" << "\\bdw\\b" << "\\belse\\b" << "\\bend\\b"
			<< "\\bendif\\b" << "\\bendm\\b" << "\\bendp\\b" << "\\bequ\\b" << "\\bexitm\\b" << "\\bif\\b"
			<< "\\binclude\\b" << "\\bincbin\\b" << "\\birp\\b" << "\\blocal\\b" << "\\bmacro\\b" << "\\borg\\b"
			<< "\\bproc\\b" << "\\bpublic\\b" << "\\brept\\b";

	foreach(const QString& pattern, keyword_list)
	{
		rule.pattern = QRegExp(pattern,Qt::CaseInsensitive);
		rule.format = fmt_keywords;
		rules.append(rule);
	}

	fmt_register.setForeground(Qt::red);

	register_list << "\\ba\\b" << "\\bb\\b" << "\\bc\\b" << "\\bd\\b" << "\\be\\b" << "\\bh\\b" << "\\bl\\b"
			 << "\\baf\\b" << "\\bbc\\b" << "\\bde\\b" << "\\bhl\\b" << "\\bix\\b" << "\\biy\\b"
			 << "\\baf'\\b" << "\\bbc'\\b" << "\\bde'\\b" << "\\bhl'\\b";

	foreach(const QString& pattern, register_list)
	{
		rule.pattern = QRegExp(pattern,Qt::CaseInsensitive);
		rule.format = fmt_register;
		rules.append(rule);
	}

	fmt_singlecomment.setFontItalic(true);
	fmt_singlecomment.setForeground(Qt::green);

	rule.pattern = QRegExp(";[^\n]*");
	rule.format = fmt_singlecomment;
	rules.append(rule);


}

void highlighter::highlightBlock(const QString& text)
{
    foreach (const syntax_rule &rule, rules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

}
