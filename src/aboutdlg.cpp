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

#include "aboutdlg.h"
#include "ui_aboutdlg.h"
#include <QDesktopServices>
#include <QTextBrowser>

#include <crengine-ng-config.h>
#include "config.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::AboutDialog) {
    m_ui->setupUi(this);
    QString crqt_ver = m_ui->lblVersion->text().replace("%ver%", QString("%1").arg(VERSION));
    m_ui->lblVersion->setText(crqt_ver);
    QString crengine_ver = m_ui->lblcrengineVersion->text().replace("%ver%", QString("%1").arg(CRE_NG_VERSION));
    m_ui->lblcrengineVersion->setText(crengine_ver);
    QString project_src_url = "https://gitlab.com/coolreader-ng/crqt-ng/";
    // preambule
    QString aboutText =
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
            "p, li { white-space: pre-wrap; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; }"
            "</style></head><body>";
    aboutText += "<p>" + tr("CoolReaderNG is free open source e-book viewer based on crengine-ng library.") + "</p>";
    aboutText += "<p>" + tr("Source code is available at") + QString(" <a href=\"%1\">%1</a> ").arg(project_src_url) +
                 tr("under the terms of GNU GPLv2 license.") + "</p>";
    aboutText += "<p>" + tr("It is a fork of the 'CoolReader' program.") + "</p>";
    aboutText += "<p style=\"-qt-paragraph-type:empty;\"><br/></p>";
    aboutText += "<p><span style=\"text-decoration: underline;\">" + tr("Third party components used in crengine-ng:") +
                 "</span></p>";
#if USE_FREETYPE == 1
    aboutText += "<p>" + tr("FreeType - font rendering") + "</p>";
#endif
#if USE_HARFBUZZ == 1
    aboutText += "<p>" + tr("HarfBuzz - text shaping, font kerning, ligatures") + "</p>";
#endif
#if USE_ZLIB == 1
    aboutText += "<p>" + tr("ZLib - compressing library") + "</p>";
#endif
#if USE_ZSTD == 1
    aboutText += "<p>" + tr("ZSTD - compressing library") + "</p>";
#endif
#if USE_LIBPNG == 1
    aboutText += "<p>" + tr("libpng - PNG image format support") + "</p>";
#endif
#if USE_LIBJPEG == 1
    aboutText += "<p>" + tr("libjpeg - JPEG image format support") + "</p>";
#endif
#if USE_FRIBIDI == 1
    aboutText += "<p>" + tr("FriBiDi - RTL writing support") + "</p>";
#endif
#if USE_LIBUNIBREAK == 1
    aboutText += "<p>" + tr("libunibreak - line breaking and word breaking algorithms") + "</p>";
#endif
#if USE_UTF8PROC == 1
    aboutText += "<p>" + tr("utf8proc - for unicode string comparision") + "</p>";
#endif
#if USE_NANOSVG == 1
    aboutText += "<p>" + tr("NanoSVG - SVG image format support") + "</p>";
#endif
#if USE_CHM == 1
    aboutText += "<p>" + tr("chmlib - chm format support") + "</p>";
#endif
#if USE_ANTIWORD == 1
    aboutText += "<p>" + tr("antiword - Microsoft Word format support") + "</p>";
#endif
#if USE_SHASUM == 1
    aboutText += "<p>" + tr("RFC6234 (sources) - SHAsum") + "</p>";
#endif
    aboutText += "<p>" + tr("hyphman - AlReader hyphenation manager") + "</p>";
    aboutText += "<p>" + tr("Hyphenation dictionaries - from Android") + "</p>";
#if USE_LOCALE_DATA
    aboutText += "<p>" + tr("Languages character set database by Fontconfig") + "</p>";
#endif
    aboutText += "</body></html>";
    m_ui->textBrowser->setHtml(aboutText);
}

AboutDialog::~AboutDialog() {
    delete m_ui;
}

void AboutDialog::changeEvent(QEvent* e) {
    switch (e->type()) {
        case QEvent::LanguageChange:
            m_ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

bool AboutDialog::showDlg(QWidget* parent) {
    AboutDialog* dlg = new AboutDialog(parent);
    //dlg->setModal( true );
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
    return true;
}

void AboutDialog::on_buttonBox_accepted() {
    close();
}
