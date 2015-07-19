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

ui_main::ui_main(QWidget* parent)
	: QMainWindow(parent),
	  m_current_project(NULL)
{
	QWidget* form;
	setupUi(this);
	setWindowTitle("CPC Builder");

	// pre-load some dialogs
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
}

ui_main::~ui_main()
{
}

void ui_main::NewProject()
{
	int ret = QDialog::Rejected;

	QString name, fname;
	fname = QFileDialog::getOpenFileName(this,tr("New Project..."),"~/",tr("Project files (*.cpc);;All files (*.*)"));
	ret = m_dlg_newproject->exec();

	if(ret == QDialog::Accepted && !fname.isEmpty())
	{
		QLineEdit* dlg_name = qFindChild<QLineEdit*>(this,"dlg_project_name");

		name = dlg_name->text();
		// new project
		m_current_project = new project(name,fname);
		if(m_current_project == NULL)
			return;
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
	QDir oldpath(QDir::current());

	if(m_project_filename.isEmpty())
		return;

	if(m_current_project != NULL)
		CloseProject();

	QFile f(m_project_filename);
	{
		QMessageBox msg(QMessageBox::Warning,"Test","Filename: " + m_project_filename + " Oldpath: "+oldpath.path()+" Projectpath: "+fpath,QMessageBox::Ok);
		msg.exec();
	}
	f.open(QFile::ReadOnly);
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
	}

	m_current_project = new project(name,m_project_filename);
	filelist.clear();
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
			{
				QMessageBox msg(QMessageBox::Warning,"Test","Filename: " + fname,QMessageBox::Ok);
				msg.exec();
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
							pfile->set_load_address(attr.value("load").toString().toUInt(0,16));
						if(attr.hasAttribute("exec"))
							pfile->set_exec_address(attr.value("exec").toString().toUInt(0,16));
						pfile->set_filename(attr.value("name").toString());
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
							pfile->set_load_address(attr.value("load").toString().toUInt(0,16));
						if(attr.hasAttribute("exec"))
							pfile->set_exec_address(attr.value("exec").toString().toUInt(0,16));
						pfile->set_filename(attr.value("name").toString());
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
						pfile->set_load_address(attr.value("load").toString().toUInt(0,16));
					if(attr.hasAttribute("exec"))
						pfile->set_exec_address(attr.value("exec").toString().toUInt(0,16));
					pfile->set_filename(attr.value("name").toString());
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
							pfile->set_load_address(attr.value("load").toString().toUInt(0,16));
						if(attr.hasAttribute("exec"))
							pfile->set_exec_address(attr.value("exec").toString().toUInt(0,16));
						pfile->set_filename(attr.value("name").toString());
						m_current_project->add_file(pfile);
					}
				}
			}
		}
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
			if(item == NULL)
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
	QDir::setCurrent(oldpath.path());
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
	if(m_current_project == NULL)
		return;
	SaveProject();
	tree_files->clear();
	delete(m_current_project);
	m_current_project = NULL;
	menu_project_close->setEnabled(false);
	menu_project_build->setEnabled(false);
	menu_project_buildoptions->setEnabled(false);
}

void ui_main::AddToProject()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(m_current_project == NULL)
		return;
	if(subwin == NULL)
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
		if(gfx != NULL)
			m_current_project->add_gfx_file(fname,gfx->get_width(),gfx->get_height());
	}
	else
		m_current_project->add_file(fname);
	// update project file list
	QTreeWidgetItem* item = tree_files->topLevelItem(0);
	if(item == NULL)
		return;  // this shouldn't happen
	QTreeWidgetItem* child = new QTreeWidgetItem(item,QTreeWidgetItem::Type);
	child->setText(0,shortname);
	child->setToolTip(0,fname);
	item->addChild(child);
	SaveProject();
}

