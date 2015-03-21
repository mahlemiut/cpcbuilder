/*
 * ui.h
 *
 *  Created on: 10/08/2011
 *      Author: bsr
 */

#ifndef UI_H_
#define UI_H_

#include <QtGui>
#include <QtUiTools/QUiLoader>
#include <list>
#include "ui_ide_main.h"
#include "main.h"
#include "project.h"
using namespace std;

struct app_settings
{
	QString path_to_mess;  // Full path to MESS
	bool use_path_to_mess;  // false to assume MESS is in the system path
	QString path_to_pasmo;  // Full path to Pasmo
	bool use_path_to_pasmo;  // false to assume Pasmo is in the system path
};

class ui_main;
class ui_QMdiSubWindow;

class highlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	highlighter(QTextDocument* parent = 0);
protected:
	virtual void highlightBlock(const QString& text);
private:
	struct syntax_rule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};
	QVector<syntax_rule> rules;

	QTextCharFormat fmt_opcode;
	QTextCharFormat fmt_keywords;
	QTextCharFormat fmt_singlecomment;
	QTextCharFormat fmt_register;
};

class ui_main : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	ui_main(QWidget* parent = 0);
	~ui_main();
	void SaveProject();  // not a slot, since the project will be always auto-saved
	ui_QMdiSubWindow* CreateWindow(int doctype);
	ui_QMdiSubWindow* CreateWindowAsm();
	ui_QMdiSubWindow* CreateWindowBinary();
	ui_QMdiSubWindow* CreateWindowGraphics();
	ui_QMdiSubWindow* CreateWindowASCII();
	bool check_for_open_file(QString filename);
	void remove_file(QString filename);

public slots:
	void NewProject();
	void OpenProject();
	void CloseProject();
	void AddToProject();
	void BuildProject();
	void NewFile();
	void NewGfxFile();
	void OpenFile();
	void SaveFile();
	void SaveFileAs();
	void CloseFile();
	void About();
	void Exit();
	void CloseAll();
	void DockOpenFile(QTreeWidgetItem* widget, int col);
	void EditProperties();
	void ImportPal();
	void ImportPalPlus();
	void ImportScr();
	void ImportImage();
	void TogglePal();
	void ExportPalClip();
	void BuildOptions();
protected:
	void closeEvent(QCloseEvent* event);
private:
	QString m_project_filename;
	QFont m_source_font;
	QList<ui_QMdiSubWindow*> m_doclist;
	project* m_current_project;  // currently loaded project
	QDialog* m_dlg_newproject;
	QDialog* m_dlg_fileprop;
	QDialog* m_dlg_buildoptions;
	QUiLoader loader;
	highlighter* syntax;
	QAction* palmenu;
};

class ui_QMdiSubWindow : public QMdiSubWindow
{
	Q_OBJECT

public:
	ui_QMdiSubWindow(int doctype = PROJECT_FILE_SOURCE_ASM, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~ui_QMdiSubWindow();
	bool load_text(QString filename);
	bool save_text(QString filename);
	bool load_binary(QString filename);
	bool load_gfx(QString filename);
	bool load_gfx(QString filename, int width, int height);
	bool save_gfx(QString filename);
	bool import_pal(QString filename);
	bool import_pal_12bit(QString filename);
	bool import_scr(QString filename);
	bool import_image(QString filename, int mode);
	bool toggle_pal();
	void export_pal_to_clipboard();  // store palette in clipboard as a DEFB statement
	int get_doctype() { return m_doctype; }
	QString get_filename() { return m_filename; }
	void set_filename(QString filename) { m_filename = filename; }
	bool is_modified() { return m_modified; }
	QWidget* parent() { return m_parent; }
public slots:
	void contents_changed();
protected:
	void closeEvent(QCloseEvent* event);
private:
	QWidget* m_parent;
	QString m_filename;
	bool m_modified;
	project* m_project;  // the project that the file is a part of, if any
	int m_doctype;  // type of document to be displayed
};

#endif /* UI_H_ */