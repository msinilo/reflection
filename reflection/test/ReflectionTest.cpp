#include <cstdio>
#include <cstddef>
#include "reflection/TypeClass.h"
#include "reflection/TypeEnum.h"
#include "reflection/TypeRegistry.h"
#include "io/FileStream.h"
#include "io/StreamReader.h"
#include "rdestl/stack.h"
#include "core/win32/Windows.h"
#include <cmath>

// 0 - no debug messages
#define DBG_VERBOSITY_LEVEL			2

enum EInitVTable
{
	INIT_VTABLE
};
struct Vector3
{
	float	x, y, z;
};
struct Color
{
	Color() {}
	Color(float r_, float g_, float b_): r(r_), g(g_), b(b_) {}

	bool AlmostEqual(const Color& rhs, float eps) const
	{
		return fabs(r - rhs.r) < eps && fabs(g - rhs.g) < eps && fabs(b - rhs.b) < eps;
	}

	float r, g, b;
};
struct IntContainer
{
	IntContainer(): pInt(new int) {}
	IntContainer(EInitVTable) {}
	IntContainer(const IntContainer& rhs): pInt(new int) 
	{
		*pInt = *rhs.pInt;
	}
	~IntContainer() { delete pInt; }

	int*	pInt;
};

struct SuperBar 
{
	SuperBar(): p(0), psb(0) {}
	explicit SuperBar(EInitVTable ei): v(rde::noinitialize), ic(ei) {}
	virtual ~SuperBar() {};

	virtual int VirtualTest()
	{
		return 5;
	}

	static void* Reflection_CreateInstance()
	{
		//--- That's only to confuse Reflector comment parser.
		int i;
		// And again, here. void* p;
		(void)i;

		return new SuperBar();
	}
	static void* Reflection_InitVTable(void* mem)
	{
		return new (mem) SuperBar(INIT_VTABLE);
	}

	unsigned long	i;
	// [Hidden]
	float*			p;
	bool			b;
	signed char		s;
	Color			color;
	SuperBar*		psb;
	typedef rde::vector<int> tVec;
	tVec			v;
	rde::vector<Color*> someColors;
	rde::vector<SuperBar*> superBars;
	rde::vector<IntContainer> containers;
	IntContainer	ic;
};

struct Bar : public SuperBar
{	
	enum
	{
		ARR_MAX = 10
	};
	enum TestEnum
	{
		FIRST	= 0,
		SECOND,
		LAST	= 10
	};	
	virtual ~Bar() {}

	virtual void Foo()
	{
	}
	float	f;
	char	c;
	short	shortArray[ARR_MAX];
	Vector3	position;
	Vector3* porient;
	SuperBar sb;
	SuperBar** pb;
	TestEnum en;
};
namespace rde
{
	RDE_IMPL_GET_TYPE_NAME(SuperBar);
}

#if DBG_VERBOSITY_LEVEL == 0
#	define DBGPRINTF
#	define DBGPRINTF2
#elif DBG_VERBOSITY_LEVEL == 1
#	define DBGPRINTF	::printf
#	define DBGPRINTF2
#else
#	define DBGPRINTF	::printf
#	define DBGPRINTF2	::printf
#endif


#include <dbghelp.h>
#include <cstdlib>
#pragma comment(lib, "dbghelp.lib")
unsigned long s_moduleBase(0);
void GetFileFromPath(const char* path, char* file, int fileNameSize)
{
	char ext[_MAX_EXT] = { 0 };
	_splitpath_s(path, 0, 0, 0, 0, file, fileNameSize, ext, _MAX_EXT);
	strncat_s(file, fileNameSize, ext, _MAX_EXT);
}
BOOL CALLBACK EnumerateModule(PCSTR /*moduleName*/, DWORD64 moduleBase, ULONG /*moduleSize*/,
	PVOID /*userContext*/)
{
	s_moduleBase = (unsigned long)moduleBase;
	return false;	// Terminate after first (main) module.
}

