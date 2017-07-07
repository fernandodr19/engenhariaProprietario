#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QString>
#include <QSettings>

class LogEntry
{
public:
    LogEntry();

//    void save(QSettings *settings);
    void load(const QString& _data, const QStringList& fields);
    static qint64 getEpochTime(const QString& date, const QString& time);

    bool feito;
    bool downloaded;
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
