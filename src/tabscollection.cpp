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

/*
 * This file contains code snippets from cr3widget.cpp
 * Copyright (C) 2009-2012,2014 Vadim Lopatin <coolreader.org@gmail.com>
 */

#include "tabscollection.h"
#include "cr3widget.h"
#include "crqtutil.h"

#include <QtCore/QSettings>

#include <lvdocview.h>
#include <lvstreamutils.h>
#include <crlog.h>

TabsCollection::TabsCollection() : QVector<TabData>(), m_props(LVCreatePropsContainer()) { }

TabsCollection::~TabsCollection() { }

TabsCollection::TabSession TabsCollection::openTabSession(const QString& filename, bool* ok) {
    TabSession session;
    TabProperty data;
    m_sessionFileName = filename;
    QSettings settings(filename, QSettings::IniFormat);
    int size = settings.beginReadArray("tabs");
    session.clear();
    session.reserve(size);
    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        data.docPath = settings.value("doc-filename").toString();
        data.title = settings.value("title").toString();
        session.append(data);
    }
    settings.endArray();
    session.currentDocument = settings.value("current").toString();
    if (ok)
        *ok = (QSettings::NoError == settings.status());
    return session;
}

bool TabsCollection::saveTabSession(const QString& filename) {
    QString fn = !filename.isEmpty() ? filename : m_sessionFileName;
    QSettings settings(fn, QSettings::IniFormat);
    settings.clear();
    settings.beginWriteArray("tabs");
    for (int i = 0; i < QVector<TabData>::size(); i++) {
        const TabData& tab = at(i);
        CR3View* view = tab.view();
        if (NULL != view)
            view->getDocView()->savePosition();
        settings.setArrayIndex(i);
        settings.setValue("doc-filename", tab.docPath());
        settings.setValue("title", tab.title());
    }
    settings.endArray();
    settings.setValue("current", m_currentDocument);
    settings.sync();
    return QSettings::NoError == settings.status();
}

bool TabsCollection::loadSettings(const QString& filename) {
    lString32 fn(qt2cr(filename));
    m_settingsFileName = fn;
    LVStreamRef stream = LVOpenFileStream(fn.c_str(), LVOM_READ);
    bool res = false;
    if (!stream.isNull() && m_props->loadFromStream(stream.get())) {
        CRLog::info("Loading settings from file %s", LCSTR(fn));
        res = true;
    } else {
        CRLog::error("Cannot load settings from file %s", LCSTR(fn));
    }
    //updateDefProps();
    return res;
}

bool TabsCollection::saveSettings(const QString& filename) {
    lString32 fn(qt2cr(filename));
    if (fn.empty())
        fn = m_settingsFileName;
    if (fn.empty())
        return false;
    m_settingsFileName = fn;
    LVStreamRef stream = LVOpenFileStream(fn.c_str(), LVOM_WRITE);
    if (!stream) {
        lString32 upath = LVExtractPath(fn);
        lString8 path = UnicodeToUtf8(upath);
        if (!LVCreateDirectory(upath)) {
            CRLog::error("Cannot create directory %s", path.c_str());
        } else {
            stream = LVOpenFileStream(fn.c_str(), LVOM_WRITE);
        }
    }
    if (stream.isNull()) {
        CRLog::error("Cannot save settings to file %s", LCSTR(fn));
        return false;
    }
    return m_props->saveToStream(stream.get());
}

void TabsCollection::setSettings(CRPropRef props) {
    CRPropRef changed = m_props ^ props;
    // Don't create new props reference, but change existing
    m_props->set(changed | m_props);
}

void TabsCollection::saveWindowPos(QWidget* window, const char* prefix) {
    ::saveWindowPosition(window, m_props, prefix);
}

void TabsCollection::restoreWindowPos(QWidget* window, const char* prefix, bool allowExtraStates) {
    ::restoreWindowPosition(window, m_props, prefix, allowExtraStates);
}

bool TabsCollection::loadHistory(const QString& filename) {
    lString32 fn(qt2cr(filename));
    CRLog::trace("TabsCollection::loadHistory( %s )", UnicodeToUtf8(fn).c_str());
    m_historyFileName = fn;
    LVStreamRef stream = LVOpenFileStream(fn.c_str(), LVOM_READ);
    if (stream.isNull()) {
        return false;
    }
    if (!m_hist.loadFromStream(stream))
        return false;
    return true;
}

bool TabsCollection::saveHistory(const QString& filename) {
    lString32 fn(qt2cr(filename));
    if (fn.empty())
        fn = m_historyFileName;
    if (fn.empty()) {
        CRLog::info("Cannot write history file - no file name specified");
        return false;
    }
    m_historyFileName = fn;
    CRLog::trace("TabsCollection::saveHistory(): filename: %s", LCSTR(fn));
    LVStreamRef stream = LVOpenFileStream(fn.c_str(), LVOM_WRITE);
    if (!stream) {
        lString32 upath = LVExtractPath(fn);
        lString8 path = UnicodeToUtf8(upath);
        if (!LVCreateDirectory(upath)) {
            CRLog::error("Cannot create directory %s", path.c_str());
        } else {
            stream = LVOpenFileStream(fn.c_str(), LVOM_WRITE);
        }
    }
    if (stream.isNull()) {
        CRLog::error("Error while creating history file %s - position will be lost", LCSTR(fn));
        return false;
    }
    for (TabsCollection::iterator it = begin(); it != end(); ++it) {
        CR3View* view = (*it).view();
        if (NULL != view) {
            view->getDocView()->savePosition();
        }
    }
    return m_hist.saveToStream(stream.get());
}

int TabsCollection::indexByViewId(lUInt64 viewId) const {
    for (int i = 0; i < size(); i++) {
        const TabData& tab = at(i);
        const CR3View* view = tab.view();
        if (NULL != view) {
            if (view->id() == viewId) {
                return i;
            }
        }
    }
    return -1;
}

void TabsCollection::cleanup() {
    for (TabsCollection::iterator it = begin(); it != end(); ++it) {
        (*it).cleanup();
    }
    clear();
}
