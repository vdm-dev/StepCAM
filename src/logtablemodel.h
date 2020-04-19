//
// This file is part of StepCAM 2.
// Project URL: https://github.com/vdm-dev/StepCAM
// Copyright (c) 2020  Dmitry Lavygin (vdm.inbox@gmail.com).
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


#ifndef LOGTABLEMODEL_H
#define LOGTABLEMODEL_H


#include <QAbstractTableModel>
#include <QList>
#include <QPixmap>

#include "logitem.h"


class QAction;


class LogTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LogTableModel(QObject* parent = nullptr);

    virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    void add(int severity, const QString& description, const QString& file = QString(),
        const QString& line = QString());

    void error(const QString& description, const QString& file = QString(),
        const QString& line = QString());
    void warning(const QString& description, const QString& file = QString(),
        const QString& line = QString());
    void notice(const QString& description, const QString& file = QString(),
        const QString& line = QString());
    void accept(const QString& description, const QString& file = QString(),
        const QString& line = QString());

    void remove(const QString& file);

signals:
    void updated(int errors, int warnings, int notices, int accepts);

public slots:
    void refresh();
    void clear();

private:
    QList<LogItem> _items;

    QPixmap _pixmapAccept;
    QPixmap _pixmapNotice;
    QPixmap _pixmapWarning;
    QPixmap _pixmapError;

    int _errors;
    int _warnings;
    int _notices;
    int _accepts;
};


inline void LogTableModel::error(const QString& description, const QString& file,
    const QString& line)
{
    add(LogItem::SeverityError, description, file, line);
}

inline void LogTableModel::warning(const QString& description, const QString& file,
    const QString& line)
{
    add(LogItem::SeverityWarning, description, file, line);
}

inline void LogTableModel::notice(const QString& description, const QString& file,
    const QString& line)
{
    add(LogItem::SeverityNotice, description, file, line);
}

inline void LogTableModel::accept(const QString& description, const QString& file,
    const QString& line)
{
    add(LogItem::SeverityAccept, description, file, line);
}


#endif // LOGTABLEMODEL_H
