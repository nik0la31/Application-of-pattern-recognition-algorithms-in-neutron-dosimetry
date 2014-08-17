#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <map>
#include <memory>
#include <project.h>
#include <vector>
#include <QDir>
#include <QFileInfo>
#include <QModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include "projectparser.h"
#include "projectitem.h"

class Workspace
{
public:

    static Workspace Instance;

    Workspace()
    {

    }

    ~Workspace()
    {
        // unload projects
    }

    QStandardItemModel* GetProjectsModel();

    void AddProject(QString& projectFullPath, bool load = false);

    void AddDocument(std::wstring& proj, QString& imagePath);

    void SetCurrent(std::wstring& projectName, std::wstring documentName = L"");

    cv::Mat GetImage(ImageType imgType, ShapeType shapeType, bool fill, bool edge, bool center);

    bool IsImageAvalilable();

private:
    // Projects.
    std::map<std::wstring, std::unique_ptr<Project> > m_Projects;

    // Projects tree model view.
    QStandardItemModel m_ProjectsModel;

    // Keep references to currently selected items for easier access.
    Project*     m_CurrentProject = nullptr;
    ProjectItem* m_CurrentProjectItem  = nullptr;
    Document*    m_CurrentDocument  = nullptr;
    ProjectItem* m_CurrentDocumentItem = nullptr;

    void SetCurrent(
            Project* project,
            ProjectItem* projectItem,
            Document* document,
            ProjectItem* documentItem);
};

#endif // WORKSPACE_H
