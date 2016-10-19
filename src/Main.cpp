#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QtWidgets>
#include <QtWebEngineWidgets>
#include <QStyle>

#include "includes/SWidgets/SWebView.hpp"
#include "includes/SWidgets/STabWidget.hpp"
#include "includes/SMainWindow.hpp"
#include "includes/SStarter.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

//	QMessageBox::information(nulthislptr, "DEBUG", SMainWindow::dataPath);

	// cr�e un objet "langue fran�aise" et l'affecte a la place de l'objet par d�faut du syst�me
	QLocale france(QLocale::French, QLocale::France);
	QString locale = france.name();
	QTranslator translator;
	QMessageBox::information(nullptr, "DEBUG", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	translator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&translator);

    SStarter *starter{ new SStarter };
    delete starter;

    return app.exec();
}
