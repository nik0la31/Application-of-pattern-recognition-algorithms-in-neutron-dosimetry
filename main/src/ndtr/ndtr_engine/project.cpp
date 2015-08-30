#include "project.h"

using namespace std;
using namespace cv;

Project::~Project()
{
    for (Document* doc : m_Documents)
    {
        delete(doc);
    }
}

void Project::Init(string name, string path, string docsPath)
{
    m_Name = name;
    m_Path = path;
    m_DocsPath = docsPath;
}

string Project::GetName()
{
    return m_Name;
}

void Project::SetName(string name)
{
    m_Name = name;
}

string Project::GetPath()
{
    return m_Path;
}

string Project::GetDocumentsPath()
{
    return m_DocsPath;
}

Document* Project::AddDocument(string name, string path, ProcessingOptions& options)
{
    Document* doc  = new Document();
    doc->Init(this, name, path);

    if (m_Documents.size() > 0)
    {
        Mat transform = Document::CalcTransform(m_Documents.back(), doc);
        doc->SetTransform(transform);
    }

    m_Documents.push_back(doc);

    return doc;
}

Document* Project::GetDocument(string name)
{
    for (Document* doc : m_Documents)
    {
        if (doc->GetName() == name)
        {
            return doc;
        }
    }

    return nullptr;
}

void Project::RemoveDocument(string name)
{
    Document* doc = GetDocument(name);
    if (doc != nullptr)
    {
        m_Documents.remove(doc);
        delete(doc);
    }
}

list<Document*> Project::GetDocuments()
{
    return m_Documents;
}
