#include "workspace.h"


#include <QObject>

#include <exception>
#include "projectparser.h"
#include "utils.h"

#include <iostream>
#include <fstream>

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
    std::string projectName = Utils::StringQ2W(projectNameQ);

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
        // "Project with the same name is already loaded."
        throw std::exception();
    }

    // Initialize project.
    m_Projects[projectName] = unique_ptr<Project>(new Project());
    m_ProjectsPersistentState[projectName] = true; // loaded or saved
    Project* project = m_Projects[projectName].get();
    project->Init(projectName,
                  projectFullPath.toStdString(),
                  projectDir.absolutePath().toStdString());

    // Load/Save project.
    if (load)
    {
        // If exists load project from file.
        if(!ProjectParser::Load(project))
        {
            // "Failed to load project file."
            throw std::exception();
        }
    }
    else
    {
        // Save project file. Serialize as XML.
        if (!ProjectParser::Save(project))
        {
            // "Failed to save project file."
            throw std::exception();
        }
    }

    // TODO: Move to method. Add images to view.
    // Add project to project model view.
    QStandardItem* rootItem = m_ProjectsModel.invisibleRootItem();
    ProjectItem* projectItem = new ProjectItem(projectName);
    QList<QStandardItem*> projectItems;
    projectItems << projectItem;
    rootItem->appendRow(projectItems);

    for (Document* doc : project->GetDocuments())
    {
        auto projRowItem = m_ProjectsModel.findItems(Utils::StringW2Q(projectName));
        ProjectItem* projectItem = (ProjectItem*) projRowItem.first();

        ProjectItem* docItem = new ProjectItem(doc->GetName(), true);
        QList<QStandardItem*> docRowItem;
        docRowItem << docItem;
        projectItem->appendRow(docItem);
    }

    // Set new project as current project.
    SetCurrent(projectName);
}

