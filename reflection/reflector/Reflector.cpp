#include "reflection/Field.h"
#include "reflection/Type.h"
#include "io/FileStream.h"
#include "io/StreamWriter.h"
#include "rdestl/fixed_vector.h"
#include "rdestl/hash_map.h"
#include "rdestl/string.h"
#include "rdestl/vector.h"
#include "core/RefCounted.h"
#include "core/OwnedPtr.h"
#include "core/RefPtr.h"
#include <dia2.h>
#include <cstdio>

namespace
{
struct FieldDescriptor;

typedef rde::fixed_substring<char, 128>	StrType;

const StrType s_initVTableFunc("Reflection_InitVTable");
const StrType s_createInstanceFunc("Reflection_CreateInstance");

struct EnumElement
{
	EnumElement(): m_value(LONG_MAX) {}
	EnumElement(const StrType& name, long value): m_name(name), m_value(value) {}

	StrType	m_name;
	long	m_value;
};
struct TypeDescriptor : public rde::RefCounted<TypeDescriptor>
{
	enum
	{
		FLAG_NEEDS_VTABLE	= RDE_BIT(0),
		// Type has been registered, but not fully reflected (this can happen for
		// members of classes from to-reflect list, that are not on the list themselves).
		FLAG_INCOMPLETE		= RDE_BIT(1),
	};

	TypeDescriptor()
	:	m_size(0), 
		m_baseClassOffset(0), 
		m_pfnInitVTable(0), 
		m_pfnCreateInstance(0), 
		m_flags(FLAG_INCOMPLETE)
	{}
	~TypeDescriptor();

	bool HasField(const StrType& name) const;
	bool AddField(FieldDescriptor*);
	// @pre	m_reflectionType == rde::ReflectionType::ENUM
	void AddEnumElement(const EnumElement& enumElement)
	{
		RDE_ASSERT(m_reflectionType == rde::ReflectionType::ENUM);
		m_enumElements.push_back(enumElement);
	}

	void WriteFields(rde::StreamWriter& sw, bool hashesOnly) const;
	void WriteTypeInfo(rde::StreamWriter& sw, bool hashesOnly) const
	{
		if ((m_flags & FLAG_NEEDS_VTABLE) && m_pfnInitVTable == 0)
		{
			printf("*** WARNING: Type '%s' has no vtable init function (%s).\n", 
				m_name.c_str(), s_initVTableFunc.data());
		}

		if (hashesOnly)
			sw.WriteInt32(rde::CRC32::GetValue(m_name.c_str()));
		else
			sw.WriteASCIIZ(m_name.c_str());
		sw.WriteInt32((long)m_size);
		sw.WriteInt32(m_reflectionType);

		if (m_reflectionType == rde::ReflectionType::CLASS)
		{
			sw.WriteInt32(rde::CRC32::GetValue(m_baseClassName.c_str()));
			sw.WriteInt16(m_baseClassOffset);
			sw.WriteInt32(m_pfnCreateInstance);
			sw.WriteInt32(m_pfnInitVTable);
			WriteFields(sw, hashesOnly);
		}
		else if (m_reflectionType == rde::ReflectionType::ENUM)
		{
			sw.WriteInt32(m_enumElements.size());
			for (EnumElements::const_iterator it = m_enumElements.begin(); it != m_enumElements.end(); ++it)
			{
				sw.WriteASCIIZ(it->m_name.c_str());
				sw.WriteInt32(it->m_value);
			}
		}
		else if (m_reflectionType == rde::ReflectionType::ARRAY)
		{
			sw.WriteInt32(rde::CRC32::GetValue(m_dependentTypeName.c_str()));
			sw.WriteInt32(m_numElements);
		}
		else if (m_reflectionType == rde::ReflectionType::POINTER)
		{
			sw.WriteInt32(rde::CRC32::GetValue(m_dependentTypeName.c_str()));
		}
		else
		{
			RDE_ASSERT(!"Unknown reflection type.");
		}
	}

