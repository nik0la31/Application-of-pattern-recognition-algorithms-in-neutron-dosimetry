#ifndef PROJECTPARSER_H
#define PROJECTPARSER_H

#include <project.h>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "utils.h"

class ProjectParser
{
public:
    ProjectParser();

    static bool Load(Project* project)
    {
        try
        {
            QFile f(Utils::StringW2Q(project->GetPath()));
            f.open(QIODevice::ReadOnly | QFile::Text);

            QXmlStreamReader stream(&f);

            stream.readNext();

            while(!stream.atEnd())
            {
                if(stream.isStartElement())
                {
                    if (stream.name() == "ndtr")
                    {
                        stream.readNext();

                    }
                    //else if(stream.name() == "name")
                    //{
                    //    project.SetName(Utils::StringQ2W(stream.readElementText()));
                    //}
                    else
                    {
                        stream.raiseError("Unexpected tag in NDTR project file.");
                    }
                }
                else
                {
                    stream.readNext();
                }
            }

            f.close();
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    static bool Save(Project* project)
    {
        try
        {
            QFile f(Utils::StringW2Q(project->GetPath()));
            f.open(QIODevice::WriteOnly);

            QXmlStreamWriter stream(&f);
            stream.setAutoFormatting(true);

            // FILE      - START
            stream.writeStartDocument();

            // NDTR      - START
            stream.writeStartElement("ndtr");

            // DOCUMENTS - START
            stream.writeStartElement("documents");

            auto documents = project->GetDocuments();
            for(auto doc : documents)
            {
                // DOCUMENT  - START/END
                stream.writeTextElement("document", Utils::StringW2Q(doc->GetName()));

                // DOCUMENT  - END
            }

            // DOCUMENTS - END
            stream.writeEndElement();

            // NDTR      - END
            stream.writeEndElement();

            // FILE      - END
            stream.writeEndDocument();

            f.close();
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

};

#endif // PROJECTPARSER_H