void ui_main::BuildProject()
{
	if(m_current_project != NULL)
	{
		QString fpath(m_project_filename.section(QDir::separator(),0,-2));
		QDir oldpath(QDir::current());

		QDir::setCurrent(fpath);
		m_current_project->build(text_console);
		QDir::setCurrent(oldpath.path());
	}
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
	return NULL;
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
	connect(widget,SIGNAL(textChanged()),subwin,SLOT(contents_changed()));
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
	data = (unsigned char*)malloc(16384);  // screen size is 16kB (not all is visible)
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
			QLineEdit* dlg_width = qFindChild<QLineEdit*>(dlg,"dlg_tile_width");
			QLineEdit* dlg_height = qFindChild<QLineEdit*>(dlg,"dlg_tile_height");
			w = dlg_width->text().toUInt(0,10);
			h = dlg_height->text().toUInt(0,10);
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
	data = (unsigned char*)malloc(w*h);
	memset(data,0,w*h);
	widget->set_data(data,w*h);
	widget->draw_scene();
}

void ui_main::OpenFile()
{
	QString filename;
	QStringList filelist;
	QFileDialog dlg(this);
	QLayout* dlg_layout = dlg.layout();
	QLabel* dlg_label = new QLabel;
	dlg_label->setText(tr("Open As:"));
	dlg_layout->addWidget(dlg_label);
	QComboBox* dlg_combo = new QComboBox;
	dlg_combo->addItem(tr("ASM source"));
	dlg_combo->addItem(tr("Binary data"));
	dlg_combo->addItem(tr("Graphics"));
	dlg_combo->addItem(tr("ASCII"));
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
	}
}

void ui_main::SaveFile()
{
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(subwin == NULL)
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
	QStringList split = filename.split(QDir::separator());
	subwin->setWindowTitle(typestr + split.last());
}

void ui_main::SaveFileAs()
{
	QString filename;
	ui_QMdiSubWindow* subwin = dynamic_cast<ui_QMdiSubWindow*>(mdi_main->currentSubWindow());
	if(subwin == NULL)
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
	if(subwin == NULL)
		return;
	subwin->close();
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
		if(win != NULL)
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

void ui_main::closeEvent(QCloseEvent* event)
{
	CloseAll();
	event->accept();
}

void ui_main::DockOpenFile(QTreeWidgetItem* widget, int col)
{
	QString filename;
	QString typestr;
	filename = widget->toolTip(col);
	if(widget->parent() == NULL)
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
	QLineEdit* dlg_load = qFindChild<QLineEdit*>(this,"dlg_load_address");
	QLineEdit* dlg_exec = qFindChild<QLineEdit*>(this,"dlg_exec_address");
	QComboBox* dlg_type = qFindChild<QComboBox*>(this,"dlg_filetype");

	unsigned int load_addr,exec_addr,filetype;
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
		m_current_project->set_load_address(filename,dlg_load->text().toUInt(0,16));
		m_current_project->set_exec_address(filename,dlg_exec->text().toUInt(0,16));
	}
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
	QFileDialog dlg(this);
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
	QLineEdit* dlg_name = qFindChild<QLineEdit*>(this,"dlg_build_filename");
	dlg_name->clear();
	dlg_name->insert(m_current_project->get_output_filename());
	if(m_dlg_buildoptions->exec() == QDialog::Accepted)
		m_current_project->set_output_filename(dlg_name->text());
}

// if file is part of the current project, then rename it.
void ui_main::rename_project_file(QString oldname, QString filename)
{
	if(m_current_project != NULL)  // check if there is an open project first
	{
		project_file* pf = m_current_project->find_file(oldname);
		if(pf != NULL)
			pf->set_filename(filename);
		redraw_project_tree();
	}
}

void ui_main::redraw_project_tree()
{
	QList<project_file*>::iterator it;
	QList<project_file*> lst = m_current_project->get_filelist();
	QString fpath(m_project_filename.section(QDir::separator(),0,-2));
	QDir oldpath(QDir::current());
	tree_files->clear();
	QTreeWidgetItem* item = new QTreeWidgetItem(QTreeWidgetItem::Type);
	QFont font("Sans",14,QFont::Bold);
	item->setText(0,m_current_project->get_name() + " files");
	item->setToolTip(0,m_project_filename);
	item->setFont(0,font);
	tree_files->addTopLevelItem(item);
	QDir::setCurrent(fpath);
	for(it=lst.begin();it!=lst.end();it++)
	{
		QString str = (*it)->get_filename();
		QStringList split = str.split(QDir::separator());
		QString shortname = split.last();
		// update project file list
		QTreeWidgetItem* item = tree_files->topLevelItem(0);
		if(item == NULL)
			return;  // this shouldn't happen
		QTreeWidgetItem* child = new QTreeWidgetItem(item,QTreeWidgetItem::Type);
		child->setText(0,shortname);
		child->setToolTip(0,str);
		item->addChild(child);
	}
	QDir::setCurrent(oldpath.path());
}


