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


#include "progressstatuswidget.h"


ProgressStatusWidget::ProgressStatusWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    connect(_button, SIGNAL(clicked(bool)), this, SIGNAL(canceled()));
}

void ProgressStatusWidget::reset()
{
    _progressBar->reset();
}

void ProgressStatusWidget::setRange(int minimum, int maximum)
{
    _progressBar->setRange(minimum, maximum);
}

void ProgressStatusWidget::setMinimum(int minimum)
{
    _progressBar->setMinimum(minimum);
}

void ProgressStatusWidget::setMaximum(int maximum)
{
    _progressBar->setMaximum(maximum);
}

void ProgressStatusWidget::setText(const QString& text)
{
    _label->setText(text);
}

void ProgressStatusWidget::setTextVisible(bool visible)
{
    _label->setVisible(visible);
}

void ProgressStatusWidget::setProgressVisible(bool visible)
{
    _progressBar->setTextVisible(visible);
}

void ProgressStatusWidget::setValue(int value)
{
    _progressBar->setValue(value);
}

void ProgressStatusWidget::setProgress(int done, int total)
{
    _progressBar->setMaximum(total);
    _progressBar->setValue(done);
}
