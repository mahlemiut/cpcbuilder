/*
 * ui.h
 *
 *  Created on: 10/08/2011
 *      Author: bsr
 */

#ifndef UI_H_
#define UI_H_

#include <QtWidgets>
#include <QtUiTools/QUiLoader>
#include <list>
#include <unordered_map>
#include "ui_ide_main.h"
#include "main.h"
#include "project.h"
#include "appsettings.h"
using namespace std;

class ui_main;
class ui_QMdiSubWindow;

typedef std::unordered_map<std::string,std::string> desc_map;

class highlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	highlighter(QTextDocument* parent = nullptr);
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
	ui_main(QWidget* parent = nullptr);
	~ui_main();
	void SaveProject();  // not a slot, since the project will be always auto-saved
	ui_QMdiSubWindow* CreateWindow(int doctype);
	ui_QMdiSubWindow* CreateWindowAsm();
	ui_QMdiSubWindow* CreateWindowBinary();
	ui_QMdiSubWindow* CreateWindowGraphics();
	ui_QMdiSubWindow* CreateWindowTileset();
	ui_QMdiSubWindow* CreateWindowASCII();
	bool check_for_open_file(QString filename);
	void remove_file(QString filename);
	void rename_project_file(QString oldname, QString filename);
	void redraw_project_tree();
	const appsettings& settings() { return m_app_settings; }
	const desc_map& fw_desc() { return m_firmware_desc; }
	void set_status(const QString &str) { statusbar->showMessage(str); }
public slots:
	void NewProject();
	void OpenProject();
	void CloseProject();
	void AddToProject();
	void BuildProject();
	void NewFile();
	void NewGfxFile();
	void NewTilesetFile();
	void OpenFile();
	void SaveFile();
	void SaveFileAs();
	void CloseFile();
	void About();
	void Exit();
	void CloseAll();
	void TestEmu();
	void DockOpenFile(QTreeWidgetItem* widget, int col);
	void EditProperties();
	void SetBootable();
	void ImportPal();
	void ImportPalPlus();
	void ImportScr();
	void ImportImage();
	void ImportTileset();
	void TogglePal();
	void ExportPalClip();
	void BuildOptions();
    void CompileOptions();
	void EmulatorSettings();
	void Find();
	void FindNext();
protected:
	void closeEvent(QCloseEvent* event);
private:
	void load_descriptions();
	void read_descriptions_from_file(QFile& qf);
	QString m_project_filename;
    QString m_includedir;
	QFont m_source_font;
	QList<ui_QMdiSubWindow*> m_doclist;
	project* m_current_project;  // currently loaded project
	QDialog* m_dlg_newproject;
	QDialog* m_dlg_fileprop;
	QDialog* m_dlg_buildoptions;
    QDialog* m_dlg_compileoptions;
	QDialog* m_dlg_emuoptions;
    QUiLoader loader;
	highlighter* syntax;
	QAction* palmenu;
	QDir m_oldpath;
	appsettings m_app_settings;
	desc_map m_firmware_desc;
	QString m_searchstring;
};

class ui_QMdiSubWindow : public QMdiSubWindow
{
	Q_OBJECT

public:
	ui_QMdiSubWindow(int doctype = PROJECT_FILE_SOURCE_ASM, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::SubWindow);
	~ui_QMdiSubWindow();
	bool load_text(QString filename);
	bool save_text(QString filename);
	bool load_binary(QString filename);
	bool load_gfx(QString filename);
	bool load_gfx(QString filename, int width, int height);
	bool load_tileset(QString filename);
	bool load_tileset(QString filename, int width, int height);
	bool save_gfx(QString filename);
	bool save_tileset(QString filename);
	bool import_pal(QString filename);
	bool import_pal_12bit(QString filename);
	bool import_scr(QString filename);
	bool import_image(QString filename, int mode);
	bool import_tileset(QString filename, int mode, int width, int height);
	bool toggle_pal();
	void export_pal_to_clipboard();  // store palette in clipboard as a DEFB statement
	int get_doctype() { return m_doctype; }
	QString get_filename() { return m_filename; }
	void set_filename(QString filename) { m_filename = filename; }
	bool is_modified() { return m_modified; }
	void update_title();
	QWidget* parent() { return m_parent; }
public slots:
	void contents_changed();
	void cursor_changed();
protected:
	bool event(QEvent *event) override;
	void closeEvent(QCloseEvent* event) override;
private:
	QWidget* m_parent;
	QString m_filename;
	bool m_modified;
	project* m_project;  // the project that the file is a part of, if any
	int m_doctype;  // type of document to be displayed
};

class AsmEditor : public QTextEdit
{
	Q_OBJECT

public:
	AsmEditor(QWidget* parent = nullptr);
	void lineNumberPaint(QPaintEvent* ev);
	int lineNumberWidth();
public slots:
	void resizeEvent(QResizeEvent* ev) override;
	void updateWidth();
	void updateLineWidth();
	void toggleLineNum(bool ch);
private:
	int getFirstBlock();
	QWidget* lineNumber;
	bool showLineNum;
};

class LineNumbers : public QWidget
{
public:
	LineNumbers(AsmEditor* ed) : QWidget(ed), editor(ed)
	{
		setFont(ed->font());
	}
	QSize sizeHint() const override
	{
		return QSize(editor->lineNumberWidth(),0);
	}
protected:
	void paintEvent(QPaintEvent* ev) override
	{
		editor->lineNumberPaint(ev);
	}
private:
	AsmEditor* editor;
};

#endif /* UI_H_ */
