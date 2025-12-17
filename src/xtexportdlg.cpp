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

#include "xtexportdlg.h"
#include "ui_xtexportdlg.h"
#include "previewwidget.h"
#include "xtexportprofile.h"
#include "mainwindow.h"
#include "crqtutil.h"

#include <lvdocview.h>
#include <xtcexport.h>

#include <QTimer>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QToolTip>

XtExportDlg::XtExportDlg(QWidget* parent, LVDocView* docView)
    : QDialog(parent)
    , m_ui(new Ui::XtExportDlg)
    , m_docView(docView)
    , m_profileManager(new XtExportProfileManager())
    , m_previewWidget(nullptr)
    , m_currentPreviewPage(0)
    , m_totalPageCount(1)
    , m_zoomPercent(100)
    , m_updatingControls(false)
    , m_previewDebounceTimer(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);  // Ensure destructor runs when dialog closes
    m_ui->setupUi(this);

    // Set zoom slider max from constant (overrides .ui file value)
    m_ui->sliderZoom->setMaximum(PreviewWidget::ZOOM_LEVEL_MAX);

    // Initialize profile manager
    m_profileManager->initialize();

    // Create and insert preview widget into placeholder
    m_previewWidget = new PreviewWidget(this);
    QVBoxLayout* previewContainerLayout = new QVBoxLayout(m_ui->previewPlaceholder);
    previewContainerLayout->setContentsMargins(0, 0, 0, 0);
    previewContainerLayout->addWidget(m_previewWidget);

    // Setup debounce timer for preview updates
    m_previewDebounceTimer = new QTimer(this);
    m_previewDebounceTimer->setSingleShot(true);
    m_previewDebounceTimer->setInterval(50);
    connect(m_previewDebounceTimer, &QTimer::timeout, this, &XtExportDlg::renderPreview);

    // Initialize profiles dropdown
    initProfiles();

    // Setup slider/spinbox synchronization
    setupSliderSpinBoxSync();

    // Connect profile change
    connect(m_ui->cbProfile, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &XtExportDlg::onProfileChanged);

    // Connect format settings
    connect(m_ui->cbFormat, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &XtExportDlg::onFormatChanged);
    connect(m_ui->sbWidth, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &XtExportDlg::onWidthChanged);
    connect(m_ui->sbHeight, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &XtExportDlg::onHeightChanged);

    // Connect dithering settings
    connect(m_ui->cbGrayPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &XtExportDlg::onGrayPolicyChanged);
    connect(m_ui->cbDitherMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &XtExportDlg::onDitherModeChanged);
    connect(m_ui->cbSerpentine, &QCheckBox::checkStateChanged,
            this, &XtExportDlg::onSerpentineChanged);
    connect(m_ui->btnResetDithering, &QPushButton::clicked,
            this, &XtExportDlg::onResetDithering);

    // Connect page range
    connect(m_ui->sbFromPage, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &XtExportDlg::onFromPageChanged);
    connect(m_ui->sbToPage, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &XtExportDlg::onToPageChanged);

    // Connect chapters
    connect(m_ui->cbChaptersEnabled, &QCheckBox::checkStateChanged,
            this, &XtExportDlg::onChaptersEnabledChanged);
    connect(m_ui->cbChapterDepth, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &XtExportDlg::onChapterDepthChanged);

    // Connect preview navigation
    connect(m_ui->btnFirstPage, &QPushButton::clicked,
            this, &XtExportDlg::onFirstPage);
    connect(m_ui->btnPrevPage, &QPushButton::clicked,
            this, &XtExportDlg::onPrevPage);
    connect(m_ui->btnNextPage, &QPushButton::clicked,
            this, &XtExportDlg::onNextPage);
    connect(m_ui->btnLastPage, &QPushButton::clicked,
            this, &XtExportDlg::onLastPage);
    connect(m_ui->sbPreviewPage, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &XtExportDlg::onPreviewPageChanged);

    // Connect zoom controls
    connect(m_ui->sliderZoom, &QSlider::valueChanged,
            this, &XtExportDlg::onZoomSliderChanged);
    connect(m_ui->btnResetZoom, &QPushButton::clicked,
            this, &XtExportDlg::onResetZoom);
    connect(m_ui->btn200Zoom, &QPushButton::clicked,
            this, &XtExportDlg::onSet200Zoom);

    // Connect preview widget mouse events
    connect(m_previewWidget, &PreviewWidget::pageChangeRequested,
            this, &XtExportDlg::onPreviewPageChangeRequested);
    connect(m_previewWidget, &PreviewWidget::zoomChangeRequested,
            this, &XtExportDlg::onPreviewZoomChangeRequested);
    connect(m_previewWidget, &PreviewWidget::zoomResetRequested,
            this, &XtExportDlg::onResetZoom);

    // Connect output path
    connect(m_ui->btnBrowse, &QPushButton::clicked,
            this, &XtExportDlg::onBrowseOutput);

    // Connect actions
    connect(m_ui->btnExport, &QPushButton::clicked,
            this, &XtExportDlg::onExport);
    // Cancel is already connected via UI file to reject()

    // Load first profile if available
    if (m_profileManager->profileCount() > 0) {
        loadProfileToUi(m_profileManager->profileByIndex(0));
    }

    // Set default output path
    m_ui->leOutputPath->setText(computeDefaultOutputPath());

    // Update control states
    updateDitheringControlsState();
    updateGrayPolicyState();

    // Load saved zoom setting
    loadZoomSetting();

    // Set initial preview page to current document page
    if (m_docView) {
        int currentPage = m_docView->getCurPage();  // 0-based page number
        m_currentPreviewPage = currentPage;
        m_ui->sbPreviewPage->setValue(currentPage + 1);  // SpinBox is 1-based
    }

    // Trigger initial preview after a short delay (allow dialog to be fully shown)
    QTimer::singleShot(100, this, &XtExportDlg::renderPreview);
}