void EnumerateModules()
{
	const HANDLE hCurrentProcess = ::GetCurrentProcess();
	if (::SymInitialize(hCurrentProcess, 0, false))
	{
		char modulePath[_MAX_PATH] = { 0 };
		::GetModuleFileName(::GetModuleHandle(0), modulePath, sizeof(modulePath));

		char moduleFile[_MAX_FNAME] = { 0 };
		GetFileFromPath(modulePath, moduleFile, sizeof(moduleFile));
		::EnumerateLoadedModules64(hCurrentProcess, EnumerateModule, moduleFile);
	}
}

void LoadFields(rde::StreamReader& sr, rde::TypeClass& tc)
{
	const int numFields = sr.ReadInt32();
	char fieldNameBuffer[128];
	for (int i = 0; i < numFields; ++i)
	{
		const rde::uint32_t typeId = sr.ReadInt32();
		const rde::uint16_t offset = sr.ReadInt16();
		const rde::uint16_t flags = sr.ReadInt16();
		sr.ReadASCIIZ(fieldNameBuffer, sizeof(fieldNameBuffer));
		rde::Field field(fieldNameBuffer, typeId, offset, &tc);
		field.m_flags = flags;
		tc.AddField(field);
	}
}

void LoadReflectionInfo(rde::Stream& stream, rde::TypeRegistry& typeRegistry)
{
	rde::StreamReader sr(&stream);
	const int numTypes = sr.ReadInt32();
	char nameBuffer[512];
	for (int i = 0; i < numTypes; ++i)
	{
		sr.ReadASCIIZ(nameBuffer, sizeof(nameBuffer));
		const rde::uint32_t typeSize = sr.ReadInt32();
		const int reflectionType = sr.ReadInt32();

		rde::Type* newType(0);
		if (reflectionType == rde::ReflectionType::CLASS)
		{
			const rde::uint32_t baseClassId = sr.ReadInt32();
			const rde::uint16_t baseClassOffset = sr.ReadInt16();

			size_t createInstanceFuncAddress = sr.ReadInt32();
			if (createInstanceFuncAddress != 0)
				createInstanceFuncAddress += s_moduleBase;
			rde::TypeClass::FnCreateInstance pfnCreateInstance = (rde::TypeClass::FnCreateInstance)createInstanceFuncAddress;

			size_t initVTableFuncAddress = sr.ReadInt32();
			if (initVTableFuncAddress != 0)
				initVTableFuncAddress += s_moduleBase;
			rde::TypeClass::FnInitVTable pfnInitVTable = (rde::TypeClass::FnInitVTable)initVTableFuncAddress;

			rde::TypeClass* tc = new rde::TypeClass(typeSize, nameBuffer, pfnCreateInstance, pfnInitVTable, 
				baseClassId, baseClassOffset);
			LoadFields(sr, *tc);
			newType = tc;
		}
		else if (reflectionType == rde::ReflectionType::ENUM)
		{
			const int numEnumElements = sr.ReadInt32();
			char enumeratorNameBuffer[128];
			rde::TypeEnum* te = new rde::TypeEnum(typeSize, nameBuffer);
			for (int i = 0; i < numEnumElements; ++i)
			{
				sr.ReadASCIIZ(enumeratorNameBuffer, sizeof(enumeratorNameBuffer));
				const int enumeratorValue = sr.ReadInt32();
				rde::TypeEnum::Constant enumConstant(enumeratorNameBuffer, enumeratorValue);
				te->AddConstant(enumConstant);
			}
			newType = te;
		}
		else if (reflectionType == rde::ReflectionType::POINTER)
		{
			const rde::uint32_t pointedTypeId = sr.ReadInt32();
			newType = new rde::TypePointer(typeSize, nameBuffer, pointedTypeId);
		}
		else if (reflectionType == rde::ReflectionType::ARRAY)
		{
			const rde::uint32_t containedTypeId = sr.ReadInt32();
			const int numElements = sr.ReadInt32();
			newType = new rde::TypeArray(typeSize, nameBuffer, containedTypeId, numElements);
		}
		else
		{
			RDE_ASSERT(!"Invalid reflection type");
		}
		if (newType)
			typeRegistry.AddType(newType);
	}
}
bool LoadReflectionInfo(const char* fileName, rde::TypeRegistry& typeRegistry)
{
	rde::FileStream fstream;
	if (!fstream.Open(fileName, rde::iosys::AccessMode::READ))
		return false;

	LoadReflectionInfo(fstream, typeRegistry);
	typeRegistry.PostInit();
	return true;
}
void PrintEnumConstant(const rde::TypeEnum::Constant& e, void*)
{
	printf("%s = %d\n", e.m_name.GetStr(), e.m_value);
}
void PrintField(const rde::Field* field, void*)
{
	printf("Field: %s, offset: %d, type name: %s\n", field->m_name.GetStr(), field->m_offset, 
		field->m_type->m_name.GetStr());
}

