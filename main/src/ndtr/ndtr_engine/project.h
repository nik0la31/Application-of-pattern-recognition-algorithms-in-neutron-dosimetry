#ifndef PROJECT_H
#define PROJECT_H

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "document.h"

class Project
{
public:
    Project() {}

    void Init(std::wstring& name, std::wstring& path, std::wstring& docsPath);

    std::wstring GetName();

    void SetName(std::wstring& newName);

    std::wstring GetPath();

    Document* AddDocument(std::wstring& name, std::wstring& path);

    Document* GetDocument(std::wstring& name);

    std::vector<Document*> GetDocuments();

    std::wstring GetDocumentsPath();

private:
    // Project name.
    std::wstring m_Name;

    // Project path.
    std::wstring m_Path;

    // Documents dir path.
    std::wstring m_DocsPath;

    // Project documents (images with processing data).
    std::map<std::wstring, std::unique_ptr<Document> > m_Documents;
};

#endif // PROJECT_H
