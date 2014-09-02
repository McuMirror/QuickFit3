/*
    Copyright (c) 2008-2014 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg

    last modification: $LastChangedDate$  (revision $Rev$)

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SubPluginLinkDialog_H
#define SubPluginLinkDialog_H

#include <QDialog>

namespace Ui {
    class SubPluginLinkDialog;
}

class SubPluginLinkDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit SubPluginLinkDialog(QWidget *parent = 0);
        ~SubPluginLinkDialog();

        QString insertText() const;

    private:
        Ui::SubPluginLinkDialog *ui;
};

#endif // SubPluginLinkDialog_H