//-----------------------------------------------------------------------------------------------------
// Load-in-place test system

struct ObjectHeader
{
	rde::uint32_t	typeTag;
	rde::uint32_t	size;
	rde::uint16_t	numPointerFixups;
};
struct PointerFixupEntry
{
	PointerFixupEntry(): m_pointerOffset(0), m_pointerValueOffset(0), m_typeTag(0) {}

	rde::uint32_t	m_pointerOffset;
	rde::uint32_t	m_pointerValueOffset;
	// 0 if no need to patch vtable.
	rde::uint32_t	m_typeTag;	
};
struct RawFieldInfo
{
	void**		m_mem;
	size_t		m_size;
	int			m_fixupIndex;
#if DBG_VERBOSITY_LEVEL > 0
	rde::StrId	m_name;
	int			m_nestLevel;
#endif
};

typedef rde::fixed_vector<PointerFixupEntry, 16, true> PointerFixups;
typedef rde::fixed_vector<RawFieldInfo, 16, true> RawFields;
typedef rde::vector<const rde::Field*> Fields;

struct CollectContext
{
	struct ObjectStackEntry
	{
		void*					m_obj;
		const rde::TypeClass*	m_objType;
		rde::uint32_t			m_pointerOffset;
	};
	typedef rde::stack<ObjectStackEntry, rde::allocator, 
		rde::fixed_vector<ObjectStackEntry, 16, true> >	ObjectStack;

	CollectContext(): m_dataSize(0), m_pointerValueOffset(0), m_typeRegistry(0) {}

	PointerFixups			m_fixups;
	ObjectStack				m_objectStack;
	RawFields				m_fields;
	rde::uint32_t			m_dataSize;
	rde::uint32_t			m_pointerValueOffset;
	rde::TypeRegistry*		m_typeRegistry;
};
void CollectMembers(const rde::Field* field, void* userData);

// Cannot have non-serializable fields.
bool IsLoadInPlaceCompatible(const rde::TypeClass* tc)
{
	const int numFields = tc->GetNumFields();
	for (int i = 0; i < numFields; ++i)
	{
		const rde::Field* field = tc->GetField(i);
		if (field->m_flags & rde::FieldFlags::NO_SERIALIZE)
			return false;
	}
	return true;
}

