#include "documenthandler.h"
#include <QFile>
#include <QTextStream>

DocumentHandler::DocumentHandler()
{

}

void DocumentHandler::exportExcelFile(const QString &fileName, const QStringList& cells)
{
    QFile file(fileName);
    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        stream << "sep=\t\n";
        for(const QString& cell : cells)
            stream << cell;
    }
    file.close();
}