XtExportDlg::~XtExportDlg()
{
    saveZoomSetting();
    delete m_profileManager;
    delete m_ui;
}

void XtExportDlg::changeEvent(QEvent* e)
{
    QDialog::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        m_ui->retranslateUi(this);
    }
}


void XtExportDlg::initProfiles()
{
    m_ui->cbProfile->clear();
    QStringList names = m_profileManager->profileNames();
    m_ui->cbProfile->addItems(names);
}

void XtExportDlg::loadProfileToUi(XtExportProfile* profile)
{
    if (!profile)
        return;

    m_updatingControls = true;

    // Format settings
    m_ui->cbFormat->setCurrentIndex(profile->format == XTC_FORMAT_XTC ? 0 : 1);
    m_ui->sbWidth->setValue(profile->width);
    m_ui->sbHeight->setValue(profile->height);

    // Dithering settings
    // Gray policy: 0=Balanced, 1=Preserve Light, 2=High Contrast
    int grayPolicyIndex = 0;
    switch (profile->grayPolicy) {
        case GRAY_SPLIT_LIGHT_DARK: grayPolicyIndex = 0; break;
        case GRAY_ALL_TO_WHITE: grayPolicyIndex = 1; break;
        case GRAY_ALL_TO_BLACK: grayPolicyIndex = 2; break;
    }
    m_ui->cbGrayPolicy->setCurrentIndex(grayPolicyIndex);

    // Dither mode: 0=None, 1=Ordered, 2=Floyd-Steinberg (auto-selects 1-bit or 2-bit based on format)
    int ditherModeIndex = 0;
    switch (profile->ditherMode) {
        case IMAGE_DITHER_NONE: ditherModeIndex = 0; break;
        case IMAGE_DITHER_ORDERED: ditherModeIndex = 1; break;
        case IMAGE_DITHER_FS_1BIT:
        case IMAGE_DITHER_FS_2BIT: ditherModeIndex = 2; break;
    }
    m_ui->cbDitherMode->setCurrentIndex(ditherModeIndex);

    // FS parameters
    m_ui->sbThreshold->setValue(profile->ditheringOpts.threshold);
    m_ui->sliderThreshold->setValue(static_cast<int>(profile->ditheringOpts.threshold * 100));
    m_ui->sbErrorDiffusion->setValue(profile->ditheringOpts.errorDiffusion);
    m_ui->sliderErrorDiffusion->setValue(static_cast<int>(profile->ditheringOpts.errorDiffusion * 100));
    m_ui->sbGamma->setValue(profile->ditheringOpts.gamma);
    m_ui->sliderGamma->setValue(static_cast<int>(profile->ditheringOpts.gamma * 100));
    m_ui->cbSerpentine->setChecked(profile->ditheringOpts.serpentine);

    // Chapters
    m_ui->cbChaptersEnabled->setChecked(profile->chaptersEnabled);
    m_ui->cbChapterDepth->setCurrentIndex(profile->chapterDepth - 1);

    m_updatingControls = false;

    // Update control states
    updateDitheringControlsState();
    updateGrayPolicyState();
}