void CollectPointer(void** rawFieldMem, CollectContext* context, const rde::TypePointer* tp,
					rde::uint32_t fieldOffset, const char* fieldName)
{
	CollectContext::ObjectStackEntry& topEntry = context->m_objectStack.top();
	const rde::uint32_t currentPointerOffset = topEntry.m_pointerOffset;

	// NULL pointer, ignore, no need to patch, it'll be written directly as 0.
	if (*rawFieldMem == 0)
		return;

	// Pointer already processed?
	int iField = -1;
	for (iField = 0; iField < context->m_fields.size(); ++iField)
	{
		if (*context->m_fields[iField].m_mem == *rawFieldMem)
			break;
	}
	const bool ptrAlreadyFound(iField != context->m_fields.size());

	const rde::uint32_t currentPointerValueOffset = context->m_pointerValueOffset;
	PointerFixupEntry ptrFixup;
	const rde::Type* pointedType = context->m_typeRegistry->FindType(tp->m_pointedTypeId);
	const rde::TypeClass* tc = rde::ReflectionTypeCast<rde::TypeClass>(pointedType);
	if (ptrAlreadyFound)
	{
		ptrFixup.m_pointerOffset = currentPointerOffset + fieldOffset;
		const int iFixup = context->m_fields[iField].m_fixupIndex;
		ptrFixup.m_pointerValueOffset = context->m_fixups[iFixup].m_pointerValueOffset;
	}
	else
	{
		ptrFixup.m_pointerOffset = currentPointerOffset + fieldOffset;
		ptrFixup.m_pointerValueOffset = currentPointerValueOffset;
	}
	if (tc && tc->HasVTable())
	{
		ptrFixup.m_typeTag = tc->m_name.GetId();
	}
	DBGPRINTF("Field: %s, offset: %d, fixup offset: %d\n", 
		fieldName, ptrFixup.m_pointerOffset, ptrFixup.m_pointerValueOffset);

	context->m_fixups.push_back(ptrFixup);
	if (!ptrAlreadyFound)
	{
		RawFieldInfo fieldInfo = 
		{ 
			rawFieldMem,
			pointedType->m_size, 
			context->m_fixups.size() - 1 
#if DBG_VERBOSITY_LEVEL > 0
			, fieldName, context->m_objectStack.size()
#endif
		};
		context->m_fields.push_back(fieldInfo);
		context->m_dataSize += pointedType->m_size;
		context->m_pointerValueOffset += pointedType->m_size;
	}
	// Recurse down if pointing to class.
	if (tc && !ptrAlreadyFound)
	{
		// Next type will be saved after what we've saved so far (so new object offset == dataSize).
		CollectContext::ObjectStackEntry newEntry = { *rawFieldMem, tc, context->m_dataSize }; 
		context->m_objectStack.push(newEntry);

		tc->EnumerateFields(CollectMembers, rde::ReflectionType::POINTER | rde::ReflectionType::CLASS, context);
		context->m_objectStack.pop();
	}
}
void CollectClass(void* rawPointer, CollectContext* context, const rde::TypeClass* tc, rde::uint32_t fieldOffset);
void CollectPointers_Vector(CollectContext* context, const rde::TypeClass* tc)
{
	// Two fields: begin & end.
	// We need to serialize whole array between those.
	CollectContext::ObjectStackEntry& topEntry = context->m_objectStack.top();

	// That's a little bit risky, we probably should access fields by name, but
	// rde::vector is 'fixed' and this way it's quicker.
	const rde::Field* fieldBegin = tc->GetField(0);
	const rde::Field* fieldEnd = tc->GetField(1);

	// Let's calculate how many bytes do we need to save.
	rde::FieldAccessor accessorBegin(topEntry.m_obj, topEntry.m_objType, fieldBegin);
	rde::FieldAccessor accessorEnd(topEntry.m_obj, topEntry.m_objType, fieldEnd);
	const rde::uint8_t** ppBegin = (const rde::uint8_t**)accessorBegin.GetRawPointer();
	const rde::uint8_t** ppEnd = (const rde::uint8_t**)accessorEnd.GetRawPointer();
	const rde::uint32_t numBytes = (rde::uint32_t)(*ppEnd - *ppBegin);
	if (numBytes == 0)
		return;

	RawFieldInfo fieldInfo = 
	{ 
		(void**)ppBegin, numBytes, -1
#if DBG_VERBOSITY_LEVEL > 0
		, "rde::vector", context->m_objectStack.size()
#endif
	};
	context->m_fields.push_back(fieldInfo);
	context->m_dataSize += numBytes;

	// Fix-ups for begin pointer.
	const rde::uint32_t currentPointerOffset = topEntry.m_pointerOffset;
	const rde::uint32_t currentPointerValueOffset = context->m_pointerValueOffset;
	PointerFixupEntry ptrFixup;
	ptrFixup.m_pointerOffset = currentPointerOffset + fieldBegin->m_offset;
	ptrFixup.m_pointerValueOffset = currentPointerValueOffset;
	context->m_fixups.push_back(ptrFixup);
//	DBGPRINTF("Vector field: %s, begin offset: %d, fixup offset: %d\n", field->m_name.GetStr(),
//		ptrFixup.m_pointerOffset, ptrFixup.m_pointerValueOffset);

	// Fixup for end pointer.
	ptrFixup.m_pointerOffset = currentPointerOffset + fieldEnd->m_offset;
	ptrFixup.m_pointerValueOffset = currentPointerValueOffset + numBytes;
	context->m_fixups.push_back(ptrFixup);
//	DBGPRINTF("Vector field: %s, end offset: %d, fixup offset: %d\n", field->m_name.GetStr(),
//		ptrFixup.m_pointerOffset, ptrFixup.m_pointerValueOffset);

	// That's how much raw data we actually save.
	context->m_pointerValueOffset += numBytes;

	// Now fixups for vector elements if needed.
	const rde::TypePointer* tp = rde::SafeReflectionCast<rde::TypePointer>(fieldBegin->m_type);
	const rde::Type* pointedType = context->m_typeRegistry->FindType(tp->m_pointedTypeId);
	if (pointedType->m_reflectionType == rde::ReflectionType::POINTER)
	{
		// Collection of pointers.
		RDE_ASSERT(numBytes % sizeof(void*) == 0);
		const int size = numBytes / sizeof(void*);
		const rde::uint8_t* pBegin = *ppBegin;
		// Fake object (starts where vector contents are saved).
		CollectContext::ObjectStackEntry newEntry = { 0, 0, currentPointerValueOffset };
		context->m_objectStack.push(newEntry);
		for (int i = 0; i < size; ++i)
		{
			CollectPointer((void**)pBegin, context, static_cast<const rde::TypePointer*>(pointedType), 
				i * sizeof(void*), "vecelem");
			pBegin += sizeof(void*);
		}
		context->m_objectStack.pop();
	}
	else if (pointedType->m_reflectionType == rde::ReflectionType::CLASS)
	{
		// Collection of classes. Need to generate fix-ups for them.
		RDE_ASSERT(numBytes % pointedType->m_size == 0);
		const int numElements = numBytes / pointedType->m_size;
		const rde::uint8_t* pBegin = *ppBegin;
		// Fake object (starts where vector contents are saved).
		CollectContext::ObjectStackEntry newEntry = { 0, 0, currentPointerValueOffset };
		context->m_objectStack.push(newEntry);
		for (int i = 0; i < numElements; ++i)
		{
			CollectClass((void*)pBegin, context, static_cast<const rde::TypeClass*>(pointedType), 
				i * pointedType->m_size);
			pBegin += pointedType->m_size;
		}
		context->m_objectStack.pop();
	}
}

