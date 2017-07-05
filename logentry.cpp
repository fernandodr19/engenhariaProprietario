#include "logentry.h"
#include <QDateTime>

LogEntry::LogEntry()
{

}

void LogEntry::load(const QString& _data, const QStringList& fields)
{
    feito = false;
    obra = fields[0];
    evento = fields[1];
    tipo = fields[2];
    nome = fields[3];
    usuario = fields[4];
    empresa = fields[5];
    hora = fields[6];
    caminho = fields[7];
    arquivo = fields[8];
    data = _data;
    epochTime = getEpochTime(data, hora);
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
