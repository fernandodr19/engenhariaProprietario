#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QString>
#include <QSettings>

class LogEntry
{
public:
    LogEntry();

//    void save(QSettings *settings);
//    LogEntry load(QSettings *settings);

    bool feito;
    QString obra;
    QString evento;
    QString tipo;
    QString nome;
    QString usuario;
    QString empresa;
    QString hora;
    QString caminho;
    QString arquivo;
    QString data;
    qint64 epochTime;
};

#endif // LOGENTRY_H