void CollectClass(void* rawPointer, CollectContext* context, const rde::TypeClass* tc, rde::uint32_t fieldOffset)
{
	CollectContext::ObjectStackEntry& topEntry = context->m_objectStack.top();
	const rde::uint32_t currentPointerOffset = topEntry.m_pointerOffset;
	RDE_ASSERT(IsLoadInPlaceCompatible(tc));
	CollectContext::ObjectStackEntry newEntry = 
	{ 
		rawPointer, tc, 
		currentPointerOffset + fieldOffset 
	};
	context->m_objectStack.push(newEntry);
	// Special case A: rde::vector.
	static const char* rdeVectorName = "rde::vector<";
	static const long rdeVectorNameLen = rde::strlen(rdeVectorName);
	if (rde::strcompare(tc->m_name.GetStr(), rdeVectorName, rdeVectorNameLen) == 0)
	{
		CollectPointers_Vector(context, tc);
	}
	else // 'ordinary' class, enumerate all pointers (TODO: handle other special classes!)
	{
		tc->EnumerateFields(CollectMembers, rde::ReflectionType::POINTER | rde::ReflectionType::CLASS, context);
	}
	context->m_objectStack.pop();
}

void CollectClass(const rde::Field* field, CollectContext* context)
{
	CollectContext::ObjectStackEntry& topEntry = context->m_objectStack.top();
	rde::FieldAccessor fieldAccessor(topEntry.m_obj, topEntry.m_objType, field);
	const rde::TypeClass* tc = static_cast<const rde::TypeClass*>(field->m_type);
	CollectClass(fieldAccessor.GetRawPointer(), context, tc, field->m_offset);
}
void CollectPointer(const rde::Field* field, CollectContext* context)
{
	CollectContext::ObjectStackEntry& topEntry = context->m_objectStack.top();
	rde::FieldAccessor fieldAccessor(topEntry.m_obj, topEntry.m_objType, field);

	void** rawFieldMem = (void**)fieldAccessor.GetRawPointer();
	CollectPointer(rawFieldMem, context, static_cast<const rde::TypePointer*>(field->m_type), field->m_offset,
		field->m_name.GetStr());
}

