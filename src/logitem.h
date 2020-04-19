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


#ifndef LOGITEM_H
#define LOGITEM_H


#include <QString>


class LogItem
{
public:
    enum Severity
    {
        SeverityNone = 0,
        SeverityAccept = 1,
        SeverityNotice = 2,
        SeverityWarning = 3,
        SeverityError = 4
    };

    LogItem()
        : severity(SeverityNone)
        , order(0)
    {
    }

    int severity;
    int order;
    QString description;
    QString file;
    QString line;
};


#endif // LOGITEM_H
