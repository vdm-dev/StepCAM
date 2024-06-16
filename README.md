# StepCAM 2
The StepCAM 2 application is a simple PCB format converter software. It converts the Excellon and HP-GL formats to G-code.

The application was written under the influence of StepCAM 1.79 by Sergey Efremov. The name **StepCAM** is saved with his permission.

#### Distinctive features of the new version
* Modern interface with separation of milling and drilling parameters.
* Automatic detection of number format in Excellon files regardless of Sprint-Layout export settings.
* Number conversion using integer arithmetic only (no accuracy loss).
* It is possible to automatically add arbitrary prologue and epilogue to the program code.
* High speed of conversion.
* Log of errors and warnings related to input data analysis.
* The program is written in C++ using the [Qt framework](https://www.qt.io/) and can be built for Windows, Linux and Mac OS X platforms.
* The source code of the program is distributed under the terms of the [GNU General Public License 3](https://www.gnu.org/licenses/gpl-3.0.html).

#### Lost features of the new version
* Conversion of Sprint-Layout export files only.
* Lack of built-in visualizer.
* **(Temporary)** Absence of the conversion process indicator.
* **(Temporary)** Lack of Russian localization.

## Screenshot
![StepCAM 2](https://github.com/vdm-dev/StepCAM/raw/master/screenshot.png)

## Licensing
Copyright &copy; 2024  Dmitry Lavygin (vdm.inbox@gmail.com).

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

The information about licenses for images can be found in [img/README.md](https://github.com/vdm-dev/StepCAM/raw/master/img/README.md).

## Donate
If you found my program useful, you can thank me with donations via

[![PayPal](https://github.com/vdm-dev/StepCAM/raw/master/img/donate.png)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=vdm.inbox@gmail.com&lc=RU&item_name=StepCAM2&no_note=0&currency_code=RUB&bn=PP-DonationsBF:btn_donateCC_LG.gif:NonHosted) [![Yandex.Money](https://github.com/vdm-dev/StepCAM/raw/master/img/donate_yandex.png)](https://money.yandex.ru/to/4100111653323774)