	void PrintDebugInfo() const
	{
		if (m_reflectionType == rde::ReflectionType::CLASS)
		{
			if (m_baseClassName.empty())
			{
				printf("Type: %s, size: %d byte(s), no base class.\n", m_name.c_str(), m_size);
			}
			else
			{
				printf("Type: %s, size: %d byte(s), base class: %s, base class offset: %d.\n", m_name.c_str(),
					m_size, m_baseClassName.c_str(), m_baseClassOffset);
			}
			printf("Create instance function address: 0x%X\n", m_pfnCreateInstance);
			printf("Init vtable function address: 0x%X\n", m_pfnInitVTable);
			PrintFieldsDebugInfo();
		}
		else if (m_reflectionType == rde::ReflectionType::POINTER)
		{
			printf("Pointer type: %s.\n", m_name.c_str());
		}
		else if (m_reflectionType == rde::ReflectionType::ARRAY)
		{
			printf("Array type: %s.\n", m_name.c_str());
		}
		else if (m_reflectionType == rde::ReflectionType::ENUM)
		{
			printf("Enum type: %s (%d enumerators).\n", m_name.c_str(), m_enumElements.size());
		}
		else if (m_reflectionType != rde::ReflectionType::FUNDAMENTAL)
			printf("Unknown reflection type: %d\n", m_reflectionType);
	}
	void PrintFieldsDebugInfo() const;

	typedef rde::OwnedPtr<FieldDescriptor>	FieldPtr;
	typedef rde::vector<FieldPtr>			Fields;
	typedef rde::vector<EnumElement>		EnumElements;
	StrType						m_name;
	size_t						m_size;
	rde::ReflectionType::Enum	m_reflectionType;
	StrType						m_baseClassName;
	rde::uint16_t				m_baseClassOffset;
	rde::uint32_t				m_numElements;	// array
	// Array/pointer (contained/pointed type name)
	StrType						m_dependentTypeName;
	EnumElements				m_enumElements;
	Fields						m_fields;
	rde::uint32_t				m_pfnInitVTable;
	rde::uint32_t				m_pfnCreateInstance;

	rde::uint32_t				m_flags;
};
struct FieldDescriptor
{
	FieldDescriptor(): m_offset(0), m_flags(0) {}

	void Write(rde::StreamWriter& sw, bool hashesOnly) const
	{
		sw.WriteInt32(rde::CRC32::GetValue(m_typeName.c_str()));
		sw.WriteInt16(m_offset);
		sw.WriteInt16(m_flags);
		if (hashesOnly)
			sw.WriteInt32(rde::CRC32::GetValue(m_name.c_str()));
		else
			sw.WriteASCIIZ(m_name.c_str());
	}
	void PrintDebugInfo() const
	{
		printf("Name: %s, type name: %s, offset: %d, flags: 0x%X.\n", m_name.c_str(), 
			m_typeName.c_str(), m_offset, m_flags);
	}