void XtExportDlg::updateDitheringControlsState()
{
    // FS parameters are enabled only for Floyd-Steinberg dithering mode (index 2)
    int modeIndex = m_ui->cbDitherMode->currentIndex();
    bool enableFs = (modeIndex == 2);

    m_ui->sliderThreshold->setEnabled(enableFs);
    m_ui->sbThreshold->setEnabled(enableFs);
    m_ui->sliderErrorDiffusion->setEnabled(enableFs);
    m_ui->sbErrorDiffusion->setEnabled(enableFs);
    m_ui->sliderGamma->setEnabled(enableFs);
    m_ui->sbGamma->setEnabled(enableFs);
    m_ui->cbSerpentine->setEnabled(enableFs);
}

void XtExportDlg::updateGrayPolicyState()
{
    // Gray policy is disabled for XTCH format (2-bit doesn't need gray-to-mono)
    int formatIndex = m_ui->cbFormat->currentIndex();
    bool enableGrayPolicy = (formatIndex == 0);  // XTC only
    m_ui->cbGrayPolicy->setEnabled(enableGrayPolicy);
}

void XtExportDlg::setupSliderSpinBoxSync()
{
    // Threshold: slider 0-100 maps to spinbox 0.00-1.00
    connect(m_ui->sliderThreshold, &QSlider::valueChanged,
            this, &XtExportDlg::onThresholdSliderChanged);
    connect(m_ui->sbThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &XtExportDlg::onThresholdSpinBoxChanged);

    // Error Diffusion: slider 0-100 maps to spinbox 0.00-1.00
    connect(m_ui->sliderErrorDiffusion, &QSlider::valueChanged,
            this, &XtExportDlg::onErrorDiffusionSliderChanged);
    connect(m_ui->sbErrorDiffusion, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &XtExportDlg::onErrorDiffusionSpinBoxChanged);

    // Gamma: slider 50-400 maps to spinbox 0.50-4.00
    connect(m_ui->sliderGamma, &QSlider::valueChanged,
            this, &XtExportDlg::onGammaSliderChanged);
    connect(m_ui->sbGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &XtExportDlg::onGammaSpinBoxChanged);
}

QString XtExportDlg::computeDefaultOutputPath()
{
    // TODO (Phase 4): Get source document directory and name
    // For now, return placeholder
    return QString();
}

ImageDitherMode XtExportDlg::resolveEffectiveDitherMode() const
{
    int modeIndex = m_ui->cbDitherMode->currentIndex();
    switch (modeIndex) {
        case 0: return IMAGE_DITHER_NONE;
        case 1: return IMAGE_DITHER_ORDERED;
        case 2: {
            // Floyd-Steinberg: select 1-bit or 2-bit based on format
            // XTC (index 0) = 1-bit, XTCH (index 1) = 2-bit
            int formatIndex = m_ui->cbFormat->currentIndex();
            return (formatIndex == 0) ? IMAGE_DITHER_FS_1BIT : IMAGE_DITHER_FS_2BIT;
        }
        default: return IMAGE_DITHER_NONE;
    }
}

// Slot implementations

void XtExportDlg::onProfileChanged(int index)
{
    if (m_updatingControls)
        return;

    XtExportProfile* profile = m_profileManager->profileByIndex(index);
    if (profile) {
        loadProfileToUi(profile);
    }
    schedulePreviewUpdate();
}

void XtExportDlg::onFormatChanged(int /*index*/)
{
    if (m_updatingControls)
        return;
    updateDitheringControlsState();
    updateGrayPolicyState();
    schedulePreviewUpdate();
}