/*
 *  subclassed QMdiSubWindow, for document display
 */
ui_QMdiSubWindow::ui_QMdiSubWindow(int doctype, QWidget* parent, Qt::WindowFlags flags) :
		QMdiSubWindow(parent,flags),
		m_parent(parent),
		m_modified(false),
		m_project(NULL),
		m_doctype(doctype)
{
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
	data = (unsigned char*)malloc(fsize);  // will be freed when the widget is closed, or when set_data is called again
	f.read((char*)data,fsize);
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
			QLineEdit* dlg_width = qFindChild<QLineEdit*>(this,"dlg_tile_width");
			QLineEdit* dlg_height = qFindChild<QLineEdit*>(this,"dlg_tile_height");
			gfx->set_size(dlg_width->text().toUInt(0,10),dlg_height->text().toUInt(0,10));
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
		data = (unsigned char*)malloc(fsize);  // will be freed when the widget is closed, or when set_data is called again
		f.seek(0x80);  // skip AMSDOS header
		f.read((char*)data,fsize);
		m_modified = true;  // files are not saved with AMSDOS header, so it is effectively modified.
	}
	else
	{
		fsize = f.size();
		data = (unsigned char*)malloc(fsize);  // will be freed when the widget is closed, or when set_data is called again
		f.read((char*)data,fsize);
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
	data = (unsigned char*)malloc(fsize);  // will be freed when the widget is closed, or when set_data is called again
	f.read((char*)data,fsize);
	f.close();

	gfx->set_data(data,fsize);
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
	out.writeRawData((char*)gfx->get_data(),size);
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
	data = (unsigned char*)malloc(fsize);  // will be freed once the data is copied
	f.read((char*)data,fsize);
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
	data = (unsigned short*)malloc(fsize);  // will be freed once the data is copied
	f.read((char*)data,fsize);
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
	int fsize;

	uif.open(QFile::ReadOnly);
	form = loader.load(&uif, this);
	uif.close();

	if(form)
	{
		QDialog* dlg = dynamic_cast<QDialog*>(form);
		if(dlg)
		{
			// add option to skip AMSDOS header
			QLineEdit* dlg_width = qFindChild<QLineEdit*>(this,"dlg_tile_width");
			QLineEdit* dlg_height = qFindChild<QLineEdit*>(this,"dlg_tile_height");
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
			gfx->set_size(dlg_width->text().toUInt(0,10),dlg_height->text().toUInt(0,10));
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
	fsize = f.size();
	data = (unsigned char*)malloc(16384);
	output = (unsigned char*)malloc(16384);  // will be freed when the widget is closed, or when set_data is called again
	m_modified = false;
	if(has_header)
	{
		f.seek(0x80);  // skip AMSDOS header
		m_modified = true;
	}
	fsize = f.read((char*)data,16384);  // limit to 16kB
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
	imageconvert conv(filename);
	unsigned char* buffer;
	int size;
	gfxeditor* gfx = dynamic_cast<gfxeditor*>(widget());

	// TODO: add options

	size = conv.calculate_size(mode);
	buffer = (unsigned char*)malloc(size);  // should be a decent enough size
	memset(buffer,0,size);
	if(!conv.convert(buffer,size,mode))
	{
		// failed
		delete(buffer);
		return false;
	}
	else
	{
		// success
		gfx->set_size(80,200);//conv.get_width(),conv.get_height());
		gfx->set_format_screen();
		gfx->set_data(buffer,size);
		for(int x=0;x<4;x++)
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
