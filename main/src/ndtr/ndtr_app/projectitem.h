#ifndef PROJECTITEM_H
#define PROJECTITEM_H

#include <QStandardItem>

class ProjectItem : public QStandardItem
{
public:
    ProjectItem(std::wstring& name, bool isDoc = false);

    std::wstring GetName();

    bool IsDocument();

    bool IsSelected();

    void Select();

    void Unselect();

private:
    std::wstring m_Name;

    bool m_IsDocument;
    bool m_IsSelected;

    static QBrush c_ProjectBrush;
    static QBrush c_DocumentBrush;
    static QBrush c_DefaultBrush;
};

#endif // PROJECTITEM_H
