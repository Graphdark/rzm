// XmlNodeWrapper.cpp: implementation of the CXmlNodeWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XmlNodeWrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const LPCTSTR DEFAULT_HEADER = _T("version=\"1.0\" encoding=\"windows-1251\"");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXmlNodeWrapper::CXmlNodeWrapper()
{
  m_bAutoRelease = TRUE;
  m_bIgnoreCaseSensitive = TRUE;
}

CXmlNodeWrapper::CXmlNodeWrapper(MSXML2::IXMLDOMNodePtr pNode,BOOL bAutoRelease)
{
	m_xmlnode = pNode;
  m_bAutoRelease = bAutoRelease;
  m_bIgnoreCaseSensitive = TRUE;
}
void CXmlNodeWrapper::operator=(MSXML2::IXMLDOMNodePtr pNode)
{
	m_xmlnode = pNode;
}

CXmlNodeWrapper::~CXmlNodeWrapper()
{
	if (!m_bAutoRelease)
		m_xmlnode.Detach();
}


BOOL CXmlNodeWrapper::IsValid()
{
	if (m_xmlnode == NULL)
		return FALSE;
	if (m_xmlnode.GetInterfacePtr() == NULL)
		return FALSE;
	return TRUE;
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::GetPrevSibling()
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->GetpreviousSibling();
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::GetNextSibling()
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->GetnextSibling();
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::FindNode(LPCTSTR nodeName)
{
	if (!IsValid())
		return NULL;
	try{
		return m_xmlnode->selectSingleNode(nodeName);
	}
	catch (_com_error e)
	{
		CString err = e.ErrorMessage();
	}
	return NULL;
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::GetNode(int nodeIndex)
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->GetchildNodes()->Getitem(nodeIndex);
}


MSXML2::IXMLDOMNode* CXmlNodeWrapper::Detach()
{
	if (IsValid())
	{
		return m_xmlnode.Detach();
	}
	else
		return NULL;
}

long CXmlNodeWrapper::NumNodes()
{
	if (IsValid())
	{
		return m_xmlnode->GetchildNodes()->Getlength();
	}
	else
		return 0;
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::InsertNode(int index,LPCTSTR nodeName)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
		MSXML2::IXMLDOMNodePtr newNode = xmlDocument->createNode(_variant_t((short)MSXML2::NODE_ELEMENT),nodeName,_T(""));
		MSXML2::IXMLDOMNodePtr refNode = GetNode(index);
		if (refNode)
			newNode = m_xmlnode->insertBefore(newNode,_variant_t(refNode.GetInterfacePtr()));
		else
			newNode = m_xmlnode->appendChild(newNode);
		return newNode;
	}
	return NULL;
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::InsertNode(int index,MSXML2::IXMLDOMNodePtr pNode)
{
	MSXML2::IXMLDOMNodePtr newNode = pNode->cloneNode(VARIANT_TRUE);
	if (newNode)
	{
		MSXML2::IXMLDOMNodePtr refNode = GetNode(index);
		if (refNode)
			newNode = m_xmlnode->insertBefore(newNode,_variant_t(refNode.GetInterfacePtr()));
		else
			newNode = m_xmlnode->appendChild(newNode);
		return newNode;
	}
	else
		return NULL;
}

CString CXmlNodeWrapper::GetXML()
{
	if (IsValid())
		return (LPCTSTR )m_xmlnode->Getxml();
	else
		return _T("");
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::RemoveNode(MSXML2::IXMLDOMNodePtr pNode)
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->removeChild(pNode);
}

MSXML2::IXMLDOMNode* CXmlNodeWrapper::Interface()
{
	if (IsValid())
		return m_xmlnode;
	return NULL;
}

MSXML2::IXMLDOMNode* CXmlNodeWrapper::AppendChildNode(MSXML2::IXMLDOMNodePtr pNode)
{
	return m_xmlnode->appendChild(pNode);
}


MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::AppendChild(LPCTSTR nodeName, LPCTSTR nodeText)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
    MSXML2::IXMLDOMNodePtr newNode = xmlDocument->createNode(_variant_t((short)MSXML2::NODE_ELEMENT),nodeName, _T(""));
		newNode = m_xmlnode->appendChild(newNode);
    if (newNode != NULL && nodeText != NULL)
    {
      newNode->Puttext(nodeText);
    }
		return newNode;
	}
	return NULL;
}


MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::InsertBefore(MSXML2::IXMLDOMNodePtr refNode, LPCTSTR nodeName)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
		MSXML2::IXMLDOMNodePtr newNode = xmlDocument->createNode(_variant_t((short)MSXML2::NODE_ELEMENT),nodeName,_T(""));
		newNode = m_xmlnode->insertBefore(newNode,_variant_t(refNode.GetInterfacePtr()));
		return newNode;
	}
	return NULL;
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::InsertAfter(MSXML2::IXMLDOMNodePtr refNode, LPCTSTR nodeName)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
		MSXML2::IXMLDOMNodePtr newNode = xmlDocument->createNode(_variant_t((short)MSXML2::NODE_ELEMENT),nodeName,_T(""));
		MSXML2::IXMLDOMNodePtr nextNode = refNode->GetnextSibling();
		if (nextNode.GetInterfacePtr() != NULL)
			newNode = m_xmlnode->insertBefore(newNode,_variant_t(nextNode.GetInterfacePtr()));
		else
			newNode = m_xmlnode->appendChild(newNode);
		return newNode;
	}
	return NULL;
}

void CXmlNodeWrapper::RemoveNodes(LPCTSTR searchStr)
{
	if (!IsValid())
		return;
	MSXML2::IXMLDOMNodeListPtr nodeList = m_xmlnode->selectNodes(searchStr);
	for (int i = 0; i < nodeList->Getlength(); i++)
	{
		try
		{
			MSXML2::IXMLDOMNodePtr pNode = nodeList->Getitem(i);
			pNode->GetparentNode()->removeChild(pNode);
		}
		catch (_com_error er)
		{
			AfxMessageBox(er.ErrorMessage());
		}
	}
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::Parent()
{
	if (IsValid())
		return m_xmlnode->GetparentNode();
	return NULL;
}

/* ********************************************************************************************************* */
CXmlDocumentWrapper::CXmlDocumentWrapper(LPCTSTR header, LPCTSTR szRootItem)
{
	try
	{
		m_xmldoc.CreateInstance(MSXML2::CLSID_DOMDocument);
		m_xmldoc->put_validateOnParse(VARIANT_FALSE);
		m_xmldoc->put_async(VARIANT_FALSE);
		
		if (szRootItem && *szRootItem)
		{
			CString sRoot;
			sRoot.Format(_T("<?xml %s?> <%s/>"), DEFAULT_HEADER, szRootItem);
			LoadXML(sRoot);
		}

	   // set header afterwards so it not overwritten
		if (header && *header)
			SetHeader(header);
	}
	catch (...)
	{
	}
}


CXmlDocumentWrapper::CXmlDocumentWrapper(MSXML2::IXMLDOMDocumentPtr pDoc)
{
	m_xmldoc = pDoc;
}

void CXmlDocumentWrapper::operator=(MSXML2::IXMLDOMDocumentPtr pDoc)
{
	if (IsValid())
		m_xmldoc.Release();
	m_xmldoc = pDoc;
}


CXmlDocumentWrapper::~CXmlDocumentWrapper()
{
}

BOOL CXmlDocumentWrapper::IsValid()
{
	if (m_xmldoc == NULL)
		return FALSE;
	if (m_xmldoc.GetInterfacePtr() == NULL)
		return FALSE;
	return TRUE;
}

MSXML2::IXMLDOMDocument* CXmlDocumentWrapper::Detach()
{
	if (!IsValid())
		return NULL;
	return m_xmldoc.Detach();
}

MSXML2::IXMLDOMDocumentPtr CXmlDocumentWrapper::Clone()
{
	if (!IsValid())
		return NULL;
	MSXML2::IXMLDOMDocumentPtr xmldoc;
	xmldoc.CreateInstance(MSXML2::CLSID_DOMDocument);
	_variant_t v(xmldoc.GetInterfacePtr());
	m_xmldoc->save(v);
	return xmldoc;
}

MSXML2::IXMLDOMDocument* CXmlDocumentWrapper::Interface()
{
	if (IsValid())
		return m_xmldoc;
	return NULL;
}

BOOL CXmlDocumentWrapper::Load(LPCTSTR path)
{
	if (!IsValid())
		return FALSE;

	_variant_t v(path);
	m_xmldoc->put_async(VARIANT_FALSE);
	VARIANT_BOOL success = m_xmldoc->load(v);
	if (success == VARIANT_TRUE)
		return TRUE;
	else
		return FALSE;
}

BOOL CXmlDocumentWrapper::LoadXML(LPCTSTR xml)
{
	if (!IsValid())
		return FALSE;
	if (m_xmldoc->loadXML(xml) == VARIANT_TRUE)
		return TRUE;
	else
		return FALSE;
}

BOOL CXmlDocumentWrapper::LoadXML(BSTR xml)
{
	if (!IsValid())
		return FALSE;
  VARIANT_BOOL success = m_xmldoc->loadXML(xml);
	if (success == VARIANT_TRUE)
		return TRUE;
	else
		return FALSE;
}

BOOL CXmlDocumentWrapper::Save(LPCTSTR path, BOOL bPreserveWhiteSpace)
{
	try
	{
		if (!IsValid())
			return FALSE;

    m_xmldoc->put_preserveWhiteSpace(bPreserveWhiteSpace ? VARIANT_TRUE : VARIANT_FALSE);

		CString szPath(path);
		if (szPath == _T(""))
		{
			_bstr_t curPath = m_xmldoc->Geturl();
			szPath = (LPTSTR)curPath;
		}
		_variant_t v(szPath);
		if (FAILED(m_xmldoc->save(v)))
			return FALSE;
		else
			return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

BOOL CXmlDocumentWrapper::SaveToFileAsFmt(LPCTSTR sFilePath)
{
	MSXML2::IMXWriterPtr pWriter(__uuidof(MSXML2::MXXMLWriter));
	MSXML2::ISAXXMLReaderPtr pReader(__uuidof(MSXML2::SAXXMLReader));
	pReader->putContentHandler((MSXML2::ISAXContentHandlerPtr)pWriter);
	pReader->putErrorHandler((MSXML2::ISAXErrorHandlerPtr)pWriter);

	pWriter->put_encoding(A2BSTR("UTF-8"));
	pWriter->put_indent(VARIANT_TRUE);
	pWriter->put_byteOrderMark(VARIANT_TRUE);
	pWriter->put_omitXMLDeclaration(VARIANT_FALSE);
	pWriter->put_standalone(VARIANT_TRUE);

	DWORD grfMode = STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE;
	IStream* pStream;
	// ...create a new file opened for write, and return an IStream interface.
	HRESULT hr = ::SHCreateStreamOnFile(sFilePath, grfMode, &pStream);
	if (SUCCEEDED(hr))
	{
		hr = pWriter->put_output(_variant_t(pStream));
		if (SUCCEEDED(hr))
			hr = pReader->parse(_variant_t(m_xmldoc.GetInterfacePtr()));
		pStream->Release();
	}
	return SUCCEEDED(hr);
}

void CXmlDocumentWrapper::setAutoFormatting(BOOL bAutoFormatting)
{

}

void CXmlDocumentWrapper::setEncoding(LPCTSTR sEncoding)
{

}

void CXmlDocumentWrapper::writeNamespace(LPCTSTR namespace_uri, LPCTSTR prefix)
{

}

void CXmlDocumentWrapper::writeStartDocument()
{

}

void CXmlDocumentWrapper::writeEndDocument()
{

}

void CXmlDocumentWrapper::writeDefaultNamespace(LPCTSTR namespace_uri)
{

}

void CXmlDocumentWrapper::writeStartElement(LPCTSTR namespace_uri, LPCTSTR name)
{

}

void CXmlDocumentWrapper::writeEndElement()
{

}

void CXmlDocumentWrapper::writeAttribute(LPCTSTR name, LPCTSTR value, LPCTSTR namespace_uri)
{
}




MSXML2::IXMLDOMNodePtr CXmlDocumentWrapper::AsNode()
{
	if (!IsValid())
		return NULL;
	return m_xmldoc->GetdocumentElement();
}

CString CXmlDocumentWrapper::GetXML(BOOL bPreserveWhiteSpace)
{
  CString sXml;
  if (IsValid())
  {
    m_xmldoc->put_preserveWhiteSpace(bPreserveWhiteSpace ? VARIANT_TRUE : VARIANT_FALSE);
    sXml = (LPCTSTR)m_xmldoc->Getxml();

    // Getxml doesn't return entire header so we need to fix it up here
    CString sHeader = GetHeader(TRUE);
    int nStart = sXml.Find(_T("?xml"));
    if (nStart > 0)
    {
      // find start brace
      while (sXml[nStart] != '<')
        nStart--;
      // find end brace
      int nEnd = sXml.Find(_T(">")) + 1; // closing brace
      sXml = sXml.Left(nStart) + sHeader + sXml.Mid(nEnd);
    }
    else // just add header
      sXml = sHeader + sXml;
  }
  return sXml;
}


CString CXmlDocumentWrapper::GetUrl()
{
	return (LPTSTR)m_xmldoc->Geturl();
}

MSXML2::IXMLDOMDocumentPtr CXmlNodeWrapper::ParentDocument()
{
	return m_xmlnode->GetownerDocument();
}

CString CXmlDocumentWrapper::Transform(LPCTSTR pathXSL) const
{
  CXmlDocumentWrapper xsl;
  try
  {
    if (xsl.Load(pathXSL))
      return (LPTSTR)m_xmldoc->transformNode(xsl.AsNode());
  }
  catch( const _com_error & err)
  {
    AfxMessageBox(err.ErrorMessage(), MB_OK | MB_ICONERROR);
  }
  return _T("");
}

CString CXmlDocumentWrapper::GetHeader(BOOL bAsXml)
{
	CString sHeader;
	if (IsValid())
	{
		CXmlNodeWrapper nodeHdr(m_xmldoc->childNodes->item[0]);
		
		if (nodeHdr.IsValid())
		{
			if (nodeHdr.GetXML().Find(_T("?xml")) == 1) // <?xml
			{
				int nAttribs = nodeHdr.NumAttributes();
				
				for (int nAttrib = 0; nAttrib < nAttribs; nAttrib++)
				{
					CString sAttrib;
					sAttrib.Format(_T("%s=\"%s\" "), nodeHdr.AName(nAttrib), nodeHdr.AValue(nAttrib));
					sHeader += sAttrib;
				}
			}
		}
		
		if (sHeader.IsEmpty())
		{
			sHeader = DEFAULT_HEADER;
			
			// get active code page
			CString sCodePage;
			
			GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, sCodePage.GetBuffer(7), 6);
			sCodePage.ReleaseBuffer();
			
			// and replace in header
			if (_tstoi(sCodePage) > 0)
				sHeader.Replace(_T("1251"), sCodePage);
		}
		
		if (bAsXml)
		{
			sHeader = _T("<?xml ") + sHeader;
			sHeader += _T("?>");
		}
	}
	
	return sHeader;
}

void CXmlDocumentWrapper::SetHeader(LPCTSTR szHeader)
{
	if (IsValid())
	{
		//_bstr_t name(_com_util_fix::ConvertStringToBSTR("xml"), FALSE);
		//_bstr_t bstr(_com_util_fix::ConvertStringToBSTR(szHeader), FALSE);
#ifdef _UNICODE
		_bstr_t name(_T("xml"));
		_bstr_t bstr(szHeader);
#else
		_bstr_t name(_com_util::ConvertStringToBSTR(_T("xml")), FALSE);
		_bstr_t bstr(_com_util::ConvertStringToBSTR(szHeader), FALSE);
#endif _UNICODE
		
		MSXML2::IXMLDOMProcessingInstructionPtr pHdr = m_xmldoc->createProcessingInstruction(name, bstr);
		
		// always insert header right at the start
		MSXML2::IXMLDOMNodePtr pNode = m_xmldoc->childNodes->item[0];

      if (pNode)
      {
         CString sXml = (LPCTSTR)pNode->Getxml();
         _variant_t varFirst(pNode.GetInterfacePtr());

         m_xmldoc->insertBefore(pHdr, varFirst);
      }
      else
		m_xmldoc->appendChild(pHdr);
	}
}

BOOL CXmlDocumentWrapper::s_bVer3orGreater = -1;

BOOL CXmlDocumentWrapper::IsVersion3orGreater()
{
	if (s_bVer3orGreater == -1)
	{
		try
		{
			// try to load some dummy xml
			CXmlDocumentWrapper doc;
			CString sXml;
			sXml.Format(_T("<?xml %s ?><NodeList/>"), DEFAULT_HEADER);
			s_bVer3orGreater = doc.LoadXML(sXml);
		}
		catch (CException* /*e*/)
		{
			s_bVer3orGreater = FALSE;
		}
	}
	return s_bVer3orGreater;
}


/* ********************************************************************************************************* */

CXmlNodeListWrapper::CXmlNodeListWrapper()
{
}
CXmlNodeListWrapper::CXmlNodeListWrapper(MSXML2::IXMLDOMNodeListPtr pList)
{
	m_xmlnodelist = pList;
}

void CXmlNodeListWrapper::operator=(MSXML2::IXMLDOMNodeListPtr pList)
{
	m_xmlnodelist = pList;
}

CXmlNodeListWrapper::~CXmlNodeListWrapper()
{

}

int CXmlNodeListWrapper::Count()
{
	if (IsValid())
		return m_xmlnodelist->Getlength();
	else
		return 0;

}

BOOL CXmlNodeListWrapper::IsValid()
{
	if (m_xmlnodelist == NULL)
		return FALSE;
	if (m_xmlnodelist.GetInterfacePtr() == NULL)
		return FALSE;
	return TRUE;
}

MSXML2::IXMLDOMNodePtr CXmlNodeListWrapper::Next()
{
	if (IsValid())
		return m_xmlnodelist->nextNode();
	else
		return NULL;
}

void CXmlNodeListWrapper::Start()
{
	if (IsValid())
		m_xmlnodelist->reset();
}

MSXML2::IXMLDOMNodePtr CXmlNodeListWrapper::Node(int index)
{
	if (IsValid())
		return m_xmlnodelist->Getitem(index);
	else
		return NULL;
}

MSXML2::IXMLDOMDocumentPtr CXmlNodeListWrapper::AsDocument()
{
	if (IsValid())
	{
		CXmlDocumentWrapper doc;
		doc.LoadXML(_T("<NodeList></NodeList>"));
		CXmlNodeWrapper root(doc.AsNode());

		for (int i = 0; i < m_xmlnodelist->Getlength(); i++)
		{
			root.InsertNode(root.NumNodes(),m_xmlnodelist->Getitem(i)->cloneNode(VARIANT_TRUE));
		}
		return doc.Interface();
	}
	else
		return NULL;
}

MSXML2::IXMLDOMNodeListPtr CXmlNodeWrapper::FindNodes(LPCTSTR searchStr)
{
	if(IsValid())
	{
		try{
			return m_xmlnode->selectNodes(searchStr);
		}
		catch (_com_error e)
		{
			CString err = e.ErrorMessage();
			return NULL;
		}
	}
	else
		return NULL;
}

CString CXmlNodeWrapper::Name()
{
	if (IsValid())
		return (LPCTSTR )m_xmlnode->GetbaseName();
	return _T("");
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::InsertAfter(MSXML2::IXMLDOMNodePtr refNode, MSXML2::IXMLDOMNodePtr pNode)
{
	MSXML2::IXMLDOMNodePtr nextNode = refNode->GetnextSibling();
	MSXML2::IXMLDOMNodePtr newNode;
	if (nextNode.GetInterfacePtr() != NULL)
		newNode = m_xmlnode->insertBefore(pNode,_variant_t(nextNode.GetInterfacePtr()));
	else
		newNode = m_xmlnode->appendChild(pNode);
	return newNode;
}

void CXmlNodeWrapper::SetText(LPCTSTR text)
{
	if (IsValid())
		m_xmlnode->Puttext(text);
}

CString CXmlNodeWrapper::GetText()
{
	if (IsValid())
		return (LPCTSTR )m_xmlnode->Gettext();
	else
		return _T("");
}

void CXmlNodeWrapper::ReplaceNode(MSXML2::IXMLDOMNodePtr pOldNode, MSXML2::IXMLDOMNodePtr pNewNode)
{
	if (IsValid())
	{
		m_xmlnode->replaceChild(pNewNode,pOldNode);
	}
}

CString CXmlNodeWrapper::NodeType()
{
  if (IsValid())
    return (LPCTSTR )m_xmlnode->GetnodeTypeString();
  return _T("");
}

int CXmlNodeWrapper::NumAttributes()
{
	if (IsValid())
	{
		MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
		if (attribs)
			return attribs->Getlength();
	}
	return 0;
}

CString CXmlNodeWrapper::AName(int index)
{
  if (IsValid())
  {
    MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
    if (attribs)
    {
      MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(index);
      if (attrib)
        return (LPCTSTR )attrib->Getname();
    }
  }
  return _T("");
}


void CXmlNodeWrapper::SetAValue(LPCTSTR valueName, LPCTSTR value)
{
  MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
  if (xmlDocument)
  {
    MSXML2::IXMLDOMNamedNodeMapPtr attributes = m_xmlnode->Getattributes();
    if (attributes)
    {
      MSXML2::IXMLDOMAttributePtr attribute = xmlDocument->createAttribute(valueName);
      if (attribute)
      {
        attribute->Puttext(value);
        attributes->setNamedItem(attribute);
      }
    }
  }
}

void CXmlNodeWrapper::SetAValue(LPCTSTR valueName, int value)
{
  CString str;
  str.Format(_T("%ld"),value);
  SetAValue(valueName,str);
}

void CXmlNodeWrapper::SetAValue(LPCTSTR valueName, short value)
{
  CString str;
  str.Format(_T("%hd"),value);
  SetAValue(valueName,str);
}

void CXmlNodeWrapper::SetAValue(LPCTSTR valueName, double value)
{
  CString str;
  str.Format(_T("%f"),value);
  SetAValue(valueName,str);
}

void CXmlNodeWrapper::SetAValue(LPCTSTR valueName, float value)
{
  CString str;
  str.Format(_T("%f"),value);
  SetAValue(valueName,str);
}

void CXmlNodeWrapper::SetAValue(LPCTSTR valueName, bool value)
{
  CString str;
  if (value)
    str = _T("True");
  else
    str = _T("False");
  SetAValue(valueName,str);
}


CString CXmlNodeWrapper::AValue(int index)
{
  if (IsValid())
  {
    MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
    if (attribs)
    {
      MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(index);
      if (attrib)
        return (LPCTSTR )attrib->Gettext();
    }
  }
  return _T("");
}

CString CXmlNodeWrapper::AValue(LPCTSTR valueName)
{
	if (IsValid())
	{
    MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
    if (attribs)
    {
      if (m_bIgnoreCaseSensitive)
      {
        CString attribName = valueName;
        attribName.MakeUpper();
        CString aName;
        for(int i = 0; i < attribs->Getlength(); i++)
        {
          MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(i);
          if (attrib)
          {
            aName = (LPCTSTR )attrib->Getname();
            aName.MakeUpper();
            if ( aName == attribName )
              return (LPCTSTR )attrib->Gettext();
          }
        }
      }
      else
      {
        MSXML2::IXMLDOMNodePtr attribute = attribs->getNamedItem(valueName);
        if (attribute)
        {
          return (LPCTSTR )attribute->Gettext();
        }
      }
    }
	}
	return _T("");
}


int    CXmlNodeWrapper::AValueInt   (LPCTSTR valueName)
{
  return _tstoi(AValue(valueName));
}

double CXmlNodeWrapper::AValueDouble(LPCTSTR valueName)
{
  return _tstof(AValue(valueName));
}

bool     CXmlNodeWrapper::AValueBool(LPCTSTR valueName)
{
  return !AValue(valueName).CompareNoCase(_T("True"));
}

MSXML2::IXMLDOMNodePtr CXmlNodeWrapper::FindNodeAValue(LPCTSTR nodeName, LPCTSTR attrName, LPCTSTR attrValue, BOOL bIgnoreCase)
{
  if (IsValid())
  {
    MSXML2::IXMLDOMNodeListPtr nodeList = m_xmlnode->selectNodes(nodeName);
    for (int i = 0; i < nodeList->Getlength(); i++)
    {
      try
      {
        MSXML2::IXMLDOMNodePtr pNode = nodeList->Getitem(i);
        CXmlNodeWrapper n(pNode);
        if (bIgnoreCase)
        {
          if (!n.AValue(attrName).CompareNoCase(attrValue))
            return pNode;
        }
        else
        {
          if (n.AValue(attrName) == attrValue)
            return pNode;
        }
      }
      catch (_com_error er)
      {
        AfxMessageBox(er.ErrorMessage());
      }
    }
  }
  return NULL;
}

void CXmlNodeWrapper::ARemove(LPCTSTR attribName)
{
  if (IsValid())
  {
    MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
    if (attribs)
    {
      CString aName = attribName;
      if (m_bIgnoreCaseSensitive)
      {
        for(int i = 0; i < attribs->Getlength(); i++)
        {
          MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(i);
          if (attrib)
          {
            CString sName = (LPCTSTR)attrib->Getname();
            if (aName.CompareNoCase(sName) < 0)
            {
              aName = sName;
              break;
            }
          }
        }
      }
      attribs->removeNamedItem((LPCTSTR)aName);
    }
  }
}

