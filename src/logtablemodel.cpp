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


#include "logtablemodel.h"

#include <QSize>
#include <QMutableListIterator>


LogTableModel::LogTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , _errors(0)
    , _warnings(0)
    , _notices(0)
    , _accepts(0)
{
    _pixmapAccept.load(QString(":/img/accept.png"));
    _pixmapNotice.load(QString(":/img/notice.png"));
    _pixmapWarning.load(QString(":/img/warning.png"));
    _pixmapError.load(QString(":/img/error.png"));
}

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (section)
    {
    // Severity
    case 0:
        if (role == Qt::ToolTipRole)
            return tr("Severity");

        break;

    // Order
    case 1:
        if (role == Qt::ToolTipRole)
            return tr("Order");

        break;

    // Description
    case 2:
        if (role == Qt::DisplayRole)
            return tr("Description");

        break;

    // File
    case 3:
        if (role == Qt::DisplayRole)
            return tr("File");

        break;

    // Line
    case 4:
        if (role == Qt::DisplayRole)
            return tr("Line");

        break;

    default:
        break;
    }


    return QVariant();
}

int LogTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return _items.size();
}

int LogTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return 5;
}

QVariant LogTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _items.size())
        return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        return (index.column() < 2 || index.column() == 4) ?
            Qt::AlignCenter : QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    // Severity
    if (index.column() == 0)
    {
        if (role == Qt::DecorationRole)
        {
            switch (_items[index.row()].severity)
            {
            case LogItem::SeverityAccept:
                return _pixmapAccept;
            case LogItem::SeverityNotice:
                return _pixmapNotice;
            case LogItem::SeverityWarning:
                return _pixmapWarning;
            case LogItem::SeverityError:
                return _pixmapError;
            case LogItem::SeverityNone:
            default:
                break;
            }
        }
        else if (role == Qt::UserRole)
        {
            return _items[index.row()].severity;
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        // Order
        case 1:
            return (_items[index.row()].order > 0) ? _items[index.row()].order : QVariant();

        // Description
        case 2:
            return _items[index.row()].description;

        // File
        case 3:
            return _items[index.row()].file;

        // Line
        case 4:
            return _items[index.row()].line;

        default:
            break;
        }
    }

    return QVariant();
}

void LogTableModel::add(int severity, const QString& description, const QString& file,
    const QString& line)
{
    LogItem item;

    switch (severity)
    {
    case LogItem::SeverityError:
        _errors++;
        item.order = _errors;
        break;
    case LogItem::SeverityWarning:
        _warnings++;
        item.order = _warnings;
        break;
    case LogItem::SeverityNotice:
        _notices++;
        item.order = _notices;
        break;
    case LogItem::SeverityAccept:
        _accepts++;
        item.order = _accepts;
        break;
    default:
        item.order = 0;
        break;
    }

    item.severity = severity;
    item.description = description;
    item.file = file;
    item.line = line;

    beginResetModel();
    _items << item;
    endResetModel();
    emit updated(_errors, _warnings, _notices, _accepts);
}

void LogTableModel::remove(const QString& file)
{
    QMutableListIterator<LogItem> iterator(_items);

    bool changed = false;

    while (iterator.hasNext())
    {
        LogItem& item = iterator.next();

        if (item.file == file)
        {
            switch (item.severity)
            {
            case LogItem::SeverityError:
                _errors--;
                break;
            case LogItem::SeverityWarning:
                _warnings--;
                break;
            case LogItem::SeverityNotice:
                _notices--;
                break;
            case LogItem::SeverityAccept:
                _accepts--;
                break;
            default:
                break;
            }

            changed = true;
            iterator.remove();
        }
    }

    if (changed)
        refresh();
}

void LogTableModel::refresh()
{
    beginResetModel();
    endResetModel();
    emit updated(_errors, _warnings, _notices, _accepts);
}

void LogTableModel::clear()
{
    beginResetModel();
    _items.clear();
    _errors = 0;
    _warnings = 0;
    _notices = 0;
    _accepts = 0;
    endResetModel();
    emit updated(_errors, _warnings, _notices, _accepts);
}
