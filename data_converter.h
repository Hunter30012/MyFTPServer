#ifndef DATA_CONVERTER_H
#define DATA_CONVERTER_H

#include <QString>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPixmap>

class DataConverter
{
public:
    static QString ByteArrayToString(const QByteArray& data);
    static QByteArray JsonArrayToByteArray(const QJsonArray& json);
    static QByteArray JsonObjectToByteArray(const QJsonObject& json);
    static QPixmap decodePixmapFromString(const QString& icon);

private:
    DataConverter();
};

#endif // DATA_CONVERTER_H
