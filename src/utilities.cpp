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


#include "utilities.h"


QString Utilities::coordinateToString(qint64 coordinate, bool trim)
{
    bool negative = false;

    if (coordinate < 0)
    {
        negative = true;
        coordinate = -coordinate;
    }

    QString result = QString::number(coordinate % 1000);

    while (result.size() < 3)
        result.prepend('0');

    while (trim && !result.isEmpty() && result.at(result.size() - 1) == '0')
        result.chop(1);

    if (result.isEmpty())
    {
        result = QString::number(coordinate / 1000);
    }
    else
    {
        result.prepend(QString::number(coordinate / 1000) + '.');
    }

    if (negative)
        result.prepend('-');

    return result;
}

QString Utilities::doubleToString(double value, int precision, bool trim)
{
    QString result = QString::number(value, 'f', precision);

    if (trim && result.indexOf('.') > -1)
    {
        while (result.at(result.size() - 1) == '0')
            result.chop(1);

        if (result.at(result.size() - 1) == '.')
            result.chop(1);
    }

    return result;
}