	StrType			m_typeName;
	rde::uint16_t	m_offset;
	rde::uint16_t	m_flags;
	StrType			m_name;
};

TypeDescriptor::~TypeDescriptor()
{
}
bool TypeDescriptor::HasField(const StrType& name) const
{
	for (int i = 0; i < m_fields.size(); ++i)
	{
		if (name == m_fields[i]->m_name)
			return true;
	}
	return false;
}
bool TypeDescriptor::AddField(FieldDescriptor* field)
{
	if (HasField(field->m_name))
		return false;
	m_fields.push_back(field);
	return true;
}

void TypeDescriptor::WriteFields(rde::StreamWriter& sw, bool hashesOnly) const
{
	sw.WriteInt32(m_fields.size());
	for (int i = 0; i < m_fields.size(); ++i)
		m_fields[i]->Write(sw, hashesOnly);
}
void TypeDescriptor::PrintFieldsDebugInfo() const
{
	if (m_fields.size() > 0)
	{
		printf("Fields:\n");
		for (int i = 0; i < m_fields.size(); ++i)
		{
			printf("\t");
			m_fields[i]->PrintDebugInfo();
		}
	}
}

typedef rde::RefPtr<TypeDescriptor>					TypeDescPtr;
typedef rde::hash_map<rde::uint32_t, TypeDescPtr>	TypeMap;
TypeMap	s_typeDescriptors;

TypeDescriptor* FindTypeDescriptor(const StrType& name)
{
	const rde::uint32_t id = rde::CRC32::GetValue(name.c_str());
	TypeMap::iterator it = s_typeDescriptors.find(id);
	return it == s_typeDescriptors.end() ? 0 : it->second.GetPtr();
}
TypeDescriptor* AddTypeDescriptor(const StrType& name, rde::ReflectionType::Enum reflectionType,
	size_t typeSize)
{
	TypeDescriptor* desc = FindTypeDescriptor(name);
	if (desc == 0)
	{
		desc = new TypeDescriptor();
		desc->m_name = name;
		desc->m_reflectionType = reflectionType;
		desc->m_size = typeSize;
		const rde::uint32_t nameId = rde::CRC32::GetValue(name.c_str());
		s_typeDescriptors.insert(rde::make_pair(nameId, TypeDescPtr(desc)));
		if (reflectionType != rde::ReflectionType::CLASS)
			desc->m_flags &= ~TypeDescriptor::FLAG_INCOMPLETE;
	}
	return desc;
}
bool HasAnyIncompleteTypes()
{
	for (TypeMap::const_iterator it = s_typeDescriptors.begin(); it != s_typeDescriptors.end(); ++it)
	{
		if (it->second->m_flags & TypeDescriptor::FLAG_INCOMPLETE)
			return true;
	}
	return false;
}
void PrintAllTypes()
{
	printf("\n* Reflected types:\n------------------\n");
	for (TypeMap::const_iterator it = s_typeDescriptors.begin(); it != s_typeDescriptors.end(); ++it)
		it->second->PrintDebugInfo();
}

void SaveReflectionInfo(rde::Stream* stream, bool hashesOnly)
{
	rde::StreamWriter sw(stream);
	const long numTypesOffset = stream->GetPosition();
	sw.WriteInt32(0);	// Prepare 'slot' for number of types
	int numTypes(0);
	for (TypeMap::iterator it = s_typeDescriptors.begin(); it != s_typeDescriptors.end(); ++it)
	{
		const TypeDescPtr& desc = it->second;
		
		// No need to save fundamental types, they don't change.
		if (desc->m_reflectionType != rde::ReflectionType::FUNDAMENTAL)
		{
			desc->WriteTypeInfo(sw, hashesOnly);
			++numTypes;
		}
	}
	stream->Seek(rde::iosys::SeekMode::BEGIN, numTypesOffset);
	sw.WriteInt32(numTypes);
}
void SaveReflectionInfo(const char* fileName, bool hashesOnly)
{
	rde::FileStream fstream;
	if (fstream.Open(fileName, rde::iosys::AccessMode::WRITE))
		SaveReflectionInfo(&fstream, hashesOnly);
}

typedef rde::fixed_vector<StrType, 128, true> TypesToReflect;
TypesToReflect	s_typesToReflect;

bool LoadTypesToReflect(const char* fileName)
{
	FILE* f = fopen(fileName, "rt");
	if (!f)
		return false;

	char lineBuffer[512];
	while (fgets(lineBuffer, sizeof(lineBuffer) - 1, f))
	{
		// Remove all whitespaces from the end.
		for (int i = 0; lineBuffer[i] != '\0'; ++i)
		{
			if (isspace(lineBuffer[i]))
			{
				lineBuffer[i] = '\0';
				break;
			}
		}
		s_typesToReflect.push_back(StrType(lineBuffer));
	}
	fclose(f);
	return true;
}
bool ShouldBeReflected(const StrType& symbolName)
{
	for (int i = 0; i < s_typesToReflect.size(); ++i)
	{
		const StrType& typeId = s_typesToReflect[i];
		if (typeId == symbolName)
			return true;
	}
	return false;
}

void BStrToString(BSTR bstr, StrType& outStr)
{
	const UINT bslen = SysStringLen(bstr);
	rde::fixed_vector<char, 64, true> str(bslen + 1);
	for (UINT i = 0; i < bslen; ++i)
	{
		str[i] = (char)((bstr[i] >= 32 && bstr[i] < 128) ? bstr[i] : '?');
	}
	str[bslen] = 0;
	outStr = str.begin();
}


rde::vector<StrType>	s_sourceFiles;
IDiaEnumSourceFiles* GetEnumSourceFiles(IDiaSession* session)
{
	IDiaEnumSourceFiles* pRet(0);
	IDiaEnumTables* pTables(0);
	if (session->getEnumTables(&pTables) == S_OK)
	{
		REFIID iid = __uuidof(IDiaEnumSourceFiles);

		IDiaTable* pTable(0);
		ULONG celt(0);
		while (pTables->Next(1, &pTable, &celt) == S_OK && celt == 1)
		{
			const HRESULT hr = pTable->QueryInterface(iid, (void**)&pRet);
			pTable->Release();
			if (hr == S_OK)
				break;
		}
		pTables->Release();
	}
	return pRet;
}

void BuildSourceFilesList(IDiaSession* session, const char* sourceFilePathPart)
{
	IDiaEnumSourceFiles* pEnumFiles = GetEnumSourceFiles(session);
	if (pEnumFiles)
	{
		ULONG celt(0);
		IDiaSourceFile* pFile(0);
		while (pEnumFiles->Next(1, &pFile, &celt) == S_OK && celt == 1)
		{
			BSTR bstrFileName;
			if (pFile->get_fileName(&bstrFileName) == S_OK)
			{
				StrType fname;
				BStrToString(bstrFileName, fname);
				if (strstr(fname.c_str(), sourceFilePathPart) != 0)
					s_sourceFiles.push_back(fname);
			}
		}
	}
}

rde::uint16_t FindFieldFlags(const char* flagsDesc)
{
	rde::uint16_t flags(0);
	if (strstr(flagsDesc, "[Hidden]") != 0)
		flags |= rde::FieldFlags::HIDDEN;
	if (strstr(flagsDesc, "[NoSerialize]") != 0)
		flags |= rde::FieldFlags::NO_SERIALIZE;

	return flags;
}

bool File_HasType(FILE* f, const StrType& typeName)
{
	char lineBuffer[512];
	const char structKeyword[] = "struct";
	const size_t structKeywordLen = strlen(structKeyword);
	const char classKeyword[] = "class";
	const size_t classKeywordLen = strlen(classKeyword);
	const size_t typeNameLen = typeName.length();
	while (fgets(lineBuffer, sizeof(lineBuffer) - 1, f))
	{
		const char* typeNamePos = strstr(lineBuffer, typeName.c_str());
		if (typeNamePos == 0 || typeNamePos == lineBuffer)
			continue;

		// Make sure it's not a forward declaration (so next char is not ';')
		const char* typeNameEndPos = typeNamePos + typeNameLen;
		while (isspace(*typeNameEndPos))
			++typeNameEndPos;
		if (*typeNameEndPos != ';')
		{
			// Make sure it's class or struct before owner name.
			--typeNamePos;
			while (typeNamePos > lineBuffer && isspace(*typeNamePos))
				--typeNamePos;
			while (typeNamePos > lineBuffer && !isspace(*typeNamePos))
				--typeNamePos;

			if (strncmp(typeNamePos, structKeyword, structKeywordLen) == 0 ||
				strncmp(typeNamePos, classKeyword, classKeywordLen) == 0)
			{
				return true;
			}
		}
	}
	return false;
}
// Returns _opened_ file that defines given type (UDT).
FILE* FindSourceFileForUDT(const StrType& typeName)
{
	for (int i = 0; i < s_sourceFiles.size(); ++i)
	{
		FILE* f = fopen(s_sourceFiles[i].c_str(), "r");
		if (f)
		{
			if (File_HasType(f, typeName))
				return f;
			fclose(f);
		}
	}
	return 0;
}

struct FileParseContext
{
	explicit FileParseContext(FILE* f_): f(f_), numOpenedBrackets(0) {}
	~FileParseContext()
	{
		if (f)
			fclose(f);
	}

