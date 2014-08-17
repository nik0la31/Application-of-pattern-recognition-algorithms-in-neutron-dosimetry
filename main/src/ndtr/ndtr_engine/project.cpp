#include "project.h"

using namespace std;

void Project::Init(wstring& name, wstring& path, wstring& docsPath)
{
    m_Name = name;
    m_Path = path;
    m_DocsPath = docsPath;
}

wstring Project::GetPath()
{
    return m_Path;
}

wstring Project::GetName()
{
    return m_Name;
}

void Project::SetName(wstring& newName)
{
    m_Name = newName;
}

vector<Document*> Project::GetDocuments()
{
    vector<Document*> documents;
    for (auto doc = m_Documents.begin(); doc != m_Documents.end(); ++doc)
    {
        documents.push_back(doc->second.get());
    }
    return documents;
}

Document* Project::GetDocument(std::wstring& name)
{
    return m_Documents[name].get();
}

wstring Project::GetDocumentsPath()
{
    return m_DocsPath;
}

Document* Project::AddDocument(wstring& name, wstring& path)
{
    m_Documents[name] = unique_ptr<Document>(new Document());
    m_Documents[name]->Init(this, name, path);

    return m_Documents[name].get();
}