void XtExportDlg::onWidthChanged(int /*value*/)
{
    if (m_updatingControls)
        return;
    schedulePreviewUpdate();
}

void XtExportDlg::onHeightChanged(int /*value*/)
{
    if (m_updatingControls)
        return;
    schedulePreviewUpdate();
}

void XtExportDlg::onGrayPolicyChanged(int /*index*/)
{
    if (m_updatingControls)
        return;
    schedulePreviewUpdate();
}

void XtExportDlg::onDitherModeChanged(int /*index*/)
{
    if (m_updatingControls)
        return;
    updateDitheringControlsState();
    schedulePreviewUpdate();
}

void XtExportDlg::onThresholdSliderChanged(int value)
{
    if (m_updatingControls)
        return;
    m_updatingControls = true;
    m_ui->sbThreshold->setValue(value / 100.0);
    m_updatingControls = false;
    schedulePreviewUpdate();
}

void XtExportDlg::onThresholdSpinBoxChanged(double value)
{
    if (m_updatingControls)
        return;
    m_updatingControls = true;
    m_ui->sliderThreshold->setValue(static_cast<int>(value * 100));
    m_updatingControls = false;
    schedulePreviewUpdate();
}

void XtExportDlg::onErrorDiffusionSliderChanged(int value)
{
    if (m_updatingControls)
        return;
    m_updatingControls = true;
    m_ui->sbErrorDiffusion->setValue(value / 100.0);
    m_updatingControls = false;
    schedulePreviewUpdate();
}

void XtExportDlg::onErrorDiffusionSpinBoxChanged(double value)
{
    if (m_updatingControls)
        return;
    m_updatingControls = true;
    m_ui->sliderErrorDiffusion->setValue(static_cast<int>(value * 100));
    m_updatingControls = false;
    schedulePreviewUpdate();
}

void XtExportDlg::onGammaSliderChanged(int value)
{
    if (m_updatingControls)
        return;
    m_updatingControls = true;
    m_ui->sbGamma->setValue(value / 100.0);
    m_updatingControls = false;
    schedulePreviewUpdate();
}

void XtExportDlg::onGammaSpinBoxChanged(double value)
{
    if (m_updatingControls)
        return;
    m_updatingControls = true;
    m_ui->sliderGamma->setValue(static_cast<int>(value * 100));
    m_updatingControls = false;
    schedulePreviewUpdate();
}

void XtExportDlg::onSerpentineChanged(Qt::CheckState /*state*/)
{
    if (m_updatingControls)
        return;
    schedulePreviewUpdate();
}

void XtExportDlg::onResetDithering()
{
    // Get default profile based on current format (XTC=0, XTCH=1)
    int formatIndex = m_ui->cbFormat->currentIndex();
    XtExportProfile defaultProfile = (formatIndex == 0)
        ? XtExportProfile::defaultXtcProfile()
        : XtExportProfile::defaultXtchProfile();

    m_updatingControls = true;

    // Reset gray policy
    int grayPolicyIndex = 0;
    switch (defaultProfile.grayPolicy) {
        case GRAY_SPLIT_LIGHT_DARK: grayPolicyIndex = 0; break;
        case GRAY_ALL_TO_WHITE: grayPolicyIndex = 1; break;
        case GRAY_ALL_TO_BLACK: grayPolicyIndex = 2; break;
    }
    m_ui->cbGrayPolicy->setCurrentIndex(grayPolicyIndex);

    // Reset dither mode
    int ditherModeIndex = 0;
    switch (defaultProfile.ditherMode) {
        case IMAGE_DITHER_NONE: ditherModeIndex = 0; break;
        case IMAGE_DITHER_ORDERED: ditherModeIndex = 1; break;
        case IMAGE_DITHER_FS_1BIT:
        case IMAGE_DITHER_FS_2BIT: ditherModeIndex = 2; break;
    }
    m_ui->cbDitherMode->setCurrentIndex(ditherModeIndex);

    // Reset FS parameters
    m_ui->sbThreshold->setValue(defaultProfile.ditheringOpts.threshold);
    m_ui->sliderThreshold->setValue(static_cast<int>(defaultProfile.ditheringOpts.threshold * 100));
    m_ui->sbErrorDiffusion->setValue(defaultProfile.ditheringOpts.errorDiffusion);
    m_ui->sliderErrorDiffusion->setValue(static_cast<int>(defaultProfile.ditheringOpts.errorDiffusion * 100));
    m_ui->sbGamma->setValue(defaultProfile.ditheringOpts.gamma);
    m_ui->sliderGamma->setValue(static_cast<int>(defaultProfile.ditheringOpts.gamma * 100));
    m_ui->cbSerpentine->setChecked(defaultProfile.ditheringOpts.serpentine);

    m_updatingControls = false;

    // Update control states and trigger preview
    updateDitheringControlsState();
    schedulePreviewUpdate();
}