	FILE*	f;
	int		numOpenedBrackets;

private:
	FileParseContext(const FileParseContext&);
};

// True if type was found in file.
rde::uint16_t FindFieldFlags(FileParseContext& fpc, const StrType& fieldName)
{
	char prevLine[512];
	char lineBuffer[512];
	char fieldNameStr[256];
	fieldNameStr[0] = ' ';
	strcpy(fieldNameStr + 1, fieldName.c_str());
	const size_t fieldNameLen = strlen(fieldNameStr);
	rde::uint16_t flags = 0;
	while (fgets(lineBuffer, sizeof(lineBuffer) - 1, fpc.f))
	{
		// Convert tabs to spaces
		for (int i = 0; lineBuffer[i] != '\0'; ++i)
		{
			if (lineBuffer[i] == '\t')
				lineBuffer[i] = ' ';
		}

		const char* fieldNamePos = strstr(lineBuffer, fieldNameStr);
		// Sadly we still don't support block comments.
		const char* lineCommentPos = strstr(lineBuffer, "\\\\");
		// Only scan global class scope, not methods/inner structs!.
		if (fpc.numOpenedBrackets == 1 && fieldNamePos != 0 && 
			(lineCommentPos == 0 || lineCommentPos > fieldNamePos))
		{
			// Next non-space after name has to be ;
			fieldNamePos += fieldNameLen;
			while (isspace(*fieldNamePos))
				++fieldNamePos;
			if (*fieldNamePos == ';')
			{
				flags = FindFieldFlags(prevLine);
				break;
			}
		}
		if (strchr(lineBuffer, '{') != 0)
			++fpc.numOpenedBrackets;
		if (strchr(lineBuffer, '}') != 0)
			--fpc.numOpenedBrackets;
		
		strcpy(prevLine, lineBuffer);
	}
	return flags;
}

} // <anonymous> namespace

