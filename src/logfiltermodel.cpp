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


#include "logfiltermodel.h"

#include <QAction>

#include "logitem.h"


LogFilterModel::LogFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , _actionErrors(nullptr)
    , _actionWarnings(nullptr)
    , _actionNotices(nullptr)
{
}

void LogFilterModel::setFilterAction(int severity, QAction* action)
{
    switch (severity)
    {
    case LogItem::SeverityError:
        _actionErrors = action;
        break;
    case LogItem::SeverityWarning:
        _actionWarnings = action;
        break;
    case LogItem::SeverityNotice:
        _actionNotices = action;
        break;
    default:
        break;
    }
}

bool LogFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    int severity = sourceModel()->data(index, Qt::UserRole).toInt();

    switch (severity)
    {
    case LogItem::SeverityError:
        if (_actionErrors && !_actionErrors->isChecked())
            return false;

        break;

    case LogItem::SeverityWarning:
        if (_actionWarnings && !_actionWarnings->isChecked())
            return false;

        break;

    case LogItem::SeverityNotice:
        if (_actionNotices && !_actionNotices->isChecked())
            return false;

        break;

    default:
        break;
    }

    return true;
}