void XtExportDlg::onFromPageChanged(int value)
{
    if (m_updatingControls)
        return;

    // Swap if From > To
    if (value > m_ui->sbToPage->value()) {
        m_updatingControls = true;
        int to = m_ui->sbToPage->value();
        m_ui->sbToPage->setValue(value);
        m_ui->sbFromPage->setValue(to);
        m_updatingControls = false;
    }
}

void XtExportDlg::onToPageChanged(int value)
{
    if (m_updatingControls)
        return;

    // Swap if To < From
    if (value < m_ui->sbFromPage->value()) {
        m_updatingControls = true;
        int from = m_ui->sbFromPage->value();
        m_ui->sbFromPage->setValue(value);
        m_ui->sbToPage->setValue(from);
        m_updatingControls = false;
    }
}

void XtExportDlg::onChaptersEnabledChanged(Qt::CheckState /*state*/)
{
    if (m_updatingControls)
        return;
    // Chapter depth combo could be enabled/disabled based on this
}

void XtExportDlg::onChapterDepthChanged(int /*index*/)
{
    if (m_updatingControls)
        return;
}

void XtExportDlg::onFirstPage()
{
    m_ui->sbPreviewPage->setValue(1);
}

void XtExportDlg::onPrevPage()
{
    int page = m_ui->sbPreviewPage->value();
    if (page > 1) {
        m_ui->sbPreviewPage->setValue(page - 1);
    }
}

void XtExportDlg::onNextPage()
{
    int page = m_ui->sbPreviewPage->value();
    int maxPage = m_ui->sbPreviewPage->maximum();
    if (page < maxPage) {
        m_ui->sbPreviewPage->setValue(page + 1);
    }
}

void XtExportDlg::onLastPage()
{
    m_ui->sbPreviewPage->setValue(m_ui->sbPreviewPage->maximum());
}

void XtExportDlg::onPreviewPageChanged(int value)
{
    if (m_updatingControls)
        return;
    m_currentPreviewPage = value - 1;  // Convert to 0-based
    // Reset pan and render immediately (no debounce for page navigation)
    renderPreview();
}

void XtExportDlg::onZoomSliderChanged(int value)
{
    m_zoomPercent = value;
    m_previewWidget->setZoom(value);
    updateZoomButtonLabel();

    // Show transient tooltip centered above the slider
    QString tooltipText = QString("%1%").arg(value);

    // Calculate tooltip size using font metrics
    QFontMetrics fm(QToolTip::font());
    QRect textRect = fm.boundingRect(tooltipText);
    // Add padding for tooltip frame (typically ~6px horizontal, ~4px vertical)
    int tooltipWidth = textRect.width() + 12;
    int tooltipHeight = textRect.height() + 8;

    // Position: centered horizontally, above slider with gap
    int gap = 20;
    int x = (m_ui->sliderZoom->width() - tooltipWidth) / 2;
    int y = -tooltipHeight - gap;

    QPoint pos = m_ui->sliderZoom->mapToGlobal(QPoint(x, y));
    QToolTip::showText(pos, tooltipText, m_ui->sliderZoom);
}

void XtExportDlg::onResetZoom()
{
    m_ui->sliderZoom->setValue(100);
}

