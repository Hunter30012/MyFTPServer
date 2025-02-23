#include "data_converter.h"

DataConverter::DataConverter() {}


QString DataConverter::ByteArrayToString(const QByteArray& data)
{
    // return QString::fromUtf8(data);
    return QString::fromStdString(data.toStdString());
}

QByteArray DataConverter::JsonArrayToByteArray(const QJsonArray& json)
{
    QJsonDocument doc(json);
    // convert to Json file in ByteArray
    return doc.toJson(QJsonDocument::Compact); // delete space
}

QByteArray DataConverter::JsonObjectToByteArray(const QJsonObject& json)
{
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

QPixmap DataConverter::decodePixmapFromString(const QString& icon) {
    auto const encoded = icon.toLatin1();
    QPixmap p;
    p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
    return p;
}
