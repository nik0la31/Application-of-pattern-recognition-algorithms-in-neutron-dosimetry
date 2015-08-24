#include "projectitem.h"

#include "utils.h"

using namespace std;

// Define background brushes.
QBrush ProjectItem::c_ProjectBrush = QBrush(QColor::fromRgb(204, 204, 255));
QBrush ProjectItem::c_DocumentBrush = QBrush(QColor::fromRgb(235, 235, 255));
QBrush ProjectItem::c_DefaultBrush = QBrush(QColor::fromRgb(255, 255, 255));

ProjectItem::ProjectItem(std::string name, bool isDoc)
    : QStandardItem(Utils::StringW2Q(name)), m_Name(name), m_IsDocument(isDoc)
{

}

string ProjectItem::GetName()
{
    return m_Name;
}

bool ProjectItem::IsDocument()
{
    return m_IsDocument;
}

bool ProjectItem::IsSelected()
{
    return m_IsSelected;
}

void ProjectItem::Select()
{
    m_IsSelected = true;

    if (m_IsDocument)
    {
        this->setBackground(c_DocumentBrush);
    }
    else
    {
        this->setBackground(c_ProjectBrush);
    }
}

void ProjectItem::Unselect()
{
    m_IsSelected = false;

    this->setBackground(c_DefaultBrush);
}
