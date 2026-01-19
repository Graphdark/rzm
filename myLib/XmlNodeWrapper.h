// XmlNodeWrapper.h: interface for the CXmlNodeWrapper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLNODEWRAPPER_H__43622334_FDEB_4175_9E6D_19BBAA3992A5__INCLUDED_)
#define AFX_XMLNODEWRAPPER_H__43622334_FDEB_4175_9E6D_19BBAA3992A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#import "MSXML3.dll" named_guids
using namespace MSXML2;

class CXmlNodeWrapper
{
public:
	CString NodeType();
	void ReplaceNode(MSXML2::IXMLDOMNodePtr pOldNode,MSXML2::IXMLDOMNodePtr pNewNode);
	CString GetText();
	void SetText(LPCTSTR text);
	MSXML2::IXMLDOMNodePtr InsertAfter(MSXML2::IXMLDOMNodePtr refNode, MSXML2::IXMLDOMNodePtr pNode);
	CString Name();
	MSXML2::IXMLDOMNodeListPtr FindNodes(LPCTSTR searchStr);
	MSXML2::IXMLDOMNodePtr Parent();
	void RemoveNodes(LPCTSTR searchStr);
	MSXML2::IXMLDOMNodePtr AppendChild(LPCTSTR nodeName, LPCTSTR nodeText = NULL);
  MSXML2::IXMLDOMNode*   AppendChildNode(MSXML2::IXMLDOMNodePtr pNode);
	MSXML2::IXMLDOMNodePtr InsertAfter(MSXML2::IXMLDOMNodePtr refNode, LPCTSTR nodeName);
	MSXML2::IXMLDOMNodePtr InsertBefore(MSXML2::IXMLDOMNodePtr refNode, LPCTSTR nodeName);
	MSXML2::IXMLDOMNode* Interface();
	MSXML2::IXMLDOMDocumentPtr ParentDocument();
	CString GetXML();
	MSXML2::IXMLDOMNodePtr RemoveNode(MSXML2::IXMLDOMNodePtr pNode);
	MSXML2::IXMLDOMNodePtr InsertNode(int index,LPCTSTR nodeName);
	MSXML2::IXMLDOMNodePtr InsertNode(int index,MSXML2::IXMLDOMNodePtr pNode);
	long NumNodes();
	MSXML2::IXMLDOMNode* Detach();
	MSXML2::IXMLDOMNodePtr FindNode(LPCTSTR nodeName);
	MSXML2::IXMLDOMNodePtr GetNode(int nodeIndex);
	MSXML2::IXMLDOMNodePtr GetPrevSibling();
	MSXML2::IXMLDOMNodePtr GetNextSibling();
  BOOL IsValid();

  int NumAttributes();
  void ARemove(LPCTSTR attrName);
  CString AName(int index);
  CString AValue(int index);
  CString AValue(LPCTSTR valueName);
  int     AValueInt   (LPCTSTR valueName);
  double  AValueDouble(LPCTSTR valueName);
  bool    AValueBool  (LPCTSTR valueName);

  void SetAValue(LPCTSTR valueName,LPCTSTR value);
	void SetAValue(LPCTSTR valueName,int value);
	void SetAValue(LPCTSTR valueName,short value);
	void SetAValue(LPCTSTR valueName,double value);
	void SetAValue(LPCTSTR valueName,float value);
	void SetAValue(LPCTSTR valueName,bool value);

  MSXML2::IXMLDOMNodePtr FindNodeAValue(LPCTSTR nodeName, LPCTSTR attrName, LPCTSTR attrValue, BOOL bIgnoreCase = TRUE);

  void SetIgnoreCaseSensitive(BOOL bIgnore = TRUE) { m_bIgnoreCaseSensitive = bIgnore; }
  BOOL IsIgnoreCaseSensitive() { return m_bIgnoreCaseSensitive; }

	CXmlNodeWrapper();
	CXmlNodeWrapper(MSXML2::IXMLDOMNodePtr pNode,BOOL bAutoRelease = TRUE);
	void operator=(MSXML2::IXMLDOMNodePtr pNode);
	virtual ~CXmlNodeWrapper();
private:
	BOOL m_bAutoRelease;
	MSXML2::IXMLDOMNodePtr m_xmlnode;
  BOOL m_bIgnoreCaseSensitive;
};

class CXmlDocumentWrapper
{
public:
	CString GetUrl();
	CString GetXML(BOOL bPreserveWhiteSpace = TRUE);

	BOOL IsValid();
	BOOL Load(LPCTSTR path);
	BOOL LoadXML(LPCTSTR xml);
	BOOL LoadXML(BSTR xml);
	BOOL Save(LPCTSTR path = _T(""), BOOL bPreserveWhiteSpace = TRUE);
	BOOL SaveToFileAsFmt(LPCTSTR sFilePath);
	CString Transform(LPCTSTR pathXSL) const;
	CString GetHeader(BOOL bAsXml = FALSE);
	
	static BOOL IsVersion3orGreater();

	MSXML2::IXMLDOMDocument* Detach();
	MSXML2::IXMLDOMDocumentPtr Clone();
	MSXML2::IXMLDOMDocument* Interface();
 	CXmlDocumentWrapper(LPCTSTR header = NULL, LPCTSTR szRootItem = NULL);
	CXmlDocumentWrapper(MSXML2::IXMLDOMDocumentPtr pDoc);
	void operator=(MSXML2::IXMLDOMDocumentPtr pDoc);
	MSXML2::IXMLDOMNodePtr AsNode();
	virtual ~CXmlDocumentWrapper();
protected:
	void SetHeader(LPCTSTR szHeader);
private:
	MSXML2::IXMLDOMDocumentPtr m_xmldoc;
	static BOOL s_bVer3orGreater;

public:
	void setAutoFormatting(BOOL bAutoFormatting);
	void setEncoding(LPCTSTR sEncoding);
	void writeNamespace(LPCTSTR namespace_uri, LPCTSTR prefix);
	void writeStartDocument();
	void writeEndDocument();
	void writeDefaultNamespace(LPCTSTR namespace_uri);
	void writeStartElement(LPCTSTR namespace_uri, LPCTSTR name);
	void writeEndElement();
	void writeAttribute(LPCTSTR name, LPCTSTR value, LPCTSTR namespace_uri);
};

class CXmlNodeListWrapper
{
public:
	MSXML2::IXMLDOMDocumentPtr AsDocument();
	MSXML2::IXMLDOMNodePtr Node(int index);
	void Start();
	MSXML2::IXMLDOMNodePtr Next();
	BOOL IsValid();
	int Count();
	CXmlNodeListWrapper();
	CXmlNodeListWrapper(MSXML2::IXMLDOMNodeListPtr pList);
	void operator=(MSXML2::IXMLDOMNodeListPtr pList);
	virtual ~CXmlNodeListWrapper();

private:
	MSXML2::IXMLDOMNodeListPtr m_xmlnodelist;
};

#endif // !defined(AFX_XMLNODEWRAPPER_H__43622334_FDEB_4175_9E6D_19BBAA3992A5__INCLUDED_)
