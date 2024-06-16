//
// This file is part of StepCAM 2.
// Project URL: https://github.com/vdm-dev/StepCAM
// Copyright (c) 2024  Dmitry Lavygin (vdm.inbox@gmail.com).
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


#ifndef PROGRESSSTATUSWIDGET_H
#define PROGRESSSTATUSWIDGET_H


#include "ui_progressstatuswidget.h"


class ProgressStatusWidget : public QWidget, private Ui::ProgressStatusWidget
{
    Q_OBJECT

public:
    explicit ProgressStatusWidget(QWidget* parent = nullptr);

signals:
    void canceled();

public slots:
    void reset();
    void setRange(int minimum, int maximum);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setText(const QString& text);
    void setTextVisible(bool visible);
    void setProgressVisible(bool visible);
    void setValue(int value);
    void setProgress(int done, int total);
};


#endif // PROGRESSSTATUSWIDGET_H