bool LoadDataFromPDB(const char* pdbName,
					 IDiaDataSource** ppSource,
					 IDiaSession** ppSession,
					 IDiaSymbol** ppGlobal)
{
	wchar_t widePdbName[_MAX_PATH];
	mbstowcs_s(0, widePdbName, _MAX_PATH, pdbName, _MAX_PATH);

	HRESULT hr = ::CoInitialize(0);
	if (FAILED(hr))
	{
		printf("CoInitialize() failed.\n");
		return false;
	}
	hr = CoCreateInstance(__uuidof(DiaSource),
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        __uuidof(IDiaDataSource),
                        (void **) ppSource);
	if (FAILED(hr))
	{
		printf("CoCreateInstance() failed.\n");
		return false;
	}

	hr = (*ppSource)->loadDataFromPdb(widePdbName);
	if (FAILED(hr))
	{
		printf("loadDataFromPdb() failed [HR = 0x%X]\n", hr);
		return false;
	}

	hr = (*ppSource)->openSession(ppSession);
	if (FAILED(hr))
	{
		printf("openSession() failed [HR = 0x%X]\n", hr);
		return false;
	}

	hr = (*ppSession)->get_globalScope(ppGlobal);
	if (FAILED(hr))
	{
		printf("get_globalScope() failed [HR = 0x%X]\n", hr);
		return false;
	}
	return true;
}

StrType GetName(IDiaSymbol* symbol)
{
	StrType retName;
	BSTR bstrName;
	if (symbol->get_name(&bstrName) != S_OK)
		return retName;
	BStrToString(bstrName, retName);
	SysFreeString(bstrName);
	return retName;
}

LONG GetSymbolOffset(IDiaSymbol* symbol)
{
	LONG offset(0);
	DWORD locType;
	if (symbol->get_locationType(&locType) == S_OK && locType == LocIsThisRel)
	{
		symbol->get_offset(&offset);
	}
	return offset;
}
size_t GetSymbolSize(IDiaSymbol* symbol)
{
	ULONGLONG len(0);
	symbol->get_length(&len);
	return (size_t)len;
}

const char* GetFundamentalTypeName(DWORD type, ULONGLONG typeSize)
{
	if (type == btUInt || type == btInt || type == btLong || type == btULong)
	{
		const bool typeSigned = (type == btInt || type == btLong);
		if (typeSize == 1)
			return typeSigned ? "int8" : "uint8";
		else if (typeSize == 2)
			return typeSigned ? "int16" : "uint16";
		else if (typeSize == 4)
			return typeSigned ? "int32" : "uint32";
		else if (typeSize == 8)
			return typeSigned ? "int64" : "uint64";
		else
			return "UnknownIntType";
	}
	else if (type == btBool)
	{
		return typeSize == 1 ? "bool" : "UnknownBoolType";
	}
	else if (type == btChar)
	{
		return typeSize == 1 ? "char" : "UnknownCharType";
	}
	else if (type == btFloat)
	{
		if (typeSize == 4)
			return "float";
		else if (typeSize == 8)
			return "double";
		else
			return "UnknownFloatType";
	}
	else
		return "UnknownFundamentalType";
}

