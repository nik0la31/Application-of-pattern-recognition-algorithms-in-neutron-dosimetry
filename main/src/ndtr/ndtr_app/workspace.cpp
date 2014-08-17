#include "workspace.h"

#include <exception>
#include "projectparser.h"
#include "utils.h"

using namespace cv;
using namespace std;

Workspace Workspace::Instance;

QStandardItemModel* Workspace::GetProjectsModel()
{
    return &m_ProjectsModel;
}

void Workspace::AddProject(QString& projectFullPath, bool load)
{
    QFileInfo projectFileInfo(projectFullPath);

    // Extract project name from path.
    QString projectNameQ = projectFileInfo.fileName().remove(".ndtr");
    std::wstring projectName = Utils::StringQ2W(projectNameQ);

    // If not exists, create dir to store images.
    QDir projectDir = projectFileInfo.absoluteDir();
    // Create dir if not exist.
    if (!projectDir.exists(projectNameQ))
    {
        projectDir.mkdir(projectNameQ);
    }
    projectDir.cd(projectNameQ);

    // There can't be more projects with same name.
    if (m_Projects.count(projectName))
    {
        throw std::exception("Project with the same name is already loaded.");
    }

    // Initialize project.
    m_Projects[projectName] = unique_ptr<Project>(new Project());
    Project* project = m_Projects[projectName].get();
    project->Init(projectName,
                  Utils::StringQ2W(projectFullPath),
                  Utils::StringQ2W(projectDir.absolutePath()));

    // Load/Save project.
    if (load)
    {
        // If exists load project from file.
        if(!ProjectParser::Load(project))
        {
            throw std::exception("Failed to load project file.");
        }
    }
    else
    {
        // Save project file. Serialize as XML.
        if (!ProjectParser::Save(project))
        {
            throw std::exception("Failed to save project file.");
        }
    }

    // TODO: Move to method. Add images to view.
    // Add project to project model view.
    QStandardItem* rootItem = m_ProjectsModel.invisibleRootItem();
    ProjectItem* projectItem = new ProjectItem(projectName);
    QList<QStandardItem*> projectItems;
    projectItems << projectItem;
    rootItem->appendRow(projectItems);

    // Set new project as current project.
    SetCurrent(projectName);
}

void Workspace::AddDocument(wstring& projectName, QString& imageFullPath)
{
    QFileInfo imageFileInfo(imageFullPath);

    // Get project.
    Project* project = m_Projects[projectName].get();
    QString projectDirPath = Utils::StringW2Q(project->GetDocumentsPath());
    QDir projectDir(projectDirPath);

    // Copy image to project dir.
    QString dstImageFullPath = projectDir.absoluteFilePath(imageFileInfo.fileName());
    QFile::copy(imageFullPath, dstImageFullPath);

    // Get image name.
    QString docNameQ = imageFileInfo.baseName();
    std::wstring docName = Utils::StringQ2W(docNameQ);

    // Add image to project.
    Document* document = project->AddDocument(
                                docName,
                                Utils::StringQ2W(dstImageFullPath));

    // Add image to view model.
    auto projRowItem = m_ProjectsModel.findItems(Utils::StringW2Q(projectName));
    ProjectItem* projectItem = (ProjectItem*) projRowItem.first();

    ProjectItem* docItem = new ProjectItem(docName, true);
    QList<QStandardItem*> docRowItem;
    docRowItem << docItem;
    projectItem->appendRow(docItem);

    SetCurrent(project, projectItem, document, docItem);

    // TODO: Save project when new image is added.
}

void Workspace::SetCurrent(
        Project* project,
        ProjectItem* projectItem,
        Document* document,
        ProjectItem* documentItem)
{
    // Unselect previous items.
    if (m_CurrentProjectItem != nullptr)
    {
        m_CurrentProjectItem->Unselect();
    }
    if (m_CurrentDocumentItem != nullptr)
    {
        m_CurrentDocumentItem->Unselect();
    }

    m_CurrentProject = project;
    m_CurrentProjectItem = projectItem;
    m_CurrentDocument = document;
    m_CurrentDocumentItem = documentItem;

    // Select new items.
    if (m_CurrentProjectItem != nullptr)
    {
        m_CurrentProjectItem->Select();
    }
    if (m_CurrentDocumentItem != nullptr)
    {
        m_CurrentDocumentItem->Select();
    }
}

void Workspace::SetCurrent(std::wstring& projectName, std::wstring documentName)
{
    Project*     proj     = nullptr;
    ProjectItem* projItem = nullptr;
    Document*    doc      = nullptr;
    ProjectItem* docItem  = nullptr;

    // Get project;
    proj = m_Projects[projectName].get();

    // Get project view item.
    auto projRowItem = m_ProjectsModel.findItems(Utils::StringW2Q(projectName));
    projItem = (ProjectItem*) projRowItem.first();

    // If document is not specified, use first doc.
    auto docs = proj->GetDocuments();
    if (documentName.empty() && !docs.empty())
    {
        documentName = docs.front()->GetName();
    }

    // If there is document, get document and document view item.
    if (!documentName.empty())
    {
        // Get document.
        for (auto d : docs)
        {
            if (d->GetName().compare(documentName) == 0)
            {
                doc = d;
                break;
            }
        }

        // Get document view item.
        for (int i = 0; i < (int) docs.size(); i++)
        {
            ProjectItem* di = (ProjectItem*) projItem->child(i);
            if(di->GetName().compare(documentName) == 0)
            {
                docItem = di;
                break;
            }
        }
    }

    SetCurrent(proj, projItem, doc, docItem);
}

Mat Workspace::GetImage(ImageType imgType, ShapeType shapeType, bool fill, bool edge, bool center)
{
    // Check is there current image.
    return m_CurrentDocument->GetImage(imgType, shapeType, fill, edge, center);
}

bool Workspace::IsImageAvalilable()
{
    return m_CurrentDocument != nullptr;
}
