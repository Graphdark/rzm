// /****************************************************************************
//
// Utility base class for widgets that represents options for actions
//
// Copyright (C) 2025 LibreCAD.org
// Copyright (C) 2025 sand1024
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// **********************************************************************
//

#include "stdafx.h"

//#include <QXmlStreamWriter>
#include "XmlNodeWrapper.h"
#include "lc_xmlwriterqxmlstreamwriter.h"


LC_XMLWriterQXmlStreamWriter::LC_XMLWriterQXmlStreamWriter():
	xmlWriter(new CXmlDocumentWrapper()) //&xml
{
	xmlWriter->setAutoFormatting(true);
	xmlWriter->setEncoding(L"UTF-8");
}

LC_XMLWriterQXmlStreamWriter::~LC_XMLWriterQXmlStreamWriter() = default;

void LC_XMLWriterQXmlStreamWriter::createRootElement(const std::string &name, const std::string &namespace_uri) {
    xmlWriter->writeStartDocument();
    xmlWriter->writeDefaultNamespace((LPCTSTR)namespace_uri.c_str());
    xmlWriter->writeStartElement((LPCTSTR)namespace_uri.c_str(), (LPCTSTR)name.c_str());
}

void LC_XMLWriterQXmlStreamWriter::addElement(const std::string &name, const std::string &namespace_uri) {
    xmlWriter->writeStartElement((LPCTSTR)namespace_uri.c_str(), (LPCTSTR)name.c_str());
}

void LC_XMLWriterQXmlStreamWriter::addAttribute(const std::string &name, const std::string &value, const std::string &namespace_uri) {
    xmlWriter->writeAttribute((LPCTSTR)namespace_uri.c_str(), (LPCTSTR)name.c_str(), (LPCTSTR)value.c_str());
}

void LC_XMLWriterQXmlStreamWriter::addNamespaceDeclaration(const std::string &prefix, const std::string &namespace_uri) {
    xmlWriter->writeNamespace((LPCTSTR)namespace_uri.c_str(), (LPCTSTR)prefix.c_str());
}

void LC_XMLWriterQXmlStreamWriter::closeElement() {
    xmlWriter->writeEndElement();
}

std::string LC_XMLWriterQXmlStreamWriter::documentAsString() {
    xmlWriter->writeEndDocument();
//    return xml.toStdString();
    CString cs = xmlWriter->GetXML();
    std::string s; // = TOU s;
    return s;
}
