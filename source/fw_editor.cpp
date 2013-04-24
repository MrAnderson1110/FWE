////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2013, Black Phoenix
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///   - Redistributions of source code must retain the above copyright
///     notice, this list of conditions and the following disclaimer.
///   - Redistributions in binary form must reproduce the above copyright
///     notice, this list of conditions and the following disclaimer in the
///     documentation and/or other materials provided with the distribution.
///   - Neither the name of the author nor the names of the contributors may
///     be used to endorse or promote products derived from this software without
///     specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
/// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
/// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
/// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////
#include <QApplication>
#include <QFile>
//#include <QCleanlooksStyle>

#include "fw_editor.h"
#include "fwe_main.h"

/// Stores current FWE flags
int fw_editor_flags = 0;

/// FoxWorks application
QApplication* fw_application;

/// FoxWorks main window
MainWindow* fw_mainWindow;


////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize FoxWorks Editor with flags
////////////////////////////////////////////////////////////////////////////////
void fw_editor_initialize(int flags, int argc, char *argv[]) {
	fw_editor_flags = flags;

	if (flags & FOXWORKS_EDITOR_STANDALONE) {
		fw_application = new QApplication(argc,argv);
		Q_INIT_RESOURCE(resources);
		//QApplication::setGraphicsSystem(QString("opengl"));
		fw_mainWindow = new MainWindow(); //Show main window
		fw_mainWindow->show();

		//Load default stylesheet
		QFile stylesheet(":/stylesheet.qss");
		stylesheet.open(QFile::ReadOnly);
		fw_application->setStyleSheet(QLatin1String(stylesheet.readAll()));

		//fw_application->setStyle(new QCleanlooksStyle);
	}

	if (flags & FOXWORKS_EDITOR_BLOCKING) {
		fw_application->exec();
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Shutdown and clean up all resources
////////////////////////////////////////////////////////////////////////////////
void fw_editor_deinitialize() {
	delete fw_application;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Call from main application OpenGL rendering frame if not standalone
////////////////////////////////////////////////////////////////////////////////
void fw_editor_frame() {

}


////////////////////////////////////////////////////////////////////////////////
/// @brief Main call (used when compiling standalone)
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
	fw_editor_initialize(FOXWORKS_EDITOR_STANDALONE | FOXWORKS_EDITOR_BLOCKING,argc,argv);
}