TypeDescriptor* FindFieldType(IDiaSymbol* symbol)
{
	BOOL isStatic(FALSE);
	if (!SUCCEEDED(symbol->get_isStatic(&isStatic)) || isStatic)
		return 0;
	DWORD tag;
	if (symbol->get_symTag(&tag) != S_OK)
		return 0;

	TypeDescriptor* typeDesc(0);
	if (tag == SymTagArrayType)
	{
		IDiaSymbol* elementType(0);
		TypeDescriptor* elementTypeDesc(0);
		if (symbol->get_type(&elementType) == S_OK)
		{
			elementTypeDesc = FindFieldType(elementType);
			DWORD numElements(0);
			if (SUCCEEDED(symbol->get_count(&numElements)))
			{
				char nameSuffix[16];
				::sprintf_s(nameSuffix, "[%d]", numElements);
				StrType tname(elementTypeDesc->m_name);
				tname.append(nameSuffix);
				typeDesc = AddTypeDescriptor(tname, rde::ReflectionType::ARRAY, GetSymbolSize(symbol));
				typeDesc->m_numElements = numElements;
				typeDesc->m_dependentTypeName = elementTypeDesc->m_name;
			}
		}
	}
	else if (tag == SymTagPointerType)
	{
		IDiaSymbol* elementType(0);
		TypeDescriptor* elementTypeDesc(0);
		if (symbol->get_type(&elementType) == S_OK)
		{
			elementTypeDesc = FindFieldType(elementType);
			StrType tname(elementTypeDesc->m_name);
			tname.append("*");
			typeDesc = AddTypeDescriptor(tname, rde::ReflectionType::POINTER, GetSymbolSize(symbol));
			typeDesc->m_dependentTypeName = elementTypeDesc->m_name;
		}
	}
	else if (tag == SymTagUDT)
	{
		StrType tname = GetName(symbol);
		typeDesc = AddTypeDescriptor(tname, rde::ReflectionType::CLASS, GetSymbolSize(symbol));
	}
	else if (tag == SymTagBaseType)
	{
		DWORD baseType;
		if (SUCCEEDED(symbol->get_baseType(&baseType)))
		{
			ULONGLONG typeSize;
			symbol->get_length(&typeSize);

			StrType tname(GetFundamentalTypeName(baseType, typeSize));
			typeDesc = AddTypeDescriptor(tname, rde::ReflectionType::FUNDAMENTAL, typeSize);
		}
	}
	else if (tag == SymTagEnum)
	{
		StrType tname = GetName(symbol);
		typeDesc = AddTypeDescriptor(tname, rde::ReflectionType::ENUM, GetSymbolSize(symbol));
	}
	return typeDesc;
}

void ProcessSymbolChild(IDiaSymbol* symbol, const StrType& parentName, FileParseContext& fpc)
{
	DWORD tag;
	if (symbol->get_symTag(&tag) != S_OK)
		return;

	if (tag == SymTagBaseClass)
	{
		StrType baseName(GetName(symbol));
		LONG offset(0);
		symbol->get_offset(&offset);

		TypeDescriptor* desc = FindTypeDescriptor(parentName);
		desc->m_baseClassName = baseName;
		desc->m_baseClassOffset = (rde::uint16_t)offset;

		// We may need to reflect base class as well.
		if (!ShouldBeReflected(baseName))
		{
			TypeDescriptor* baseDesc = FindTypeDescriptor(baseName);
			if (baseDesc == 0)
			{
				baseDesc = AddTypeDescriptor(baseName, rde::ReflectionType::CLASS, GetSymbolSize(symbol));
			}
		}
	}
	else if (tag == SymTagData)	// field (member variable)
	{
		IDiaSymbol* typeSymbol(0);
		if (symbol->get_type(&typeSymbol) == S_OK)
		{
			TypeDescriptor* fieldTypeDesc = FindFieldType(typeSymbol);
			if (fieldTypeDesc)
			{
				fieldTypeDesc->m_size = GetSymbolSize(typeSymbol);

				StrType fieldName = GetName(symbol);
				TypeDescriptor* desc = FindTypeDescriptor(parentName);
				if (!desc->HasField(fieldName))
				{
					FieldDescriptor* fieldDesc = new FieldDescriptor();
					fieldDesc->m_name = fieldName;
					fieldDesc->m_typeName = fieldTypeDesc->m_name;
					fieldDesc->m_offset = (rde::uint16_t)GetSymbolOffset(symbol);
					if (fpc.f)
						fieldDesc->m_flags = FindFieldFlags(fpc, fieldName);
					desc->AddField(fieldDesc);
				}
			}
			typeSymbol->Release();
		}
	}
	else if (tag == SymTagFunction)
	{
		TypeDescriptor* desc = FindTypeDescriptor(parentName);
		BOOL isVirtual(FALSE);
		if (SUCCEEDED(symbol->get_virtual(&isVirtual)) && isVirtual)
			desc->m_flags |= TypeDescriptor::FLAG_NEEDS_VTABLE;

		BOOL isStatic(FALSE);
		if (SUCCEEDED(symbol->get_isStatic(&isStatic)) && isStatic)
		{
			StrType funcNameQualified = GetName(symbol);
			// Function names are usually in class::func format.
			// We only need 'func' part (after second :).
			const int colonPos = funcNameQualified.find_index_of_last(':');
			StrType funcName;
			if (colonPos >= 0)
			{
				const char* pname = funcNameQualified.c_str();
				funcName = pname + colonPos + 1;
			}
			if (funcName == s_initVTableFunc || funcName == s_createInstanceFunc)
			{
				ULONGLONG address(0);
				symbol->get_virtualAddress(&address);
				RDE_ASSERT(address < UINT_MAX);
				if (funcName == s_initVTableFunc)
					desc->m_pfnInitVTable = (rde::uint32_t)address;
				else if (funcName == s_createInstanceFunc)
					desc->m_pfnCreateInstance = (rde::uint32_t)address;
			}
		}
	}
	else if (tag == SymTagVTable)
	{
		TypeDescriptor* desc = FindTypeDescriptor(parentName);
		desc->m_flags |= TypeDescriptor::FLAG_NEEDS_VTABLE;
	}
}

