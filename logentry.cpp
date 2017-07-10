#include "logentry.h"
#include <QDateTime>

LogEntry::LogEntry()
{

}

void LogEntry::load(const QString& _data, const QStringList& fields)
{
    done = false;
    downloaded = false;
    work = fields[0];
    event = fields[1];
    type = fields[2];
    name = fields[3];
    user = fields[4];
    company = fields[5];
    hour = fields[6];
    path = fields[7];
    file = fields[8];
    date = _data;
    epochTime = getEpochTime(date, hour);
}

qint64 LogEntry::getEpochTime(const QString& date, const QString& time)
{
    if(date.length() < 10 || time.length() < 8)
        return -1;

    QStringList splitedDate = date.split("/");
    int year = splitedDate[2].toInt();
    int month = splitedDate[1].toInt();
    int day = splitedDate[0].toInt();

    QStringList splitedTime = time.split(":");
    int hour = splitedTime[0].toInt();
    int minute = splitedTime[1].toInt();
    int second = splitedTime[2].toInt();

    QDateTime dateTime(QDate(year, month, day), QTime(hour, minute, second));
    return dateTime.toMSecsSinceEpoch();
}