void CollectMembers(const rde::Field* field, void* userData)
{
	CollectContext* context = (CollectContext*)userData;

	// Special case: field is some other class.
	if (field->m_type->m_reflectionType == rde::ReflectionType::CLASS)
	{
		CollectClass(field, context);
	}
	else	// ptr
	{
		CollectPointer(field, context);
	}
}

// Rough layout:
//	- header
//	- pointer fixups
//	- main object
//	- objects referenced in main object (raw mem).
// Pointer fixups are in format:
//	- offset of pointer to fix-up (from start of main object),
//	- offset of memory to set pointer to

void SaveObjectImpl(const void* obj, const rde::StrId& typeName, rde::Stream& stream, 
				rde::TypeRegistry& typeRegistry)
{
	const rde::TypeClass* type = rde::ReflectionTypeCast<rde::TypeClass>(typeRegistry.FindType(typeName));
	RDE_ASSERT(IsLoadInPlaceCompatible(type));

	ObjectHeader objectHeader;
	objectHeader.typeTag = type->m_name.GetId();
 
	// Initialize context.
	CollectContext collectContext;
	// We first save 'obj', skip it here (pointer data is saved after main object).
	collectContext.m_pointerValueOffset = type->m_size;
	CollectContext::ObjectStackEntry startEntry = { (void*)obj, type, 0 };
	collectContext.m_objectStack.push(startEntry);
	collectContext.m_typeRegistry = &typeRegistry;
	// Treat us as a field as well (in case someone keeps a reference to us).
	RawFieldInfo startField = 
	{ 
		(void**)&obj, type->m_size, 0 
#if DBG_VERBOSITY_LEVEL > 0
		, typeName, 0
#endif
	};
	collectContext.m_fields.push_back(startField);
	// Initial fix-up (0, 0) for main object.
	PointerFixupEntry ptrFixup;
	collectContext.m_fixups.push_back(ptrFixup);
	type->EnumerateFields(CollectMembers, rde::ReflectionType::POINTER | rde::ReflectionType::CLASS, &collectContext);
	// Fix size, couldn't do it earlier, because we used this as object offset, so it should start at 0.
	collectContext.m_dataSize += type->m_size;

	// Write object header.
	objectHeader.size = collectContext.m_dataSize;
	objectHeader.numPointerFixups = (rde::uint16_t)collectContext.m_fixups.size() - 1;
	stream.Write(&objectHeader, sizeof(ObjectHeader));

	// Write fixups
	// Skip initial fixup, it's always 0, 0
	if (objectHeader.numPointerFixups > 0)
		stream.Write(collectContext.m_fixups.begin() + 1, objectHeader.numPointerFixups * sizeof(PointerFixupEntry));

	// Raw object memory (main obj + ptr fields).
	const int objectMemStart = stream.GetPosition();
	for (RawFields::iterator it = collectContext.m_fields.begin(); it != collectContext.m_fields.end(); ++it)
	{
		DBGPRINTF2("%*s%d: %s [%d byte(s)]\n", it->m_nestLevel, "", 
			stream.GetPosition() - objectMemStart, it->m_name.GetStr(), it->m_size);
		stream.Write(*it->m_mem, (long)it->m_size);
	}
}
template<typename T>
void SaveObject(const T& obj, rde::Stream& stream, rde::TypeRegistry& typeRegistry)
{
	SaveObjectImpl(&obj, rde::GetTypeName<T>(), stream, typeRegistry);
}