void ProcessTopLevelSymbol(IDiaSymbol* symbol, bool processFlags)
{
	DWORD tag;
	if (symbol->get_symTag(&tag) != S_OK)
		return;

	if (tag == SymTagUDT)
	{
		StrType name(GetName(symbol));
		TypeDescriptor* typeDesc = FindTypeDescriptor(name);
		const bool typeIncomplete = (typeDesc && typeDesc->m_flags & TypeDescriptor::FLAG_INCOMPLETE);
		if (typeIncomplete || ShouldBeReflected(name))
		{
			// For some reasons, type info seems to repeated in PDB files, we're only interested in the first
			// occurance.
			if (typeDesc && !(typeDesc->m_flags & TypeDescriptor::FLAG_INCOMPLETE))
			{
				return;
			}
			typeDesc = AddTypeDescriptor(name, rde::ReflectionType::CLASS, GetSymbolSize(symbol));

			FILE* f = (processFlags ? FindSourceFileForUDT(name) : 0);
			FileParseContext fpc(f);
			// Enumerate member variables for this type.
			IDiaEnumSymbols* enumChildren(0);
			if (SUCCEEDED(symbol->findChildren(SymTagNull, 0, nsNone, &enumChildren)))
			{
				IDiaSymbol* child(0);
				ULONG celt(0);
				while (SUCCEEDED(enumChildren->Next(1, &child, &celt)) && celt == 1)
				{
					ProcessSymbolChild(child, name, fpc);

					child->Release();
				} // <for every child>

				enumChildren->Release();
			}
			typeDesc->m_flags &= ~TypeDescriptor::FLAG_INCOMPLETE;
		} 
	}
	else if (tag == SymTagEnum)
	{
		StrType name(GetName(symbol));
		if (ShouldBeReflected(name) && !FindTypeDescriptor(name))
		{
			TypeDescriptor* desc = AddTypeDescriptor(name, rde::ReflectionType::ENUM, GetSymbolSize(symbol));

			// Enumerate constants.
			IDiaEnumSymbols* enumChildren(0);
			if (SUCCEEDED(symbol->findChildren(SymTagNull, 0, nsNone, &enumChildren)))
			{
				IDiaSymbol* child(0);
				ULONG celt(0);
				while (SUCCEEDED(enumChildren->Next(1, &child, &celt)) && celt == 1)
				{
					VARIANT enumValue;
					enumValue.vt = VT_EMPTY;

					if (SUCCEEDED(child->get_value(&enumValue)))
					{
						EnumElement enumElement(GetName(child), enumValue.lVal);
						desc->AddEnumElement(enumElement);
					}
					else
					{
						printf("WARNING: Couldn't obtain enum constant (%s)\n", name.c_str());
					}

					child->Release();
				} // <for every child>
				enumChildren->Release();
			}
		}
	}
}

void PrintHelp()
{
	printf("Usage:\n");
	printf("Reflector.exe file.pdb typesToReflect_file [-flags name_part] [-hashesonly] [-out output file] [-verbose]\n");
	printf("TypesToReflect_file should be plain text file with single type per line. Example:\n");
	printf("TypesToReflect.txt:\n");
	printf("Foo\n");
	printf("Bar\n");
	printf("If -flag option is specified, following must be part of directory path where source files are stored.\n");
	printf("For example, if our project is in c:\\projects\\myproject, -flags myproject could be used.\n");
	printf("This option will make Reflector scan source codes searching for annotations in comments to dig out\n"
	    "flags for class fields. It can slow down processing significantly on big projects.\n");
	printf("-hashesonly - reflector will save strings as hashes.\n");
	printf("If output file is not specified, it's assumed to be file.ref.\n");
}

void CreateOutputFileNameFromPdb(char* outputFileName, size_t bufSize, const char* pdbFileName)
{
	strcpy_s(outputFileName, bufSize, pdbFileName);
	int i;
	for (i = 0; outputFileName[i] != '.'; ++i)
		;
	++i;	// skip '.'.
	outputFileName[i++] = 'r';
	outputFileName[i++] = 'e';
	outputFileName[i++] = 'f';
	outputFileName[i] = '\0';
}