void XtExportDlg::onSet200Zoom()
{
    // Cycle through zoom levels: find next level higher than current, or wrap to first
    // The button label update is handled by onZoomSliderChanged() -> updateZoomButtonLabel()
    for (int i = 0; i < PreviewWidget::ZOOM_LEVELS_COUNT; ++i) {
        if (m_zoomPercent < PreviewWidget::ZOOM_LEVELS[i]) {
            m_ui->sliderZoom->setValue(PreviewWidget::ZOOM_LEVELS[i]);
            return;
        }
    }
    // At or above max level, wrap to first
    m_ui->sliderZoom->setValue(PreviewWidget::ZOOM_LEVELS[0]);
}

void XtExportDlg::onPreviewPageChangeRequested(int delta)
{
    int newPage = m_ui->sbPreviewPage->value() + delta;
    newPage = qBound(1, newPage, m_ui->sbPreviewPage->maximum());
    m_ui->sbPreviewPage->setValue(newPage);
}

void XtExportDlg::onPreviewZoomChangeRequested(int delta)
{
    int newZoom = m_ui->sliderZoom->value() + delta;
    newZoom = qBound(m_ui->sliderZoom->minimum(), newZoom, m_ui->sliderZoom->maximum());
    m_ui->sliderZoom->setValue(newZoom);
}

void XtExportDlg::onBrowseOutput()
{
    QString currentPath = m_ui->leOutputPath->text();
    QString dir = currentPath.isEmpty() ? QString() : QFileInfo(currentPath).absolutePath();

    QString filter = tr("XT* Files (*.xtc *.xtch);;All Files (*)");
    QString filename = QFileDialog::getSaveFileName(this, tr("Export to..."), dir, filter);

    if (!filename.isEmpty()) {
        m_ui->leOutputPath->setText(filename);
    }
}

void XtExportDlg::onExport()
{
    // TODO (Phase 5): Implement export
    accept();
}

void XtExportDlg::onCancel()
{
    // TODO (Phase 5): Handle cancellation during export
    reject();
}

void XtExportDlg::schedulePreviewUpdate()
{
    // Restart the debounce timer (restarts if already running)
    m_previewDebounceTimer->start();
}

void XtExportDlg::renderPreview()
{
    if (!m_docView) {
        m_previewWidget->showMessage(tr("No document loaded"));
        return;
    }

    m_previewWidget->showMessage(tr("Rendering..."));

    // Create and configure exporter
    XtcExporter exporter;
    configureExporter(exporter);

    // Set preview mode for current page
    exporter.setPreviewPage(m_currentPreviewPage);

    // Execute preview render (no file output in preview mode)
    bool success = exporter.exportDocument(m_docView, static_cast<LVStreamRef>(nullptr));

    if (success) {
        // Get rendered BMP data
        const LVArray<lUInt8>& bmpData = exporter.getPreviewBmp();

        if (bmpData.length() > 0) {
            // Load BMP from memory into QImage
            QImage image;
            if (image.loadFromData(bmpData.ptr(), bmpData.length(), "BMP")) {
                m_previewWidget->setImage(image, m_zoomPercent);
            } else {
                m_previewWidget->showMessage(tr("Failed to decode preview"));
            }
        } else {
            m_previewWidget->showMessage(tr("Preview rendering failed"));
        }

        // Update page count from exporter
        int totalPages = exporter.getLastTotalPageCount();
        if (totalPages > 0) {
            updatePageCount(totalPages);
        }
    } else {
        m_previewWidget->showMessage(tr("Preview rendering failed"));
    }
}

