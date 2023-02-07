/***************************************************************************
 *   crqt-ng                                                               *
 *   Copyright (C) 2009-2012,2014 Vadim Lopatin <coolreader.org@gmail.com> *
 *   Copyright (C) 2018 Mihail Slobodyanuk <slobodyanukma@gmail.com>       *
 *   Copyright (C) 2019,2020 Konstantin Potapov <pkbo@users.sourceforge.net>
 *   Copyright (C) 2018,2020-2023 Aleksey Chernov <valexlin@gmail.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License           *
 *   as published by the Free Software Foundation; either version 2        *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA 02110-1301, USA.                                                   *
 ***************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QStyleFactory>
#else
#include <QtGui/QFileDialog>
#include <QtGui/QStyleFactory>
#endif

#include <QClipboard>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QToolButton>

#include "app-props.h"
#include "settings.h"
#include "tocdlg.h"
#include "recentdlg.h"
#include "aboutdlg.h"
#include "filepropsdlg.h"
#include "bookmarklistdlg.h"
#include "addbookmarkdlg.h"
#include "crqtutil.h"
#include "wolexportdlg.h"
#include "exportprogressdlg.h"
#include "searchdlg.h"

#ifndef ENABLE_BOOKMARKS_DIR
#define ENABLE_BOOKMARKS_DIR 1
#endif

// In the engine, the maximum number of open documents is 16
// 1 reserved for preview in settings dialog
#define MAX_TABS_COUNT 15

MainWindow::MainWindow(const QStringList& filesToOpen, QWidget* parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindowClass)
        , _filenamesToOpen(filesToOpen)
        , _prevIndex(-1) {
    ui->setupUi(this);

    // New tab tool button on tab widget
    QToolButton* newTabButton = new QToolButton(this);
    newTabButton->setIcon(ui->actionNew_tab->icon());
    newTabButton->setToolTip(ui->actionNew_tab->toolTip());
    ui->tabWidget->setCornerWidget(newTabButton, Qt::TopLeftCorner);
    connect(newTabButton, SIGNAL(clicked()), this, SLOT(on_actionNew_tab_triggered()));

    QIcon icon = QIcon(":/icons/icons/crqt.png");
    CRLog::warn("\n\n\n*** ######### application icon %s\n\n\n", icon.isNull() ? "null" : "found");
    qApp->setWindowIcon(icon);

    addAction(ui->actionOpen);
    addAction(ui->actionRecentBooks);
    addAction(ui->actionTOC);
    addAction(ui->actionToggle_Full_Screen);
    addAction(ui->actionSettings);
    addAction(ui->actionClose);
    addAction(ui->actionToggle_Pages_Scroll);
    addAction(ui->actionMinimize);
    addAction(ui->actionNextPage);
    addAction(ui->actionPrevPage);
    addAction(ui->actionNextPage2);
    addAction(ui->actionPrevPage2);
    addAction(ui->actionNextPage3);
    addAction(ui->actionNextLine);
    addAction(ui->actionPrevLine);
    addAction(ui->actionFirstPage);
    addAction(ui->actionLastPage);
    addAction(ui->actionBack);
    addAction(ui->actionForward);
    addAction(ui->actionNextChapter);
    addAction(ui->actionPrevChapter);
    addAction(ui->actionZoom_In);
    addAction(ui->actionZoom_Out);
    addAction(ui->actionCopy);
    addAction(ui->actionCopy2); // alternative shortcut
    addAction(ui->actionAddBookmark);
    addAction(ui->actionShowBookmarksList);
    addAction(ui->actionToggleEditMode);
    addAction(ui->actionNextSentence);
    addAction(ui->actionPrevSentence);
    addAction(ui->actionNew_tab);

    QString configDir = cr2qt(getConfigDir());
    QString iniFile = configDir + "crui.ini";
    QString histFile = configDir + "cruihist.bmk";
    if (!_tabs.loadSettings(iniFile)) {
        // If config file not found in config dir
        //  save its to config dir
        _tabs.saveSettings(iniFile);
    }
    if (!_tabs.loadHistory(histFile)) {
        _tabs.saveHistory(histFile);
    }
    _tabs.restoreWindowPos(this, "main.", true);
    addNewDocTab();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    _tabs.saveWindowPos(this, "main.");
    _tabs.saveHistory();
    _tabs.saveSettings();
    int tabIndex = ui->tabWidget->currentIndex();
    if (tabIndex >= 0)
        _tabs.setCurrentDocument(_tabs[tabIndex].docPath());
    else
        _tabs.setCurrentDocument("");
    _tabs.saveTabSession(cr2qt(getConfigDir()) + "tabs.ini");
}

MainWindow::~MainWindow() {
    ui->tabWidget->clear();
    _tabs.cleanup();
    delete ui;
}

TabData MainWindow::createNewDocTabWidget() {
    QWidget* widget = new QWidget(ui->tabWidget, Qt::Widget);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    CR3View* view = new CR3View(widget);
    QScrollBar* scrollBar = new QScrollBar(Qt::Vertical, widget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(view, 10);
    layout->addWidget(scrollBar, 0);
    view->setScrollBar(scrollBar);
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));

    QString configDir = cr2qt(getConfigDir());
    QString engineDataDir = cr2qt(getEngineDataDir());
    QString cssFile = configDir + "fb2.css";
    QString cssFileInEngineDir = engineDataDir + "fb2.css";
    QString mainHyphDir = engineDataDir + "hyph" + QDir::separator();
    QString userHyphDir = configDir + "hyph" + QDir::separator();
    QString bookmarksDir = configDir + "bookmarks" + QDir::separator();

    view->setHyphDir(mainHyphDir);
    view->setHyphDir(userHyphDir + "hyph" + QDir::separator(), false);
    view->setPropsChangeCallback(this);
    view->setDocViewStatusCallback(this);
    view->setSharedSettings(_tabs.getSettings());
    view->setSharedHistory(_tabs.getHistory());
    if (!view->loadCSS(cssFile)) {
        view->loadCSS(cssFileInEngineDir);
    }
    lString32 backgound = _tabs.getSettings()->getStringDef(PROP_APP_BACKGROUND_IMAGE, "");
    if (!backgound.empty() && backgound[0] != '[') {
        LVImageSourceRef img;
        CRLog::debug("Background image file: %s", LCSTR(backgound));
        LVStreamRef stream = LVOpenFileStream(backgound.c_str(), LVOM_READ);
        if (!stream.isNull()) {
            img = LVCreateStreamImageSource(stream);
        }
        backgound = backgound.lowercase();
        bool tiled = (backgound.pos(cs32("\\textures\\")) >= 0 || backgound.pos(cs32("/textures/")) >= 0);
        view->getDocView()->setBackgroundImage(img, tiled);
    }
#if ENABLE_BOOKMARKS_DIR == 1
    view->setBookmarksDir(bookmarksDir);
#endif
    TabData tab(widget, layout, view, scrollBar);
    tab.setTitle(view->getDocTitle());
    return tab;
}

void MainWindow::addNewDocTab() {
    if (_tabs.count() >= MAX_TABS_COUNT) {
        QMessageBox::warning(this, tr("Warning"), tr("The maximum number of tabs has been exceeded!"), QMessageBox::Ok);
        return;
    }
    TabData tab = createNewDocTabWidget();
    if (tab.isValid()) {
        _tabs.append(tab);
        int tabIndex = ui->tabWidget->addTab(tab.widget(), tab.title());
        ui->tabWidget->setCurrentIndex(tabIndex);
        ui->tabWidget->setTabToolTip(tabIndex, tab.title());
    }
}

void MainWindow::closeDocTab(int index) {
    if (index >= 0 && index < ui->tabWidget->count()) {
        ui->tabWidget->removeTab(index);
        TabData tab = _tabs[index];
        _tabs.saveHistory();
        tab.cleanup();
        _tabs.remove(index);
    }
}

CR3View* MainWindow::currentCRView() const {
    CR3View* view = NULL;
    int tabIndex = ui->tabWidget->currentIndex();
    if (tabIndex >= 0)
        return _tabs[tabIndex].view();
    return view;
}

void MainWindow::syncTabWidget(const QString& currentDocument) {
    QString currentDocFilePath = currentDocument;
    if (currentDocFilePath.isEmpty()) {
        QWidget* widget = ui->tabWidget->currentWidget();
        for (TabsCollection::const_iterator it = _tabs.begin(); it != _tabs.end(); ++it) {
            const TabData& tab = *it;
            if (widget == tab.widget()) {
                currentDocFilePath = tab.docPath();
                break;
            }
        }
    }
    ui->tabWidget->blockSignals(true);
    ui->tabWidget->clear();
    int tabIndex = -1;
    QString currentTitle;
    for (TabsCollection::const_iterator it = _tabs.begin(); it != _tabs.end(); ++it) {
        const TabData& tab = *it;
        CR3View* view = tab.view();
        int index = ui->tabWidget->addTab(tab.widget(), tab.title());
        ui->tabWidget->setTabToolTip(index, tab.title());
        if (tab.docPath() == currentDocFilePath)
            tabIndex = index;
    }
    ui->tabWidget->blockSignals(false);
    if (-1 == tabIndex)
        tabIndex = ui->tabWidget->count() - 1;
    if (tabIndex >= 0) {
        ui->tabWidget->setCurrentIndex(tabIndex);
        const TabData& tab = _tabs[tabIndex];
        const CR3View* view = tab.view();
        if (NULL != view)
            currentTitle = tab.title();
    }
    if (!currentTitle.isEmpty())
        setWindowTitle("CoolReaderNG/Qt - " + currentTitle);
    else
        setWindowTitle("CoolReaderNG/Qt");
}

QString MainWindow::openFileDialogImpl() {
    QString lastPath;
    LVPtrVector<CRFileHistRecord>& files = _tabs.getHistory()->getRecords();
    if (files.length() > 0) {
        lString32 pathname = files[0]->getFilePathName();
        lString32 arcPathName, arcItemPathName;
        if (LVSplitArcName(pathname, arcPathName, arcItemPathName))
            lastPath = cr2qt(LVExtractPath(arcPathName));
        else
            lastPath = cr2qt(LVExtractPath(pathname));
    }
    QString all_fmt_flt =
#if (USE_CHM == 1) && ((USE_CMARK == 1) || (USE_CMARK_GFM == 1))
            QString(" (*.fb2 *.fb3 *.txt *.tcr *.rtf *.odt *.doc *.docx *.epub *.html *.shtml *.htm *.md *.chm *.zip *.pdb *.pml *.prc *.pml *.mobi);;");
#elif (USE_CHM == 1)
            QString(" (*.fb2 *.fb3 *.txt *.tcr *.rtf *.odt *.doc *.docx *.epub *.html *.shtml *.htm *.chm *.zip *.pdb *.pml *.prc *.pml *.mobi);;");
#else
            QString(" (*.fb2 *.fb3 *.txt *.tcr *.rtf *.odt *.doc *.docx *.epub *.html *.shtml *.htm *.zip *.pdb *.pml *.prc *.pml *.mobi);;");
#endif

    QString fileName = QFileDialog::getOpenFileName(
            this, tr("Open book file"), lastPath,
            // clang-format off
            tr("All supported formats") + all_fmt_flt +
                    tr("FB2 books") + QString(" (*.fb2 *.fb2.zip);;") +
                    tr("FB3 books") + QString(" (*.fb3);;") +
                    tr("Text files") + QString(" (*.txt);;") +
                    tr("Rich text") +  QString(" (*.rtf);;") +
                    tr("MS Word document") + QString(" (*.doc *.docx);;") +
                    tr("Open Document files") + QString(" (*.odt);;") +
                    tr("HTML files") + QString(" (*.shtml *.htm *.html);;") +
#if (USE_CMARK == 1) || (USE_CMARK_GFM == 1)
                    tr("Markdown files") + QString(" (*.md);;") +
#endif
                    tr("EPUB files") + QString(" (*.epub);;") +
#if USE_CHM == 1
                    tr("CHM files") + QString(" (*.chm);;") +
#endif
                    tr("MOBI files") + QString(" (*.mobi *.prc *.azw);;") +
                    tr("PalmDOC files") + QString(" (*.pdb *.pml);;") +
                    tr("ZIP archives") + QString(" (*.zip)"));
    // clang-format on
    return fileName;
}

class ExportProgressCallback: public LVDocViewCallback
{
    ExportProgressDlg* _dlg;
public:
    ExportProgressCallback(ExportProgressDlg* dlg) : _dlg(dlg) { }
    /// document formatting started
    virtual void OnFormatStart() {
        _dlg->setPercent(0);
    }
    /// document formatting finished
    virtual void OnFormatEnd() {
        _dlg->setPercent(100);
    }
    /// format progress, called with values 0..100
    virtual void OnFormatProgress(int percent) {
        //_dlg->setPercent(percent);
    }
    /// export progress, called with values 0..100
    virtual void OnExportProgress(int percent) {
        _dlg->setPercent(percent);
    }
};

void MainWindow::on_actionExport_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export document to"), QString(), tr("WOL book (*.wol)"));
    if (fileName.length() == 0)
        return;
    WolExportDlg* dlg = new WolExportDlg(this);
    //dlg->setModal( true );
    dlg->setWindowTitle(tr("Export to WOL format"));
    //    dlg->setModal( true );
    //    dlg->show();
    //dlg->raise();
    //dlg->activateWindow();
    int result = dlg->exec();
    if (result == QDialog::Accepted) {
        int bpp = dlg->getBitsPerPixel();
        int levels = dlg->getTocLevels();
        delete dlg;
        repaint();
        ExportProgressDlg* msg = new ExportProgressDlg(this);
        msg->show();
        msg->raise();
        msg->activateWindow();
        msg->repaint();
        repaint();
        ExportProgressCallback progress(msg);
        LVDocViewCallback* oldCallback = view->getDocView()->getCallback();
        view->getDocView()->setCallback(&progress);
        view->getDocView()->exportWolFile(qt2cr(fileName).c_str(), bpp > 1, levels);
        view->getDocView()->setCallback(oldCallback);
        delete msg;
    } else {
        delete dlg;
    }
}

void MainWindow::on_actionOpen_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    QString fileName = openFileDialogImpl();
    if (fileName.length() == 0)
        return;
    if (!view->loadDocument(fileName)) {
        // error
    } else {
        update();
    }
}

void MainWindow::on_actionMinimize_triggered() {
    showMinimized();
}

void MainWindow::on_actionClose_triggered() {
    close();
}

void MainWindow::on_actionNextPage_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->nextPage();
}

void MainWindow::on_actionPrevPage_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->prevPage();
}

void MainWindow::on_actionNextPage2_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->nextPage();
}

void MainWindow::on_actionPrevPage2_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->prevPage();
}

void MainWindow::on_actionNextLine_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->nextLine();
}

void MainWindow::on_actionPrevLine_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->prevLine();
}

void MainWindow::on_actionFirstPage_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->firstPage();
}

void MainWindow::on_actionLastPage_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->lastPage();
}

void MainWindow::on_actionBack_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->historyBack();
}

void MainWindow::on_actionForward_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->historyForward();
}

void MainWindow::on_actionNextChapter_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->nextChapter();
}

void MainWindow::on_actionPrevChapter_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->prevChapter();
}

void MainWindow::on_actionToggle_Pages_Scroll_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->togglePageScrollView();
}

void MainWindow::on_actionToggle_Full_Screen_triggered() {
    toggleProperty(PROP_WINDOW_FULLSCREEN);
}

void MainWindow::on_actionZoom_In_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->zoomIn();
}

void MainWindow::on_actionZoom_Out_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->zoomOut();
}

void MainWindow::on_actionTOC_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    TocDlg::showDlg(this, view);
}

void MainWindow::on_actionRecentBooks_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    RecentBooksDlg::showDlg(this, view);
}

void MainWindow::on_actionSettings_triggered() {
    CR3View* currView = currentCRView();
    if (NULL == currView) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    SettingsDlg dlg(this, currView->getOptions());
    if (dlg.exec() == QDialog::Accepted) {
        for (TabsCollection::iterator it = _tabs.begin(); it != _tabs.end(); ++it) {
            CR3View* view = (*it).view();
            if (NULL != view) {
                PropsRef newProps = dlg.options();
                _tabs.setSettings(qt2cr(newProps));
                view->setOptions(newProps, view == currView);
                view->getDocView()->requestRender();
                // docview is not rendered here, only planned
            }
        }
    }
}

void MainWindow::toggleProperty(const char* name) {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->toggleProperty(name);
}

void MainWindow::onPropsChange(PropsRef props) {
    for (int i = 0; i < props->count(); i++) {
        QString name = props->name(i);
        QString value = props->value(i);
        int v = (value != "0");
        CRLog::debug("MainWindow::onPropsChange [%d] '%s'=%s ", i, props->name(i), props->value(i).toUtf8().data());
        if (name == PROP_WINDOW_FULLSCREEN) {
            bool state = windowState().testFlag(Qt::WindowFullScreen);
            bool vv = v ? true : false;
            if (state != vv)
                setWindowState(windowState() ^ Qt::WindowFullScreen);
        } else if (name == PROP_WINDOW_SHOW_MENU) {
            ui->menuBar->setVisible(v);
        } else if (name == PROP_WINDOW_SHOW_SCROLLBAR) {
            for (int i = 0; i < _tabs.count(); i++) {
                const TabData& tab = _tabs[i];
                if (NULL != tab.scrollBar())
                    tab.scrollBar()->setVisible(v);
            }
        } else if (name == PROP_APP_BACKGROUND_IMAGE) {
            lString32 fn = qt2cr(value);
            LVImageSourceRef img;
            if (!fn.empty() && fn[0] != '[') {
                CRLog::debug("Background image file: %s", LCSTR(fn));
                LVStreamRef stream = LVOpenFileStream(fn.c_str(), LVOM_READ);
                if (!stream.isNull()) {
                    img = LVCreateStreamImageSource(stream);
                }
            }
            fn.lowercase();
            bool tiled = (fn.pos(cs32("\\textures\\")) >= 0 || fn.pos(cs32("/textures/")) >= 0);
            for (int i = 0; i < _tabs.count(); i++) {
                const TabData& tab = _tabs[i];
                if (NULL != tab.view())
                    tab.view()->getDocView()->setBackgroundImage(img, tiled);
            }
        } else if (name == PROP_WINDOW_TOOLBAR_SIZE) {
            ui->mainToolBar->setVisible(v);
        } else if (name == PROP_WINDOW_SHOW_STATUSBAR) {
            ui->statusBar->setVisible(v);
        } else if (name == PROP_WINDOW_STYLE) {
            QApplication::setStyle(value);
        }
    }
}

void MainWindow::onDocumentLoaded(lUInt64 viewId, const QString& atitle, const QString& error,
                                  const QString& fullDocPath) {
    CRLog::debug("MainWindow::onDocumentLoaded '%s', error=%s ", atitle.toLocal8Bit().constData(),
                 error.toLocal8Bit().constData());
    int cbIndex = _tabs.indexByViewId(viewId);
    int currentIndex = ui->tabWidget->currentIndex();
    if (cbIndex >= 0) {
        if (error.isEmpty()) {
            TabData& tab = _tabs[cbIndex];
            tab.setTitle(atitle);
            tab.setDocPath(fullDocPath);
            ui->tabWidget->setTabText(cbIndex, atitle);
            ui->tabWidget->setTabToolTip(cbIndex, atitle);
        }
        if (cbIndex == currentIndex) {
            if (error.isEmpty()) {
                setWindowTitle("CoolReaderNG/Qt - " + atitle);
            } else {
                setWindowTitle("CoolReaderNG/Qt");
            }
        }
    }
}

void MainWindow::onCanGoBack(lUInt64 viewId, bool canGoBack) {
    int cbIndex = _tabs.indexByViewId(viewId);
    int currentIndex = ui->tabWidget->currentIndex();
    if (cbIndex >= 0) {
        _tabs[cbIndex].setCanGoBack(canGoBack);
        if (cbIndex == currentIndex)
            ui->actionBack->setEnabled(canGoBack);
    }
}

void MainWindow::onCanGoForward(lUInt64 viewId, bool canGoForward) {
    int cbIndex = _tabs.indexByViewId(viewId);
    int currentIndex = ui->tabWidget->currentIndex();
    if (cbIndex >= 0) {
        _tabs[cbIndex].setCanGoForward(canGoForward);
        if (cbIndex == currentIndex)
            ui->actionForward->setEnabled(canGoForward);
    }
}

void MainWindow::contextMenu(QPoint pos) {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    QMenu* menu = new QMenu;
    menu->addAction(ui->actionOpen);
    menu->addAction(ui->actionRecentBooks);
    menu->addAction(ui->actionTOC);
    menu->addAction(ui->actionToggle_Full_Screen);
    menu->addAction(ui->actionSettings);
    if (view->isPointInsideSelection(pos))
        menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionAddBookmark);
    menu->addAction(ui->actionClose);
    menu->exec(view->mapToGlobal(pos));
}

void MainWindow::on_actionCopy_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    QString txt = view->getSelectionText();
    if (txt.length() > 0) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(txt);
    }
}

void MainWindow::on_actionCopy2_triggered() {
    on_actionCopy_triggered();
}

static bool firstShow = true;

void MainWindow::showEvent(QShowEvent* event) {
    if (!firstShow)
        return;
    CRLog::debug("first showEvent()");
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    firstShow = false;
    int n = view->getOptions()->getIntDef(PROP_APP_START_ACTION, 0);
    if (!_filenamesToOpen.isEmpty()) {
        // file names specified at command line
        CRLog::info("Startup Action: filename passed in command line");
        int processed = 0;
        int index = 0;
        TabData tab;
        for (QStringList::const_iterator it = _filenamesToOpen.begin(); it != _filenamesToOpen.end(); ++it, ++index) {
            const QString& filePath = *it;
            if (index < _tabs.size()) {
                tab = _tabs[index];
            } else {
                tab = createNewDocTabWidget();
                _tabs.append(tab);
            }
            // When opening files from the command line,
            //  open them immediately without delay.
            //  To have information on all tabs at once.
            tab.view()->setActive(true);
            if (!tab.view()->loadDocument(filePath)) {
                CRLog::error("cannot load document \"%s\"", filePath.toLocal8Bit().constData());
            }
            processed++;
            if (processed >= MAX_TABS_COUNT)
                break;
        }
        if (_tabs.size() > processed) {
            for (int i = processed; i < _tabs.size(); i++) {
                _tabs[i].cleanup();
            }
            _tabs.resize(processed);
        }
        if (0 == _tabs.size()) {
            tab = createNewDocTabWidget();
            if (tab.isValid()) {
                _tabs.append(tab);
            }
        }
        syncTabWidget();
    } else if (n == 0) {
        // restore session
        CRLog::info("Startup Action: Restore session (restore tabs)");
        bool ok;
        TabsCollection::TabSession session = _tabs.openTabSession(cr2qt(getConfigDir()) + "tabs.ini", &ok);
        if (ok && session.size() > 0) {
            int processed = 0;
            int index = 0;
            TabData tab;
            for (TabsCollection::TabSession::const_iterator it = session.begin(); it != session.end(); ++it, ++index) {
                const TabsCollection::TabProperty& data = *it;
                if (index < _tabs.size()) {
                    tab = _tabs[index];
                } else {
                    tab = createNewDocTabWidget();
                    _tabs.append(tab);
                }
                tab.setTitle(data.title);
                tab.setDocPath(data.docPath);
                _tabs[index] = tab;
                if (!data.docPath.isEmpty()) {
                    if (!tab.view()->loadDocument(data.docPath)) {
                        CRLog::error("cannot load document \"%s\"", data.docPath.toLocal8Bit().constData());
                    }
                }
                processed++;
                if (processed >= MAX_TABS_COUNT)
                    break;
            }
            if (_tabs.size() > processed) {
                for (int i = processed; i < _tabs.size(); i++) {
                    _tabs[i].cleanup();
                }
                _tabs.resize(processed);
            }
            if (0 == _tabs.size()) {
                tab = createNewDocTabWidget();
                if (tab.isValid()) {
                    _tabs.append(tab);
                }
            }
            syncTabWidget(session.currentDocument);
        } else {
            view->loadLastDocument();
        }
    } else if (n == 1) {
        // show recent books dialog
        CRLog::info("Startup Action: Show recent books dialog");
        //hide();
        RecentBooksDlg::showDlg(this, view);
        //show();
    } else if (n == 2) {
        // show file open dialog
        CRLog::info("Startup Action: Show file open dialog");
        //hide();
        on_actionOpen_triggered();
        //RecentBooksDlg::showDlg( ui->view );
        //show();
    }
}

static bool firstFocus = true;

void MainWindow::focusInEvent(QFocusEvent* event) {
    if (!firstFocus)
        return;
    CRLog::debug("first focusInEvent()");
    //    int n = ui->view->getOptions()->getIntDef( PROP_APP_START_ACTION, 0 );
    //    if ( n==1 ) {
    //        // show recent books dialog
    //        CRLog::info("Startup Action: Show recent books dialog");
    //        RecentBooksDlg::showDlg( ui->view );
    //    }

    firstFocus = false;
}

void MainWindow::on_actionAboutQT_triggered() {
    QApplication::aboutQt();
}

void MainWindow::on_actionAboutCoolReader_triggered() {
    AboutDialog::showDlg(this);
}

void MainWindow::on_actionAddBookmark_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    AddBookmarkDialog::showDlg(this, view);
}

void MainWindow::on_actionShowBookmarksList_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    BookmarkListDialog::showDlg(this, view);
}

void MainWindow::on_actionFileProperties_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    FilePropsDialog::showDlg(this, view);
}

void MainWindow::on_actionFindText_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    SearchDialog::showDlg(this, view);
    //    QMessageBox * mb = new QMessageBox( QMessageBox::Information, tr("Not implemented"), tr("Search is not implemented yet"), QMessageBox::Close, this );
    //    mb->exec();
}

void MainWindow::on_actionRotate_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->rotate(1);
}

void MainWindow::on_actionToggleEditMode_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->setEditMode(!view->getEditMode());
}

void MainWindow::on_actionNextPage3_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->nextPage();
}

void MainWindow::on_actionNextSentence_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->nextSentence();
}

void MainWindow::on_actionPrevSentence_triggered() {
    CR3View* view = currentCRView();
    if (NULL == view) {
        CRLog::debug("NULL view in current tab!");
        return;
    }
    view->prevSentence();
}

void MainWindow::on_actionNew_tab_triggered() {
    addNewDocTab();
}

void MainWindow::on_tabWidget_currentChanged(int index) {
    QString title;
    if (_prevIndex >= 0 && _prevIndex < _tabs.size()) {
        const TabData& tab = _tabs[_prevIndex];
        CR3View* view = tab.view();
        if (NULL != view) {
            view->getDocView()->swapToCache();
            view->getDocView()->savePosition();
            view->setActive(false);
        }
    }
    if (index >= 0 && index < _tabs.size()) {
        const TabData& tab = _tabs[index];
        CR3View* view = tab.view();
        if (NULL != view) {
            view->setActive(true);
            title = tab.title();
            view->getDocView()->swapToCache();
            view->getDocView()->savePosition();
            ui->actionBack->setEnabled(tab.canGoBack());
            ui->actionForward->setEnabled(tab.canGoForward());
        }
    }
    if (!title.isEmpty())
        setWindowTitle("CoolReaderNG/Qt - " + title);
    else
        setWindowTitle("CoolReaderNG/Qt");
    _prevIndex = index;
}

void MainWindow::on_tabWidget_tabCloseRequested(int index) {
    closeDocTab(index);
}

void MainWindow::on_actionOpen_in_new_tab_triggered() {
    QString fileName = openFileDialogImpl();
    if (fileName.length() == 0)
        return;
    TabData tab = createNewDocTabWidget();
    if (tab.isValid()) {
        _tabs.append(tab);
        CR3View* view = tab.view();
        int tabIndex = ui->tabWidget->addTab(tab.widget(), tab.title());
        ui->tabWidget->setCurrentIndex(tabIndex);
        ui->tabWidget->setTabToolTip(tabIndex, tab.title());
        if (!view->loadDocument(fileName)) {
            // error
        } else {
            update();
        }
    }
}
