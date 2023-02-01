/***************************************************************************
 *   crqt-ng                                                               *
 *   Copyright (C) 2023 Aleksey Chernov <valexlin@gmail.com>               *
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

#ifndef TABSCOLLECTION_H
#define TABSCOLLECTION_H

#include <QtCore/QVector>

#include <lvstring.h>
#include <crhist.h>

#include "tabdata.h"

class TabsCollection: public QVector<TabData>
{
public:
    TabsCollection();
    virtual ~TabsCollection();
    bool saveTabSession(const QString& filename);
    static bool openTabSession(QStringList& files, const QString& filename);
    /// load history from file
    bool loadHistory(const QString& filename);
    /// save history to file
    bool saveHistory(const QString& filename);
    /// save history to file
    bool saveHistory() {
        return saveHistory(QString());
    }
    CRFileHist* getHistory() {
        return &m_hist;
    }
    int indexByViewId(lUInt64 viewId) const;
    void cleanup();
private:
    CRFileHist m_hist;
    lString32 m_historyFileName;
};

#endif // TABSCOLLECTION_H
