#ifndef PROJECTPARSER_H
#define PROJECTPARSER_H

#include <project.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "utils.h"
#include <stack>

class ProjectParser
{
public:
    ProjectParser();

    static bool Load(Project* project)
    {
        try
        {
            QFile file(QString(project->GetPath().c_str()));
            file.open(QIODevice::ReadOnly | QFile::Text);

            QXmlStreamReader stream(&file);

            std::stack<std::string> tags;

            Document* doc;

            std::string path;

            while(!stream.atEnd())
            {
                QString xname = stream.name().toString();
                if(stream.isStartElement())
                {
                    if (stream.name() == "ndtr")
                    {
                        tags.push("ndtr");
                    }
                    else if(stream.name() == "documents")
                    {
                        tags.push("documents");
                    }
                    else if(stream.name() == "document")
                    {
                        tags.push("document");

                        QXmlStreamAttributes attributes = stream.attributes();
                        std::string name = attributes.value(QString("name")).toString().toStdString();
                        std::string ext = attributes.value(QString("ext")).toString().toStdString();
                        std::string separator = QString(QDir::separator()).toStdString();
                        path = project->GetDocumentsPath().append(separator).append(name).append(ext);

                        doc = new Document();
                        doc->Init(project, name, path);
                    }
                    else if(stream.name() == "options")
                    {
                        tags.push("options");

                        ProcessingOptions options;
                        RatioOptions ratioOptions;
                        LoadOptions(stream, options, ratioOptions);

                        //doc->Update(options);
// use ext,not path
                        Document* doc2 = project->AddDocument(doc->GetName(), path, options);
                        doc2->SetRatioOptions(ratioOptions.PixelsPerUnit, ratioOptions.XCenterOffset, ratioOptions.YCenterOffset); // todo
                        doc2->SetBaseUnitRatio(ratioOptions.BaseRatio); // todo
                    }
                    else
                    {
                        stream.raiseError("Unexpected tag in NDTR project file.");
                    }
                }
                else if (stream.isEndElement())
                {
                    std::string name = stream.name().toString().toStdString();

                    if (tags.top().compare(name) == 0)
                    {
                        tags.pop();
                    }
                    else
                    {
                        stream.raiseError("Unexpected closing tag in NDTR project file.");
                    }
                }

                stream.readNext();
            }

            file.close();
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    static void LoadOptions(QXmlStreamReader& stream, ProcessingOptions& options, RatioOptions& ratioOptions)
    {
        QXmlStreamAttributes attributes = stream.attributes();

        if (attributes.hasAttribute(QString("AutomaticOtsuThreshold")))
        {
            int autoThresh = attributes.value(QString("AutomaticOtsuThreshold")).toInt();
            options.AutomaticOtsuThreshold = autoThresh != 0;
        }

        if (attributes.hasAttribute(QString("OtsuThreshold")))
        {
            int thresh = attributes.value(QString("OtsuThreshold")).toInt();
            options.OtsuThreshold = thresh;
        }

        if (attributes.hasAttribute(QString("GaussianBlur")))
        {
            int blur = attributes.value(QString("GaussianBlur")).toInt();
            options.GaussianBlur = blur != 0;
        }

        if (attributes.hasAttribute(QString("WoB")))
        {
            int wob = attributes.value(QString("WoB")).toInt();
            options.WoB = wob != 0;
        }

        if (attributes.hasAttribute(QString("MinTraceDiameter")))
        {
            int min = attributes.value(QString("MinTraceDiameter")).toInt();
            options.MinTraceDiameter = min;
        }

        if (attributes.hasAttribute(QString("MaxTraceDiameter")))
        {
            int max = attributes.value(QString("MaxTraceDiameter")).toInt();
            options.MaxTraceDiameter = max;
        }

        if (attributes.hasAttribute(QString("PixelsPerUnit")))
        {
            ratioOptions.PixelsPerUnit = attributes.value(QString("PixelsPerUnit")).toFloat();
        }

        if (attributes.hasAttribute(QString("XCenterOffset")))
        {
            ratioOptions.XCenterOffset = attributes.value(QString("XCenterOffset")).toFloat();
        }

        if (attributes.hasAttribute(QString("YCenterOffset")))
        {
            ratioOptions.YCenterOffset = attributes.value(QString("YCenterOffset")).toFloat();
        }

        if (attributes.hasAttribute(QString("BaseRatio")))
        {
            ratioOptions.BaseRatio = attributes.value(QString("BaseRatio")).toInt();
        }
    }

    static bool Save(Project* project)
    {
        try
        {
            QFile f(QString(project->GetPath().c_str()));
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
                stream.writeStartElement("document");

                QString name("name");
                stream.writeAttribute(name, QString(doc->GetName().c_str()));
                QString ext("ext");
                stream.writeAttribute(ext, QString(doc->GetExt().c_str()));

                // OPTIONS  - START/END
                stream.writeStartElement("options");

                ProcessingOptions options = doc->GetOptions();

                QString autoThresh("AutomaticOtsuThreshold");
                stream.writeAttribute(autoThresh, QString::number(options.AutomaticOtsuThreshold ? 1 : 0));
                QString thresh("OtsuThreshold");
                stream.writeAttribute(thresh, QString::number(options.OtsuThreshold));
                QString blur("GaussianBlur");
                stream.writeAttribute(blur, QString::number(options.GaussianBlur ? 1 : 0));
                QString wob("WoB");
                stream.writeAttribute(wob, QString::number(options.WoB ? 1 : 0));
                QString min("MinTraceDiameter");
                stream.writeAttribute(min, QString::number(options.MinTraceDiameter));
                QString max("MaxTraceDiameter");
                stream.writeAttribute(max, QString::number(options.MaxTraceDiameter));

                RatioOptions ratioOptions = doc->GetRatioOptions();

                QString ppu("PixelsPerUnit");
                stream.writeAttribute(ppu, QString::number(ratioOptions.PixelsPerUnit));
                QString xco("XCenterOffset");
                stream.writeAttribute(xco, QString::number(ratioOptions.XCenterOffset));
                QString yco("YCenterOffset");
                stream.writeAttribute(yco, QString::number(ratioOptions.YCenterOffset));
                QString br("BaseRatio");
                stream.writeAttribute(br, QString::number(ratioOptions.BaseRatio));

                // OPTIONS  - END
                stream.writeEndElement();

                // DOCUMENT  - END
                stream.writeEndElement();
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
