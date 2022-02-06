/**
 * crqt-ng
 *
 * (c) Vadim Lopatin, 2000-2014
 * (c) Other CoolReader authors (See AUTHORS file)
 * (c) crqt-ng authors (See AUTHORS file)
 *
 * This source code is distributed under the terms of
 * GNU General Public License version 2
 * See LICENSE file for details
 */

#ifndef ADDBOOKMARKDLG_H
#define ADDBOOKMARKDLG_H

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QDialog>
#else
#include <QtGui/QDialog>
#endif

namespace Ui
{
    class AddBookmarkDialog;
}

class CR3View;
class CRBookmark;

class AddBookmarkDialog: public QDialog
{
    Q_OBJECT
public:
    ~AddBookmarkDialog();

    static bool showDlg(QWidget* parent, CR3View* docView);
    static bool editBookmark(QWidget* parent, CR3View* docView, CRBookmark* bm);
protected:
    explicit AddBookmarkDialog(QWidget* parent, CR3View* docView, CRBookmark* bm);
    void changeEvent(QEvent* e);
    virtual void closeEvent(QCloseEvent* event);
private:
    Ui::AddBookmarkDialog* m_ui;
    CR3View* _docview;
    CRBookmark* _bm;
    bool _edit;

private slots:
    void on_cbType_currentIndexChanged(int index);
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // ADDBOOKMARKDLG_H