void Workspace::AddDocument(string projectName, QString& imageFullPath, ProcessingOptions& options)
{
    QFileInfo imageFileInfo(imageFullPath);

    // Get project.
    Project* project = m_Projects[projectName].get();
    m_ProjectsPersistentState[projectName] = false;
    QString projectDirPath = QString(project->GetDocumentsPath().c_str());
    QDir projectDir(projectDirPath);

    // Copy image to project dir.
    QString dstImageFullPath = projectDir.absoluteFilePath(imageFileInfo.fileName());
    QFile::copy(imageFullPath, dstImageFullPath);

    // Get image name.
    QString docNameQ = imageFileInfo.baseName();
    std::string docName = Utils::StringQ2W(docNameQ);

    // Add image to project.
    Document* document = project->AddDocument(
                                docName,
                                Utils::StringQ2W(dstImageFullPath),
                                options);

    // Add image to view model.
    auto projRowItem = m_ProjectsModel.findItems(Utils::StringW2Q(projectName));
    ProjectItem* projectItem = (ProjectItem*) projRowItem.first();

    ProjectItem* docItem = new ProjectItem(docName, true);
    QList<QStandardItem*> docRowItem;
    docRowItem << docItem;
    projectItem->appendRow(docItem);

    SetCurrent(project, projectItem, document, docItem);
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

void Workspace::SetCurrent(std::string projectName, std::string documentName)
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

Mat Workspace::GetImage(ViewOptions view)
{
    // Check is there current image.
    return m_CurrentDocument->GetImage(view);
}

Stats Workspace::GetStats()
{
    // Check is there current image.
    return m_CurrentDocument->GetStats();
}

bool Workspace::IsImageAvalilable()
{
    return m_CurrentDocument != nullptr;
}

Document* Workspace::GetCurrentDocument()
{
    return m_CurrentDocument;
}

Project* Workspace::GetCurrentProject()
{
    return m_CurrentProject;
}

void Workspace::SaveCurrentProject()
{
    if (Workspace::Instance.GetCurrentProject() != nullptr)
    {
        ProjectParser::Save(Workspace::Instance.GetCurrentProject());
        m_ProjectsPersistentState[m_CurrentProject->GetName()] = true;
    }
}

void Workspace::SaveAllProjects()
{
    for (auto& proj : m_Projects)
    {
        ProjectParser::Save(proj.second.get());
        m_ProjectsPersistentState[proj.second.get()->GetName()] = true;
    }
}

void Workspace::Update(ProcessingOptions& options, bool processImmediately, bool keepManualEdits)
{
    if (processImmediately)
    {
        m_CurrentDocument->Process(options, keepManualEdits);
    }
    else
    {
        m_CurrentDocument->SetOptions(options);
        m_CurrentDocument->Clear();
    }

    m_ProjectsPersistentState[m_CurrentProject->GetName()] = true;
}

bool Workspace::IsCurrentProjectPersistent()
{
    return m_ProjectsPersistentState[m_CurrentProject->GetName()];
}

bool Workspace::AreAllProjectsPersistent()
{
    for (auto& persistent : m_ProjectsPersistentState)
    {
        if (!persistent.second)
        {
            return false;
        }
    }

    return true;
}

void Workspace::CloseCurrentProject()
{
    std::string name = m_CurrentProject->GetName();
    m_ProjectsModel.removeRow(m_CurrentProjectItem->row());
    m_Projects.erase(name);
    m_ProjectsPersistentState.erase(name);

    m_CurrentProject = nullptr;
    m_CurrentProjectItem = nullptr;
    m_CurrentDocument = nullptr;
    m_CurrentDocumentItem = nullptr;
}

void Workspace::CloseAllProjects()
{
    m_ProjectsModel.clear();
    m_Projects.clear();
    m_ProjectsPersistentState.clear();

    m_CurrentProject = nullptr;
    m_CurrentProjectItem = nullptr;
    m_CurrentDocument = nullptr;
    m_CurrentDocumentItem = nullptr;
}

void Workspace::RemoveCurrentDocument()
{
    m_CurrentProjectItem->removeRow(m_CurrentDocumentItem->row());
    m_CurrentProject->RemoveDocument(m_CurrentDocument->GetName());

    m_CurrentDocument = nullptr;
    m_CurrentDocumentItem = nullptr;
}

void Workspace::ExportTraces(std::string filePath)
{
    ofstream out;
    out.open(filePath);

    RatioOptions ratioOptions = m_CurrentDocument->GetRatioOptions();
    Stats stats = m_CurrentDocument->GetStats();

    out << "Broj tragova,\n";
    out << stats.TracesCount << ",\n";


    float surface = (m_CurrentDocument->GetWidth() * m_CurrentDocument->GetHeigth()) / (ratioOptions.PixelsPerUnit * ratioOptions.PixelsPerUnit);
    float countPerSurface = stats.TracesCount / surface;

    out << "Broj tragova po kvadratnom ";
    out << ratioOptions.Unit << ",\n";

    out << countPerSurface / (float) ratioOptions.PixelsPerUnit << ",\n";

    out << "Statistika,\n";
    out << "Diajmetar,\n";
    out << "Min,Max,Avg\n";
    out << stats.MinDiameter / (float) ratioOptions.PixelsPerUnit<< "," << stats.MaxDiameter / (float) ratioOptions.PixelsPerUnit << "," << stats.AverageDiameter / (float) ratioOptions.PixelsPerUnit << ",\n";
    out << "Intenzitet,\n";
    out << "Min,Max,Avg\n";
    out << stats.MinIntensity << "," << stats.MaxIntensity << "," << stats.AverageIntensity << ",\n";

    out << "Tragovi,\n";
    out << "x,y,ugao,dijametar1,dijametar2,intenzitet,\n";
    vector<Trace>& traces = m_CurrentDocument->GetTraces();
    for (Trace& trace : traces)
    {
        out << trace.x << "," << trace.y << "," << trace.angle << "," << trace.diameter1 << "," << trace.diameter2 << "," << trace.intensity << ",\n";
    }

    out.close();
}
