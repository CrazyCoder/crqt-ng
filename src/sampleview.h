/**
 * crqt-ng
 *
 * (c) crqt-ng authors (See AUTHORS file)
 *
 * This source code is distributed under the terms of
 * GNU General Public License version 2
 * See LICENSE file for details
 */

#ifndef SAMPLEVIEW_H
#define SAMPLEVIEW_H

#include <QWidget>

class CR3View;

class SampleView: public QWidget
{
    Q_OBJECT
public:
    SampleView(QWidget* parent);
    CR3View* creview() {
        return m_view;
    }
    void updatePositionForParent();
private:
    CR3View* m_view;
};

#endif // SAMPLEVIEW_H
