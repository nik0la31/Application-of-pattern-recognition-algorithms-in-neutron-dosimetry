#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <string>

class Utils
{
public:

    static std::string StringQ2W(QString& str)
    {
        return str.toStdString();
    }

    static QString StringW2Q(std::string& str)
    {
        return QString(str.c_str());
    }
};

#endif // UTILS_H
