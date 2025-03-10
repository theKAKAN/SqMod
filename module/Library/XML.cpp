// ------------------------------------------------------------------------------------------------
#include "Library/XML.hpp"

// ------------------------------------------------------------------------------------------------
#include <sqratConst.h>

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
SQMOD_DECL_TYPENAME(XmlParseResultTypename, _SC("SqXmlParseResult"))
SQMOD_DECL_TYPENAME(XmlDocumentTypename, _SC("SqXmlDocument"))
SQMOD_DECL_TYPENAME(XmlNodeTypename, _SC("SqXmlNode"))
SQMOD_DECL_TYPENAME(XmlAttributeTypename, _SC("SqXmlAttribute"))
SQMOD_DECL_TYPENAME(XmlTextTypename, _SC("SqXmlText"))

// ------------------------------------------------------------------------------------------------
XmlNode XmlDocument::GetNode() const
{
    // Validate the document handle
    m_Doc.Validate();
    // Return the requested information
    return XmlNode(m_Doc, m_Doc->document_element());
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::GetFirstAttr() const
{
    return XmlAttribute(m_Doc, m_Node.first_attribute());
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::GetLastAttr() const
{
    return XmlAttribute(m_Doc, m_Node.last_attribute());
}

// ------------------------------------------------------------------------------------------------
XmlText XmlNode::GetText() const
{
    return XmlText(m_Doc, m_Node.text());
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::GetAttribute(StackStrF & name) const
{
    return XmlAttribute(m_Doc, m_Node.attribute(name.mPtr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::AttributeFrom(StackStrF & name, XmlAttribute & attr) const
{
    return XmlAttribute(m_Doc, m_Node.attribute(name.mPtr, attr.m_Attr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::AppendAttr(StackStrF & name)
{
    return XmlAttribute(m_Doc, m_Node.append_attribute(name.mPtr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::PrependAttr(StackStrF & name)
{
    return XmlAttribute(m_Doc, m_Node.prepend_attribute(name.mPtr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::InsertAttrAfter(StackStrF & name, const XmlAttribute & attr)
{
    return XmlAttribute(m_Doc, m_Node.insert_attribute_after(name.mPtr, attr.m_Attr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::InsertAttrBefore(StackStrF & name, const XmlAttribute & attr)
{
    return XmlAttribute(m_Doc, m_Node.insert_attribute_before(name.mPtr, attr.m_Attr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::AppendAttrCopy(const XmlAttribute & proto)
{
    return XmlAttribute(m_Doc, m_Node.append_copy(proto.m_Attr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::PrependAttrCopy(const XmlAttribute & proto)
{
    return XmlAttribute(m_Doc, m_Node.prepend_copy(proto.m_Attr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::InsertAttrCopyAfter(const XmlAttribute & proto, const XmlAttribute & attr)
{
    return XmlAttribute(m_Doc, m_Node.insert_copy_after(proto.m_Attr, attr.m_Attr));
}

// ------------------------------------------------------------------------------------------------
XmlAttribute XmlNode::InsertAttrCopyBefore(const XmlAttribute & proto, const XmlAttribute & attr)
{
    return XmlAttribute(m_Doc, m_Node.insert_copy_before(proto.m_Attr, attr.m_Attr));
}

// ------------------------------------------------------------------------------------------------
bool XmlNode::RemoveAttrInst(const XmlAttribute & attr)
{
    return m_Node.remove_attribute(attr.m_Attr);
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlAttribute::AsLong(SQInteger def) const
{
    return m_Attr.as_llong(def);
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlAttribute::AsUlong(SQInteger def) const
{
    return static_cast< SQInteger >(m_Attr.as_ullong(static_cast< uint64_t >(def)));
}

// ------------------------------------------------------------------------------------------------
bool XmlAttribute::ApplyLong(SQInteger value)
{
    return m_Attr.set_value(value);
}

// ------------------------------------------------------------------------------------------------
bool XmlAttribute::ApplyUlong(SQInteger value)
{
    return m_Attr.set_value(value);
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlAttribute::GetLong() const
{
    return m_Attr.as_llong();
}

// ------------------------------------------------------------------------------------------------
void XmlAttribute::SetLong(SQInteger value)
{
    m_Attr = value;
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlAttribute::GetUlong() const
{
    return static_cast< SQInteger >(m_Attr.as_ullong());
}

// ------------------------------------------------------------------------------------------------
void XmlAttribute::SetUlong(SQInteger value)
{
    m_Attr = static_cast< uint64_t >(value);
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlText::AsLong(SQInteger def) const
{
    return m_Text.as_llong(def);
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlText::AsUlong(SQInteger def) const
{
    return static_cast< SQInteger >(m_Text.as_ullong(static_cast< uint64_t >(def)));
}

// ------------------------------------------------------------------------------------------------
bool XmlText::ApplyLong(SQInteger value)
{
    return m_Text.set(value);
}

// ------------------------------------------------------------------------------------------------
bool XmlText::ApplyUlong(SQInteger value)
{
    return m_Text.set(static_cast< uint64_t >(value));
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlText::GetLong() const
{
    return m_Text.as_llong();
}

// ------------------------------------------------------------------------------------------------
void XmlText::SetLong(SQInteger value)
{
    m_Text = value;
}

// ------------------------------------------------------------------------------------------------
SQInteger XmlText::GetUlong() const
{
    return static_cast< SQInteger >(m_Text.as_ullong());
}

// ------------------------------------------------------------------------------------------------
void XmlText::SetUlong(SQInteger value)
{
    m_Text = static_cast< uint64_t >(value);
}

// ------------------------------------------------------------------------------------------------
XmlNode XmlText::GetData() const
{
    return XmlNode(m_Doc, m_Text.data());
}

// ================================================================================================
void Register_XML(HSQUIRRELVM vm)
{
    Table xmlns(vm);

    xmlns.Bind(_SC("ParseResult"), Class< XmlParseResult >(vm, XmlParseResultTypename::Str)
        // Constructors
        .Ctor()
        .Ctor< const XmlParseResult & >()
        // Core Meta-methods
        .Func(_SC("_cmp"), &XmlParseResult::Cmp)
        .SquirrelFunc(_SC("_typename"), &XmlParseResultTypename::Fn)
        .Func(_SC("_tostring"), &XmlParseResult::ToString)
        // Properties
        .Prop(_SC("Valid"), &XmlParseResult::IsValid)
        .Prop(_SC("References"), &XmlParseResult::GetRefCount)
        .Prop(_SC("Ok"), &XmlParseResult::IsOk)
        .Prop(_SC("Status"), &XmlParseResult::GetStatus)
        .Prop(_SC("Offset"), &XmlParseResult::GetOffset)
        .Prop(_SC("Encoding"), &XmlParseResult::GetEncoding)
        .Prop(_SC("Description"), &XmlParseResult::GetDescription)
        // Member Methods
        .Func(_SC("Check"), &XmlParseResult::Check)
    );

    xmlns.Bind(_SC("Attribute"), Class< XmlAttribute >(vm, XmlAttributeTypename::Str)
        // Constructors
        .Ctor()
        .Ctor< const XmlAttribute & >()
        // Core Meta-methods
        .Func(_SC("_cmp"), &XmlAttribute::Cmp)
        .SquirrelFunc(_SC("_typename"), &XmlAttributeTypename::Fn)
        .Func(_SC("_tostring"), &XmlAttribute::ToString)
        // Properties
        .Prop(_SC("Valid"), &XmlAttribute::IsValid)
        .Prop(_SC("References"), &XmlAttribute::GetRefCount)
        .Prop(_SC("Empty"), &XmlAttribute::IsEmpty)
        .Prop(_SC("Hash"), &XmlAttribute::GetHashValue)
        .Prop(_SC("Name"), &XmlAttribute::GetName, &XmlAttribute::SetName)
        .Prop(_SC("Value"), &XmlAttribute::GetValue, &XmlAttribute::SetValue)
        .Prop(_SC("Int"), &XmlAttribute::GetInt, &XmlAttribute::SetInt)
        .Prop(_SC("Uint"), &XmlAttribute::GetUint, &XmlAttribute::SetUint)
        .Prop(_SC("Float"), &XmlAttribute::GetFloat, &XmlAttribute::SetFloat)
        .Prop(_SC("Double"), &XmlAttribute::GetDouble, &XmlAttribute::SetDouble)
        .Prop(_SC("Long"), &XmlAttribute::GetLong, &XmlAttribute::SetLong)
        .Prop(_SC("Ulong"), &XmlAttribute::GetUlong, &XmlAttribute::SetUlong)
        .Prop(_SC("Bool"), &XmlAttribute::GetBool, &XmlAttribute::SetBool)
        .Prop(_SC("Next"), &XmlAttribute::NextAttribute)
        .Prop(_SC("Prev"), &XmlAttribute::PrevAttribute)
        // Member Methods
        .Func(_SC("SetName"), &XmlAttribute::ApplyName)
        .Func(_SC("SetValue"), &XmlAttribute::ApplyValue)
        .Func(_SC("AsString"), &XmlAttribute::AsString)
        .Func(_SC("AsInt"), &XmlAttribute::AsInt)
        .Func(_SC("AsUint"), &XmlAttribute::AsUint)
        .Func(_SC("AsFloat"), &XmlAttribute::AsFloat)
        .Func(_SC("AsDouble"), &XmlAttribute::AsDouble)
        .Func(_SC("AsLong"), &XmlAttribute::AsLong)
        .Func(_SC("AsUlong"), &XmlAttribute::AsUlong)
        .Func(_SC("AsBool"), &XmlAttribute::AsBool)
        .Func(_SC("SetString"), &XmlAttribute::ApplyString)
        .Func(_SC("SetInt"), &XmlAttribute::ApplyInt)
        .Func(_SC("SetUint"), &XmlAttribute::ApplyUint)
        .Func(_SC("SetFloat"), &XmlAttribute::ApplyFloat)
        .Func(_SC("SetDouble"), &XmlAttribute::ApplyDouble)
        .Func(_SC("SetLong"), &XmlAttribute::ApplyLong)
        .Func(_SC("SetUlong"), &XmlAttribute::ApplyUlong)
        .Func(_SC("SetBool"), &XmlAttribute::ApplyBool)
    );

    xmlns.Bind(_SC("Text"), Class< XmlText >(vm, XmlTextTypename::Str)
        // Constructors
        .Ctor()
        .Ctor< const XmlText & >()
        // Core Meta-methods
        .Func(_SC("_cmp"), &XmlText::Cmp)
        .SquirrelFunc(_SC("_typename"), &XmlTextTypename::Fn)
        .Func(_SC("_tostring"), &XmlText::ToString)
        // Properties
        .Prop(_SC("Valid"), &XmlText::IsValid)
        .Prop(_SC("References"), &XmlText::GetRefCount)
        .Prop(_SC("Empty"), &XmlText::IsEmpty)
        .Prop(_SC("Value"), &XmlText::GetValue)
        .Prop(_SC("Int"), &XmlText::GetInt, &XmlText::SetInt)
        .Prop(_SC("Uint"), &XmlText::GetUint, &XmlText::SetUint)
        .Prop(_SC("Float"), &XmlText::GetFloat, &XmlText::SetFloat)
        .Prop(_SC("Double"), &XmlText::GetDouble, &XmlText::SetDouble)
        .Prop(_SC("Long"), &XmlText::GetLong, &XmlText::SetLong)
        .Prop(_SC("Ulong"), &XmlText::GetUlong, &XmlText::SetUlong)
        .Prop(_SC("Bool"), &XmlText::GetBool, &XmlText::SetBool)
        .Prop(_SC("Data"), &XmlText::GetData)
        // Member Methods
        .Func(_SC("AsString"), &XmlText::AsString)
        .Func(_SC("AsInt"), &XmlText::AsInt)
        .Func(_SC("AsUint"), &XmlText::AsUint)
        .Func(_SC("AsFloat"), &XmlText::AsFloat)
        .Func(_SC("AsDouble"), &XmlText::AsDouble)
        .Func(_SC("AsLong"), &XmlText::AsLong)
        .Func(_SC("AsUlong"), &XmlText::AsUlong)
        .Func(_SC("AsBool"), &XmlText::AsBool)
        .Func(_SC("SetString"), &XmlText::ApplyString)
        .Func(_SC("SetInt"), &XmlText::ApplyInt)
        .Func(_SC("SetUint"), &XmlText::ApplyUint)
        .Func(_SC("SetFloat"), &XmlText::ApplyFloat)
        .Func(_SC("SetDouble"), &XmlText::ApplyDouble)
        .Func(_SC("SetLong"), &XmlText::ApplyLong)
        .Func(_SC("SetUlong"), &XmlText::ApplyUlong)
        .Func(_SC("SetBool"), &XmlText::ApplyBool)
    );

    xmlns.Bind(_SC("Node"), Class< XmlNode >(vm, XmlNodeTypename::Str)
        // Constructors
        .Ctor()
        .Ctor< const XmlNode & >()
        // Core Meta-methods
        .Func(_SC("_cmp"), &XmlNode::Cmp)
        .SquirrelFunc(_SC("_typename"), &XmlNodeTypename::Fn)
        .Func(_SC("_tostring"), &XmlNode::ToString)
        // Properties
        .Prop(_SC("Valid"), &XmlNode::IsValid)
        .Prop(_SC("References"), &XmlNode::GetRefCount)
        .Prop(_SC("Empty"), &XmlNode::IsEmpty)
        .Prop(_SC("Hash"), &XmlNode::GetHashValue)
        .Prop(_SC("OffsetDebug"), &XmlNode::GetOffsetDebug)
        .Prop(_SC("Type"), &XmlNode::GetType)
        .Prop(_SC("Name"), &XmlNode::GetName, &XmlNode::SetName)
        .Prop(_SC("Value"), &XmlNode::GetValue, &XmlNode::SetValue)
        .Prop(_SC("FirstAttr"), &XmlNode::GetFirstAttr)
        .Prop(_SC("LastAttr"), &XmlNode::GetLastAttr)
        .Prop(_SC("FirstChild"), &XmlNode::GetFirstChild)
        .Prop(_SC("LastChild"), &XmlNode::GetLastChild)
        .Prop(_SC("NextSibling"), &XmlNode::GetNextSibling)
        .Prop(_SC("PrevSibling"), &XmlNode::GetPrevSibling)
        .Prop(_SC("Parent"), &XmlNode::GetParent)
        .Prop(_SC("Root"), &XmlNode::GetRoot)
        .Prop(_SC("Text"), &XmlNode::GetText)
        .Prop(_SC("ChildValue"), &XmlNode::GetChildValue)
        // Member Methods
        .Overload(_SC("AppendBuffer"), &XmlNode::AppendBuffer1)
        .Overload(_SC("AppendBuffer"), &XmlNode::AppendBuffer2)
        .Overload(_SC("AppendBuffer"), &XmlNode::AppendBuffer3)
        .Func(_SC("SetName"), &XmlNode::ApplyName)
        .Func(_SC("SetValue"), &XmlNode::ApplyValue)
        .Func(_SC("GetChild"), &XmlNode::Child)
        .Func(_SC("GetAttr"), &XmlNode::GetAttribute)
        .Func(_SC("GetAttribute"), &XmlNode::GetAttribute)
        .Func(_SC("GetAttrFrom"), &XmlNode::AttributeFrom)
        .Func(_SC("GetAttributeFrom"), &XmlNode::AttributeFrom)
        .Func(_SC("GetNextSibling"), &XmlNode::NextSibling)
        .Func(_SC("GetPrevSibling"), &XmlNode::PrevSibling)
        .Func(_SC("GetChildValue"), &XmlNode::ChildValue)
        .Func(_SC("AppendAttr"), &XmlNode::AppendAttr)
        .Func(_SC("PrependAttr"), &XmlNode::PrependAttr)
        .Func(_SC("InsertAttrAfter"), &XmlNode::InsertAttrAfter)
        .Func(_SC("InsertAttrBefore"), &XmlNode::InsertAttrBefore)
        .Func(_SC("AppendAttrCopy"), &XmlNode::AppendAttrCopy)
        .Func(_SC("PrependAttrCopy"), &XmlNode::PrependAttrCopy)
        .Func(_SC("InsertAttrCopyAfter"), &XmlNode::InsertAttrCopyAfter)
        .Func(_SC("InsertAttrCopyBefore"), &XmlNode::InsertAttrCopyBefore)
        .Func(_SC("AppendChild"), &XmlNode::AppendChild)
        .Func(_SC("PrependChild"), &XmlNode::PrependChild)
        .Func(_SC("AppendChildNode"), &XmlNode::AppendChildNode)
        .Func(_SC("PrependChildNode"), &XmlNode::PrependChildNode)
        .Func(_SC("AppendChildType"), &XmlNode::AppendChildType)
        .Func(_SC("PrependChildType"), &XmlNode::PrependChildType)
        .Func(_SC("InsertChildAfter"), &XmlNode::InsertChildAfter)
        .Func(_SC("InsertChildBefore"), &XmlNode::InsertChildBefore)
        .Func(_SC("InsertChildTypeAfter"), &XmlNode::InsertChildTypeAfter)
        .Func(_SC("InsertChildTypeBefore"), &XmlNode::InsertChildTypeBefore)
        .Func(_SC("AppendCopy"), &XmlNode::AppendCopy)
        .Func(_SC("PrependCopy"), &XmlNode::PrependCopy)
        .Func(_SC("InsertCopyAfter"), &XmlNode::InsertCopyAfter)
        .Func(_SC("InsertCopyBefore"), &XmlNode::InsertCopyBefore)
        .Func(_SC("AppendMove"), &XmlNode::AppendMove)
        .Func(_SC("PrependMove"), &XmlNode::PrependMove)
        .Func(_SC("InsertMoveAfter"), &XmlNode::InsertMoveAfter)
        .Func(_SC("InsertMoveBefore"), &XmlNode::InsertMoveBefore)
        .Func(_SC("RemoveAttr"), &XmlNode::RemoveAttr)
        .Func(_SC("RemoveAttrInst"), &XmlNode::RemoveAttrInst)
        .Func(_SC("RemoveChild"), &XmlNode::RemoveChild)
        .Func(_SC("RemoveChildInst"), &XmlNode::RemoveChildInst)
        .Overload(_SC("FindChildByAttr"), &XmlNode::FindChildByAttr2)
        .Overload(_SC("FindChildByAttr"), &XmlNode::FindChildByAttr3)
        .Func(_SC("FindElemByPath"), &XmlNode::FindElemByPath)
    );

    xmlns.Bind(_SC("Document"), Class< XmlDocument, NoCopy< XmlDocument > >(vm, XmlDocumentTypename::Str)
        // Constructors
        .Ctor()
        // Core Meta-methods
        .Func(_SC("_cmp"), &XmlDocument::Cmp)
        .SquirrelFunc(_SC("_typename"), &XmlDocumentTypename::Fn)
        .Func(_SC("_tostring"), &XmlDocument::ToString)
        // Properties
        .Prop(_SC("Valid"), &XmlDocument::IsValid)
        .Prop(_SC("References"), &XmlDocument::GetRefCount)
        .Prop(_SC("Node"), &XmlDocument::GetNode)
        // Member Methods
        .Overload(_SC("Reset"), &XmlDocument::Reset0)
        .Overload(_SC("Reset"), &XmlDocument::Reset1)
        .Overload(_SC("LoadString"), &XmlDocument::LoadData1)
        .Overload(_SC("LoadString"), &XmlDocument::LoadData2)
        .Overload(_SC("LoadFile"), &XmlDocument::LoadFile1)
        .Overload(_SC("LoadFile"), &XmlDocument::LoadFile2)
        .Overload(_SC("LoadFile"), &XmlDocument::LoadFile3)
        .Overload(_SC("SaveFile"), &XmlDocument::SaveFile1)
        .Overload(_SC("SaveFile"), &XmlDocument::SaveFile1)
        .Overload(_SC("SaveFile"), &XmlDocument::SaveFile2)
        .Overload(_SC("SaveFile"), &XmlDocument::SaveFile3)
        .Overload(_SC("SaveFile"), &XmlDocument::SaveFile4)
    );

    RootTable(vm).Bind(_SC("SqXml"), xmlns);

    ConstTable(vm).Enum(_SC("SqXmlNodeType"), Enumeration(vm)
        .Const(_SC("Null"),                     static_cast< int32_t >(node_null))
        .Const(_SC("XmlDocument"),              static_cast< int32_t >(node_document))
        .Const(_SC("Element"),                  static_cast< int32_t >(node_element))
        .Const(_SC("PCData"),                   static_cast< int32_t >(node_pcdata))
        .Const(_SC("CData"),                    static_cast< int32_t >(node_cdata))
        .Const(_SC("Comment"),                  static_cast< int32_t >(node_comment))
        .Const(_SC("Pi"),                       static_cast< int32_t >(node_pi))
        .Const(_SC("Declaration"),              static_cast< int32_t >(node_declaration))
        .Const(_SC("Doctype"),                  static_cast< int32_t >(node_doctype))
    );

    ConstTable(vm).Enum(_SC("SqXmlParse"), Enumeration(vm)
        .Const(_SC("Minimal"),                  static_cast< int32_t >(parse_minimal))
        .Const(_SC("Default"),                  static_cast< int32_t >(parse_default))
        .Const(_SC("Full"),                     static_cast< int32_t >(parse_full))
        .Const(_SC("Pi"),                       static_cast< int32_t >(parse_pi))
        .Const(_SC("Comments"),                 static_cast< int32_t >(parse_comments))
        .Const(_SC("CData"),                    static_cast< int32_t >(parse_cdata))
        .Const(_SC("WSPCData"),                 static_cast< int32_t >(parse_ws_pcdata))
        .Const(_SC("Escapes"),                  static_cast< int32_t >(parse_escapes))
        .Const(_SC("EOL"),                      static_cast< int32_t >(parse_eol))
        .Const(_SC("WConvAttribute"),           static_cast< int32_t >(parse_wconv_attribute))
        .Const(_SC("WNormAttribute"),           static_cast< int32_t >(parse_wnorm_attribute))
        .Const(_SC("Declaration"),              static_cast< int32_t >(parse_declaration))
        .Const(_SC("Doctype"),                  static_cast< int32_t >(parse_doctype))
        .Const(_SC("WSPCDataSingle"),           static_cast< int32_t >(parse_ws_pcdata_single))
        .Const(_SC("TrimPCData"),               static_cast< int32_t >(parse_trim_pcdata))
        .Const(_SC("Fragment"),                 static_cast< int32_t >(parse_fragment))
        .Const(_SC("EmbedPCData"),              static_cast< int32_t >(parse_embed_pcdata))
    );

    ConstTable(vm).Enum(_SC("SqXmlEncoding"), Enumeration(vm)
        .Const(_SC("Auto"),                     static_cast< int32_t >(encoding_auto))
        .Const(_SC("Utf8"),                     static_cast< int32_t >(encoding_utf8))
        .Const(_SC("Utf16LE"),                  static_cast< int32_t >(encoding_utf16_le))
        .Const(_SC("Utf16BE"),                  static_cast< int32_t >(encoding_utf16_be))
        .Const(_SC("Utf16"),                    static_cast< int32_t >(encoding_utf16))
        .Const(_SC("Utf32LE"),                  static_cast< int32_t >(encoding_utf32_le))
        .Const(_SC("Utf32BE"),                  static_cast< int32_t >(encoding_utf32_be))
        .Const(_SC("Utf32"),                    static_cast< int32_t >(encoding_utf32))
        .Const(_SC("WChar"),                    static_cast< int32_t >(encoding_wchar))
        .Const(_SC("Latin1"),                   static_cast< int32_t >(encoding_latin1))
    );

    ConstTable(vm).Enum(_SC("SqXmlFormat"), Enumeration(vm)
        .Const(_SC("Indent"),                   static_cast< int32_t >(format_indent))
        .Const(_SC("WriteBOM"),                 static_cast< int32_t >(format_write_bom))
        .Const(_SC("Raw"),                      static_cast< int32_t >(format_raw))
        .Const(_SC("NoDeclaration"),            static_cast< int32_t >(format_no_declaration))
        .Const(_SC("NoEscapes"),                static_cast< int32_t >(format_no_escapes))
        .Const(_SC("SaveFileText"),             static_cast< int32_t >(format_save_file_text))
        .Const(_SC("IndentAttributes"),         static_cast< int32_t >(format_indent_attributes))
        .Const(_SC("Default"),                  static_cast< int32_t >(format_default))
    );

    ConstTable(vm).Enum(_SC("SqXmlParseStatus"), Enumeration(vm)
        .Const(_SC("Ok"),                       static_cast< int32_t >(status_ok))
        .Const(_SC("FileNotFound"),             static_cast< int32_t >(status_file_not_found))
        .Const(_SC("IOError"),                  static_cast< int32_t >(status_io_error))
        .Const(_SC("OutOfMemory"),              static_cast< int32_t >(status_out_of_memory))
        .Const(_SC("InternalError"),            static_cast< int32_t >(status_internal_error))
        .Const(_SC("UnrecognizedTag"),          static_cast< int32_t >(status_unrecognized_tag))
        .Const(_SC("BadPi"),                    static_cast< int32_t >(status_bad_pi))
        .Const(_SC("BadComment"),               static_cast< int32_t >(status_bad_comment))
        .Const(_SC("BadCData"),                 static_cast< int32_t >(status_bad_cdata))
        .Const(_SC("BadDoctype"),               static_cast< int32_t >(status_bad_doctype))
        .Const(_SC("BadPCData"),                static_cast< int32_t >(status_bad_pcdata))
        .Const(_SC("BadStartElement"),          static_cast< int32_t >(status_bad_start_element))
        .Const(_SC("BadAttribute"),             static_cast< int32_t >(status_bad_attribute))
        .Const(_SC("BadEndElement"),            static_cast< int32_t >(status_bad_end_element))
        .Const(_SC("EndElementMismatch"),       static_cast< int32_t >(status_end_element_mismatch))
        .Const(_SC("AppendInvalidRoot"),        static_cast< int32_t >(status_append_invalid_root))
        .Const(_SC("NoDocumentElement"),        static_cast< int32_t >(status_no_document_element))
    );

    ConstTable(vm).Enum(_SC("SqXmlXpathValueType"), Enumeration(vm)
        .Const(_SC("None"),                     static_cast< int32_t >(xpath_type_none))
        .Const(_SC("NodeSet"),                  static_cast< int32_t >(xpath_type_node_set))
        .Const(_SC("Number"),                   static_cast< int32_t >(xpath_type_number))
        .Const(_SC("String"),                   static_cast< int32_t >(xpath_type_string))
        .Const(_SC("Boolean"),                  static_cast< int32_t >(xpath_type_boolean))
    );
}

} // Namespace:: SqMod