void* LoadObjectImpl(rde::Stream& stream, rde::TypeRegistry& typeRegistry)
{
	ObjectHeader objectHeader;
	stream.Read(&objectHeader, sizeof(objectHeader));

	const rde::TypeClass* type = 
		static_cast<const rde::TypeClass*>(typeRegistry.FindType(objectHeader.typeTag));
	PointerFixups pointerFixups;
	if (objectHeader.numPointerFixups > 0)
	{
		pointerFixups.resize(objectHeader.numPointerFixups);
		stream.Read(pointerFixups.begin(), objectHeader.numPointerFixups * sizeof(PointerFixupEntry));
	}
	void* objectMem = operator new(objectHeader.size);
	stream.Read(objectMem, objectHeader.size);
	type->InitVTable(objectMem);

	rde::uint8_t* objectMem8 = static_cast<rde::uint8_t*>(objectMem);
	for (PointerFixups::const_iterator it = pointerFixups.begin(); it != pointerFixups.end(); ++it)
	{
		rde::uint8_t* pptr = objectMem8 + it->m_pointerOffset;
		void* patchedMem = objectMem8 + it->m_pointerValueOffset;
		*reinterpret_cast<void**>(pptr) = patchedMem;

		if (it->m_typeTag != 0)
		{
			const rde::TypeClass* fieldType = 
				static_cast<const rde::TypeClass*>(typeRegistry.FindType(it->m_typeTag));
			// We already initialized vtable for 'main' object.
			if (patchedMem != objectMem)
				fieldType->InitVTable(patchedMem);
		}
	}
	return objectMem;
}
template<typename T>
T* LoadObject(rde::Stream& stream, rde::TypeRegistry& typeRegistry)
{
	return static_cast<T*>(LoadObjectImpl(stream, typeRegistry));
}

void TestLoadInPlace(rde::TypeRegistry& typeRegistry)
{
#define RDE_LIP_ASSERT	RDE_ASSERT_ALWAYS

	SuperBar sb;
	sb.i = 5;
	sb.b = false;
	sb.s = -100;
	sb.color.r = 0.7f;
	sb.color.g = 0.2f;
	sb.color.b = 0.55f;
	sb.p = &sb.color.r;
	SuperBar sb2;
	sb2.p = &sb.color.g;
	sb.psb = &sb2;
	sb2.psb = &sb;
	*sb2.ic.pInt = 333;
	*sb.ic.pInt = 666;
	
	sb.v.push_back(1);
	sb.v.push_back(2);
	const int vcapacity = sb.v.capacity();

	sb.someColors.push_back(new Color(0.f, 0.f, 1.f));
	sb.someColors.push_back(new Color(0.f, 1.f, 0.f));
	sb.someColors.push_back(new Color(1.f, 0.f, 0.f));

	SuperBar* sb3 = new SuperBar();
	sb3->i = 10;
	sb3->psb = &sb2;
	sb.superBars.push_back(&sb2);
	sb.superBars.push_back(sb3);

	IntContainer ic1;
	*ic1.pInt = 1;
	sb.containers.push_back(ic1);
	IntContainer ic2;
	*ic2.pInt = 2;
	sb.containers.push_back(ic2);
	
	{
		rde::FileStream ofstream;
		if (!ofstream.Open("test.lip", rde::iosys::AccessMode::WRITE))
			return;
		SaveObject(sb, ofstream, typeRegistry);
		ofstream.Close();
	}
	{
		rde::FileStream ifstream;
		if (!ifstream.Open("test.lip", rde::iosys::AccessMode::READ))
			return;

		delete sb.someColors[0];
		delete sb.someColors[1];
		delete sb.someColors[2];
		sb.someColors.clear();
		delete sb3;

		SuperBar* psb = LoadObject<SuperBar>(ifstream, typeRegistry);
		ifstream.Close();

		RDE_LIP_ASSERT(psb->i == sb.i);
		RDE_LIP_ASSERT(psb->b == sb.b);
		RDE_LIP_ASSERT(psb->s == sb.s);
		RDE_LIP_ASSERT(psb->color.r == sb.color.r);
		RDE_LIP_ASSERT(psb->color.g == sb.color.g);
		RDE_LIP_ASSERT(psb->color.b == sb.color.b);
		RDE_LIP_ASSERT(*psb->p == *sb.p);
		RDE_LIP_ASSERT(*psb->p == psb->color.r);
		RDE_LIP_ASSERT(psb->VirtualTest() == 5);
		RDE_LIP_ASSERT(psb->v.size() == 2);
		RDE_LIP_ASSERT(psb->v.capacity() == vcapacity);
		RDE_LIP_ASSERT(psb->v[0] == 1);
		RDE_LIP_ASSERT(psb->v[1] == 2);
		RDE_LIP_ASSERT(*psb->ic.pInt == 666);
		RDE_LIP_ASSERT(*psb->psb->ic.pInt == 333);

		RDE_LIP_ASSERT(psb->someColors.size() == 3);
		RDE_LIP_ASSERT(psb->someColors[0]->AlmostEqual(Color(0.f, 0.f, 1.f), 1e-4f));
		RDE_LIP_ASSERT(psb->someColors[1]->AlmostEqual(Color(0.f, 1.f, 0.f), 1e-4f));
		RDE_LIP_ASSERT(psb->someColors[2]->AlmostEqual(Color(1.f, 0.f, 0.f), 1e-4f));
		RDE_LIP_ASSERT(*psb->containers[0].pInt == 1);
		RDE_LIP_ASSERT(*psb->containers[1].pInt == 2);

 		RDE_LIP_ASSERT(psb->superBars[0]->psb == psb);
		RDE_LIP_ASSERT(psb->superBars[1]->i == 10);
		RDE_LIP_ASSERT(psb->superBars[1]->psb == psb->psb);

		RDE_LIP_ASSERT(psb->psb->psb == psb);
		RDE_LIP_ASSERT(*psb->psb->p == psb->color.g);
		RDE_LIP_ASSERT(psb->psb->VirtualTest() == 5);

		operator delete(psb);
	}
}

