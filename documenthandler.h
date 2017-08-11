#ifndef DOCUMENTHANDLER_H
#define DOCUMENTHANDLER_H


class QString;
class QStringList;

class DocumentHandler
{
public:
    DocumentHandler();

    void exportExcelFile(const QString& fileName, const QStringList& cells);
};

#endif // DOCUMENTHANDLER_H
