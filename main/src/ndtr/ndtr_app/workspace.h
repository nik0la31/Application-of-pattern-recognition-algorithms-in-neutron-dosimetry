#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "stats.h"

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
#include "viewoptions.h"
#include "processingoptions.h"

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

    void AddDocument(std::string& proj, QString& imagePath, ProcessingOptions& options);

    void SetCurrent(std::string& projectName, std::string documentName = "");

    cv::Mat GetImage(ViewOptions view);

    Stats GetStats();

    bool IsImageAvalilable();

    Document* GetCurrentDocument();

    Project* GetCurrentProject();

    void SaveCurrentProject();

    void SaveAllProjects();

    void RemoveCurrentDocument();

    void Update(ProcessingOptions& options);

    bool IsCurrentProjectPersistent();

    bool AreAllProjectsPersistent();

    void CloseCurrentProject();

    void CloseAllProjects();

    void ExportTraces(std::string& filePath);

private:
    // Projects.
    std::map<std::string, std::unique_ptr<Project> > m_Projects;
    std::map<std::string, bool> m_ProjectsPersistentState;

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