void XtExportDlg::configureExporter(XtcExporter& exporter)
{
    // Format
    XtcExportFormat format = (m_ui->cbFormat->currentIndex() == 0)
                             ? XTC_FORMAT_XTC : XTC_FORMAT_XTCH;
    exporter.setFormat(format);

    // Dimensions
    exporter.setDimensions(static_cast<uint16_t>(m_ui->sbWidth->value()),
                           static_cast<uint16_t>(m_ui->sbHeight->value()));

    // Gray policy (only affects XTC/1-bit)
    GrayToMonoPolicy grayPolicy;
    switch (m_ui->cbGrayPolicy->currentIndex()) {
        case 0: grayPolicy = GRAY_SPLIT_LIGHT_DARK; break;
        case 1: grayPolicy = GRAY_ALL_TO_WHITE; break;
        case 2: grayPolicy = GRAY_ALL_TO_BLACK; break;
        default: grayPolicy = GRAY_SPLIT_LIGHT_DARK; break;
    }
    exporter.setGrayPolicy(grayPolicy);

    // Dither mode
    ImageDitherMode ditherMode = resolveEffectiveDitherMode();
    exporter.setImageDitherMode(ditherMode);

    // Dithering options (for Floyd-Steinberg modes)
    if (ditherMode == IMAGE_DITHER_FS_1BIT || ditherMode == IMAGE_DITHER_FS_2BIT) {
        DitheringOptions opts;
        opts.threshold = static_cast<float>(m_ui->sbThreshold->value());
        opts.errorDiffusion = static_cast<float>(m_ui->sbErrorDiffusion->value());
        opts.gamma = static_cast<float>(m_ui->sbGamma->value());
        opts.serpentine = m_ui->cbSerpentine->isChecked();
        exporter.setDitheringOptions(opts);
    }

    // Chapters (not used in preview, but configure anyway)
    exporter.enableChapters(m_ui->cbChaptersEnabled->isChecked());
    exporter.setChapterDepth(m_ui->cbChapterDepth->currentIndex() + 1);
}

void XtExportDlg::updatePageCount(int totalPages)
{
    if (totalPages == m_totalPageCount)
        return;

    m_totalPageCount = totalPages;
    m_updatingControls = true;

    // Update page count label
    m_ui->lblPageCount->setText(QString("/ %1").arg(totalPages));

    // Update spinbox maximums
    m_ui->sbPreviewPage->setMaximum(totalPages);
    m_ui->sbFromPage->setMaximum(totalPages);
    m_ui->sbToPage->setMaximum(totalPages);
    m_ui->sbToPage->setValue(totalPages);


    // Clamp current preview page if needed
    if (m_currentPreviewPage >= totalPages) {
        m_currentPreviewPage = totalPages - 1;
        m_ui->sbPreviewPage->setValue(m_currentPreviewPage + 1);
    }

    // Clamp From/To page values if needed
    if (m_ui->sbFromPage->value() > totalPages) {
        m_ui->sbFromPage->setValue(totalPages);
    }
    if (m_ui->sbToPage->value() > totalPages) {
        m_ui->sbToPage->setValue(totalPages);
    }

    // Make sure To >= From
    if (m_ui->sbToPage->value() < m_ui->sbFromPage->value()) {
        int from = m_ui->sbFromPage->value();
        int to = m_ui->sbToPage->value();
        m_ui->sbFromPage->setValue(to);
        m_ui->sbToPage->setValue(from);
    }

    m_updatingControls = false;
}

void XtExportDlg::loadZoomSetting()
{
    int zoom = 100;  // default
    if (auto* mainWin = qobject_cast<MainWindow*>(parent())) {
        CRPropRef props = mainWin->getSettings();
        zoom = props->getIntDef("xtexport.preview.zoom", 100);
    }
    zoom = qBound(50, zoom, PreviewWidget::ZOOM_LEVEL_MAX);

    m_zoomPercent = zoom;
    m_ui->sliderZoom->setValue(zoom);
    updateZoomButtonLabel();
}

void XtExportDlg::updateZoomButtonLabel()
{
    // Button shows the zoom level it will jump TO when clicked
    // Find next level higher than current, or wrap to first
    for (int i = 0; i < PreviewWidget::ZOOM_LEVELS_COUNT; ++i) {
        if (m_zoomPercent < PreviewWidget::ZOOM_LEVELS[i]) {
            m_ui->btn200Zoom->setText(QString("%1%").arg(PreviewWidget::ZOOM_LEVELS[i]));
            return;
        }
    }
    // At or above max level, show first level (wraps)
    m_ui->btn200Zoom->setText(QString("%1%").arg(PreviewWidget::ZOOM_LEVELS[0]));
}

void XtExportDlg::saveZoomSetting()
{
    if (auto* mainWin = qobject_cast<MainWindow*>(parent())) {
        CRPropRef props = mainWin->getSettings();
        props->setInt("xtexport.preview.zoom", m_ui->sliderZoom->value());
    }
}