bool ProcessSymbolsOfTag(IDiaSymbol* globalScope, enum SymTagEnum tag, bool processFlags)
{
	IDiaEnumSymbols* enumSymbols(0);
	if (FAILED(globalScope->findChildren(tag, NULL, nsNone, &enumSymbols)))
		return false;

	IDiaSymbol* symbol(0);
	ULONG celt(0);
	while (SUCCEEDED(enumSymbols->Next(1, &symbol, &celt)) && celt == 1)
	{
		ProcessTopLevelSymbol(symbol, processFlags);
		symbol->Release();
	}
	enumSymbols->Release();
	return true;
}

// -1 if argument not found.
int GetArgumentIndex(int argc, char const *argv[], const char* arg)
{
	static const char argDelimiters[] = "-/";
	const int numDelimiters = (int)strlen(argDelimiters);
	for (int i = 0; i < argc; ++i)
	{
		bool delimiterFound(false);
		for (int j = 0; !delimiterFound && j < numDelimiters; ++j)
		{
			if (argv[i][0] == argDelimiters[j])
				delimiterFound = true;
		}
		if (delimiterFound)
		{
			if (stricmp(&argv[i][1], arg) == 0)
				return i;
		}
	}
	return -1;
}

int __cdecl main(int argc, char const *argv[])
{
	if (argc < 3 || argc > 9)
	{
		PrintHelp();
		return 1;
	}
	const char* pdbFileName = argv[1];
	if (strstr(pdbFileName, ".pdb") == 0)
	{
		PrintHelp();
		return 1;
	}
	const char* typeListFileName = argv[2];

	// --- Flag processing mode.
	const char* sourceFilePathPart = 0;
	const int iFlagsArg = GetArgumentIndex(argc, argv, "flags");
	bool processFlags(false);
	if (iFlagsArg > 2)
	{
		// We need at least one more argument.
		if (argc < iFlagsArg + 1)
		{
			PrintHelp();
			return 1;
		}
		sourceFilePathPart = argv[iFlagsArg + 1];
		processFlags = true;
	}

	const bool hashesOnly = (GetArgumentIndex(argc, argv, "hashesonly") > 2);
	const bool verboseMode = (GetArgumentIndex(argc, argv, "verbose") > 2);
	if (verboseMode)
	{
		printf("Reflector settings:\n-------------------\n");
		printf("* Extract field flags: %s (source files path part: %s)\n", 
			(sourceFilePathPart == 0 ? "no" : "yes"),
			(sourceFilePathPart == 0 ? "---" : sourceFilePathPart));
		printf("* Hashes only: %s\n", (hashesOnly ? "yes" : "no"));
	}

	if (!LoadTypesToReflect(typeListFileName))
	{
		printf("Unable to load list of files to reflect from file '%s'\n", typeListFileName);
		printf("This should be plain text file with single type per line. Example:\n");
		printf("TypesToReflect.txt:\n");
		printf("Foo\n");
		printf("Bar\n");
	}

	IDiaSession* session(0);
	IDiaDataSource* dataSource(0);
	IDiaSymbol* globalScope(0);
	if (!LoadDataFromPDB(pdbFileName, &dataSource, &session, &globalScope))
	{
		printf("Unable to load '%s'\n", pdbFileName);
		return 1;
	}

	if (sourceFilePathPart)
		BuildSourceFilesList(session, sourceFilePathPart);

	if (!ProcessSymbolsOfTag(globalScope, SymTagEnum, false))
		printf("Error while processing enum symbols.\n");
	do
	{
		if (!ProcessSymbolsOfTag(globalScope, SymTagUDT, processFlags))
			printf("Error while processing UDT symbols.\n");
	} while (HasAnyIncompleteTypes());

	if (verboseMode)
		PrintAllTypes();

	char outputFileName[_MAX_PATH];
	const int iOutputFileArg = GetArgumentIndex(argc, argv, "out");
	if (iOutputFileArg > 2 && argc > iOutputFileArg + 1)
	{
		strcpy_s(outputFileName, argv[iOutputFileArg + 1]);
	}
	else
	{
		CreateOutputFileNameFromPdb(outputFileName, sizeof(outputFileName), pdbFileName);
	}
	if (verboseMode)
		printf("* Writing reflection info to %s\n", outputFileName);
	SaveReflectionInfo(outputFileName, hashesOnly);

	return 0;
}
