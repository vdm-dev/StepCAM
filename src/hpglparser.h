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


#ifndef HPGLPARSER_H
#define HPGLPARSER_H


#include "abstractparser.h"


class HpglParser : public AbstractParser
{
    Q_OBJECT

public:
    explicit HpglParser(QObject* parent = nullptr);

    virtual ParserType type() const;

    virtual void clear();
    virtual bool parse(QFile& file);

    virtual const QMap<int, AbstractTool>& tools() const;
    virtual const QList<AbstractCurve>& curves() const;

public slots:
    virtual void interrupt();

private:
    QMap<int, AbstractTool> _tools;
    QList<AbstractCurve> _curves;

    bool _toolIsUp;
};


inline AbstractParser::ParserType HpglParser::type() const
{
    return ParserMillling;
}

inline const QMap<int, AbstractTool>& HpglParser::tools() const
{
    return _tools;
}

inline const QList<AbstractCurve>& HpglParser::curves() const
{
    return _curves;
}


#endif // HPGLPARSER_H