// Fake references to stop linker from stripping those functions.
void RegisterFunctions()
{
	SuperBar::Reflection_InitVTable(0);
	delete SuperBar::Reflection_CreateInstance();
}

int __cdecl main(int, char const *[])
{
	EnumerateModules();

	rde::TypeRegistry typeRegistry;
	if (!LoadReflectionInfo("reflectiontest.ref", typeRegistry))
	{
		printf("Couldn't load reflection info.\n");
		return 1;
	}

	Bar bar;
	const rde::TypeClass* barType = rde::SafeReflectionCast<rde::TypeClass>(typeRegistry.FindType("Bar"));
	rde::FieldAccessor accessor_f(&bar, barType, "f");
	accessor_f.Set(5.f);
	RDE_ASSERT(bar.f == 5.f);

	const rde::Field* field = barType->FindField("i");
	RDE_ASSERT(field->m_type->m_reflectionType == rde::ReflectionType::FUNDAMENTAL);
	RDE_ASSERT(field->m_type == rde::TypeOf<unsigned long>());
	field->Set(&bar, barType, 10);
	RDE_ASSERT(bar.i == 10);

	rde::FieldAccessor accessorArray(&bar, barType, "shortArray");
	short* pArray = (short*)accessorArray.GetRawPointer();
	pArray[0] = 0;
	pArray[1] = 1;
	pArray[2] = 2;
	RDE_ASSERT(bar.shortArray[0] == 0 && bar.shortArray[1] == 1 && bar.shortArray[2] == 2);
	const rde::Field* fieldShortArray = barType->FindField("shortArray");
	const rde::TypeArray* typeShortArray = rde::SafeReflectionCast<rde::TypeArray>(fieldShortArray->m_type);
	RDE_ASSERT(typeShortArray->m_numElements == Bar::ARR_MAX);

	// Print all pointers of Bar.
	barType->EnumerateFields(PrintField, rde::ReflectionType::POINTER);
	const rde::TypeClass* superBarType = rde::SafeReflectionCast<rde::TypeClass>(typeRegistry.FindType("SuperBar"));
	RDE_ASSERT(barType->IsDerivedFrom(superBarType));
	RDE_ASSERT(!superBarType->IsDerivedFrom(barType));

	// Print all enum constants from TestEnum.
	const rde::TypeEnum* enumType = rde::SafeReflectionCast<rde::TypeEnum>(typeRegistry.FindType("Bar::TestEnum"));
	printf("enum %s\n", enumType->m_name.GetStr());
	enumType->EnumerateConstants(PrintEnumConstant);

	SuperBar* psb = typeRegistry.CreateInstance<SuperBar>("SuperBar");
	RDE_ASSERT(psb->VirtualTest() == 5);
	delete psb;

	TestLoadInPlace(typeRegistry);

	return 0;
}
