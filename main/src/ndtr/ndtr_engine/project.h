#ifndef PROJECT_H
#define PROJECT_H

#include "document.h"
#include "processingoptions.h"

#include <list>
#include <string>

class Project
{
public:
    Project() {}

    ~Project();

    void Init(std::string& name, std::string& path, std::string& docsPath);

    std::string GetName();

    void SetName(std::string& name);

    std::string GetPath();

    std::string GetDocumentsPath();

    Document* AddDocument(std::string& name, std::string& path, ProcessingOptions& options);

    Document* GetDocument(std::string& name);

    void RemoveDocument(std::string& name);

    std::list<Document*> GetDocuments();

private:
    // Project name.
    std::string m_Name;

    // Project path.
    std::string m_Path;

    // Documents dir path.
    std::string m_DocsPath;

    // Project documents (images with processing data).
    std::list<Document*> m_Documents;
};

#endif // PROJECT_H
