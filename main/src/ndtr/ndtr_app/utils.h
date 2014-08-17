#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <string>

class Utils
{
public:

    static std::wstring StringQ2W(QString& str)
    {
        return str.toStdWString();
    }

    static QString StringW2Q(std::wstring& str)
    {
        return QString::fromWCharArray(str.c_str());
    }
};

#endif // UTILS_H
