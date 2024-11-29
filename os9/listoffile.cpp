#include "listoffile.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ListOfFile::ListOfFile() {}

file& ListOfFile::operator[](size_t a){
    return list[a];
}
size_t ListOfFile::size() {
    return list.size();
}
size_t ListOfFile::getSizeOfList(){
    return sizeOfList;
}
std::vector<file> ListOfFile::getlist(){
    return list;
}

void ListOfFile::getlist(char* msg) {

    list.clear();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray(msg));

    // Отримання JSON-об'єкта
    QJsonObject jsonObj = jsonDoc.object();

    // Зчитуємо total_size
    if (jsonObj.contains("total_size")) {
        sizeOfList = jsonObj["total_size"].toInt();
    }

    // Зчитуємо масив files
    if (jsonObj.contains("files") && jsonObj["files"].isArray()) {
        QJsonArray filesArray = jsonObj["files"].toArray();

        // Проходимо по всім елементам масиву
        for (const QJsonValue& fileValue : filesArray) {
            if (!fileValue.isObject()) continue;

            QJsonObject fileObj = fileValue.toObject();

            QString name = fileObj["name"].toString();
            QString creationTime = fileObj["creation_time"].toString();
            int size = fileObj["size"].toInt(); // Зчитуємо поле size

            // Додаємо до списку (додаємо і розмір, якщо потрібно)
            list.push_back(file(name.toStdString(), creationTime.toStdString(),size));
        }
    } else {
        //qWarning() << "Масив 'files' відсутній або має неправильний формат.";
    }
}



