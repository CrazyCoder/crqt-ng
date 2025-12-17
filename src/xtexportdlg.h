/***************************************************************************
 *   crqt-ng                                                               *
 *   Copyright (C) 2025 Serge Baranov                                      *
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

#ifndef XTEXPORTDLG_H
#define XTEXPORTDLG_H

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QDialog>
#else
#include <QtGui/QDialog>
#endif

#include <lvbasedrawbuf.h>
#include <xtcexport.h>

class QTimer;
class XtExportProfileManager;
class XtExportProfile;
class PreviewWidget;
class LVDocView;

namespace Ui
{
    class XtExportDlg;
}

/**
 * @brief Export dialog for XT* formats (XTC/XTCH)
 *
 * Full-featured export dialog with live preview, profile management,
 * and integrated progress tracking.
 */
class XtExportDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(XtExportDlg)

public:
    explicit XtExportDlg(QWidget* parent, LVDocView* docView);
    ~XtExportDlg();

protected:
    void changeEvent(QEvent* e) override;

private slots:
    // Profile
    void onProfileChanged(int index);

    // Format settings
    void onFormatChanged(int index);
    void onWidthChanged(int value);
    void onHeightChanged(int value);

    // Dithering settings
    void onGrayPolicyChanged(int index);
    void onDitherModeChanged(int index);
    void onThresholdSliderChanged(int value);
    void onThresholdSpinBoxChanged(double value);
    void onErrorDiffusionSliderChanged(int value);
    void onErrorDiffusionSpinBoxChanged(double value);
    void onGammaSliderChanged(int value);
    void onGammaSpinBoxChanged(double value);
    void onSerpentineChanged(Qt::CheckState state);
    void onResetDithering();

    // Page range
    void onFromPageChanged(int value);
    void onToPageChanged(int value);

    // Chapters
    void onChaptersEnabledChanged(Qt::CheckState state);
    void onChapterDepthChanged(int index);

    // Preview navigation
    void onFirstPage();
    void onPrevPage();
    void onNextPage();
    void onLastPage();
    void onPreviewPageChanged(int value);

    // Zoom controls
    void onZoomSliderChanged(int value);
    void onResetZoom();
    void onSet200Zoom();

    // Preview widget wheel events
    void onPreviewPageChangeRequested(int delta);
    void onPreviewZoomChangeRequested(int delta);

    // Output path
    void onBrowseOutput();

    // Actions
    void onExport();
    void onCancel();

private:
    /**
     * @brief Initialize profile dropdown from ProfileManager
     */
    void initProfiles();

    /**
     * @brief Load settings from profile to UI controls
     */
    void loadProfileToUi(XtExportProfile* profile);

    /**
     * @brief Update dithering controls enable state based on dither mode
     */
    void updateDitheringControlsState();

    /**
     * @brief Update gray policy enable state based on format
     */
    void updateGrayPolicyState();

    /**
     * @brief Setup bidirectional sync between slider and spinbox
     */
    void setupSliderSpinBoxSync();

    /**
     * @brief Compute default output path
     */
    QString computeDefaultOutputPath();

    /**
     * @brief Resolve effective dither mode from UI selection
     *
     * For Floyd-Steinberg mode, automatically selects 1-bit or 2-bit
     * based on the current format selection (XTC=1-bit, XTCH=2-bit).
     *
     * @return The actual ImageDitherMode to use for export/preview
     */
    ImageDitherMode resolveEffectiveDitherMode() const;

    /**
     * @brief Schedule a debounced preview update
     */
    void schedulePreviewUpdate();

    /**
     * @brief Render preview page using XtcExporter
     */
    void renderPreview();

    /**
     * @brief Configure XtcExporter from current UI settings
     */
    void configureExporter(XtcExporter& exporter);

    /**
     * @brief Update page count display after resolution change
     */
    void updatePageCount(int totalPages);

    /**
     * @brief Load zoom setting from crui.ini
     */
    void loadZoomSetting();

    /**
     * @brief Save zoom setting to crui.ini
     */
    void saveZoomSetting();

    /**
     * @brief Update zoom button label based on current zoom level
     */
    void updateZoomButtonLabel();

    Ui::XtExportDlg* m_ui;
    LVDocView* m_docView;
    XtExportProfileManager* m_profileManager;
    PreviewWidget* m_previewWidget;

    int m_currentPreviewPage;       ///< Current preview page (0-based)
    int m_totalPageCount;           ///< Total page count at export resolution
    int m_zoomPercent;              ///< Current zoom level (50-200)
    bool m_updatingControls;        ///< Flag to prevent signal loops

    // Debounce timer for preview updates (Phase 3)
    QTimer* m_previewDebounceTimer;
};

#endif // XTEXPORTDLG_H
