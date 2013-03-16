//
// Copyright (c) 2013 whitehat2k9 <whitehat2k9@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "outputpage.h"
#include "ui_outputpage.h"

#include <QtGui>

OutputPage::OutputPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::OutputPage)
{
    ui->setupUi(this);

    registerField("outputPath*", ui->outputPath);
    connect(ui->browseOutputButton, SIGNAL(clicked()), this, SLOT(browseOutputLocation()));

}

void OutputPage::initializePage() {
    QFileInfo inputInfo(field("inputPath").toString());
    showSaveDir = inputInfo.isDir() && field("batchMode").toBool();
    if(showSaveDir)
        ui->description->setText("Choose a directory to save the torrent files in.");
    else
        ui->description->setText("Choose a name and save location for the torrent file.");
}

void OutputPage::browseOutputLocation() {
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if(!showSaveDir) {
        dialog.setDefaultSuffix("torrent");
        dialog.setNameFilter("Torrent file (*.torrent)");
    }
    else {
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
    }
    if(dialog.exec())
        ui->outputPath->setText(dialog.selectedFiles().first());
}

OutputPage::~OutputPage()
{
    delete ui;
}
