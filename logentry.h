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

    QString forwarded;
    bool downloaded;
    bool commented;
    QString work;
    QString event;
    QString type;
    QString name;
    QString user;
    QString company;
    QString hour;
    QString path;
    QString file;
    QString date;
    qint64 epochTime;
};

#endif // LOGENTRY_H
