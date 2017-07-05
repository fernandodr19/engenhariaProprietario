#include "mainwindow.h"
#include "database.h"
#include <QApplication>

QSettings *g_settings = nullptr;
Database *g_database = nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Fluxo Engenharia");
    QCoreApplication::setOrganizationDomain("fluxoengenharia.com.br");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setApplicationName("Controle EP");

    g_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());

    g_database = new Database();
    g_database->load();

    MainWindow w;
    w.show();

    int ret = a.exec();

    g_database->save();
    delete g_database;
    delete g_settings;

    return ret;
}
