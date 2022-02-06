/**
 * crqt-ng
 *
 * (c) crqt-ng authors (See AUTHORS file)
 *
 * This source code is distributed under the terms of
 * GNU General Public License version 2
 * See LICENSE file for details
 */

#include "sampleview.h"

#include <QVBoxLayout>
#include <QCloseEvent>

#include "cr3widget.h"

SampleView::SampleView(QWidget* parent)
        : QWidget(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Style Preview"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_view = new CR3View(this);
    layout->addWidget(m_view, 10);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMinimumSize(300, 150);
    updatePositionForParent();
}

void SampleView::updatePositionForParent() {
    QPoint parentPos = parentWidget()->pos();
    QSize parentFrameSize = parentWidget()->frameSize();
    move(parentPos.x() + parentFrameSize.width(), parentPos.y() + 40);
}
