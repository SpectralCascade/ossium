#ifndef SCHEMAMODEL_H
#define SCHEMAMODEL_H

#include <SDL.h>
#include <functional>

#include "stringintern.h"
#include "basics.h"
#include "jsondata.h"

using namespace std;

namespace Ossium
{

    ///
    /// Schema
    ///

    template<class BaseType, unsigned int MaximumMembers = 20>
    struct Schema
    {
        const static unsigned int MaxMembers = MaximumMembers;

        typedef unsigned int MemberIdent;

        static unsigned int AddMember(const char* type, const char* name, size_t mem_size,
                                      function<bool(void*, const char*, string)> lambdaFromString, function<string(void*, const char*)> lambdaToString, const char* ultimate_name)
        {
            DEBUG_ASSERT(count < MaximumMembers, "Exceeded maximum number of members. Please allocate a higher maximum in the Schema.");
            member_names[count] = name;
            member_types[count] = type;
            member_byte_offsets[count] = mem_size;
            member_from_string[count] = lambdaFromString;
            member_to_string[count] = lambdaToString;
            schema_name = ultimate_name;
            count++;
            return count - 1;
        }

        /// This root schema has no members of it's own.
        static unsigned int GetMemberCount()
        {
            return 0;
        }

        static const char* GetMemberType(unsigned int index)
        {
            return member_types[index];
        }

        static const char* GetMemberName(unsigned int index)
        {
            return member_names[index];
        }

        void* GetMember(unsigned int index)
        {
            return (void*)((size_t)((void*)this) + member_byte_offsets[index]);
        }

        void FromString(string& str)
        {
            JSON data(str);
            SerialiseIn(&data);
        }

        /// Creates a JSON string with all the schema members.
        string ToString()
        {
            JSON data;
            for (unsigned int i = 0; i < count; i++)
            {
                /// Key consists of type and member name
                string key = member_names[i];
                void* member = GetMember(i);
                if (member != nullptr)
                {
                    /// Value is obtained directly from the member
                    string value = member_to_string[i](member, member_types[i]);
                    /// Add the key-value pair to the JSON object
                    data[key] = value;
                }
                else
                {
                    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not get member '%s' at index [%d] during schema ToString()!", key.c_str(), i);
                }
            }
            return Utilities::ToString(data);
        }

        /// Creates key-values pairs using all members of the local schema hierarchy with the provided JSON object.
        void SerialiseOut(JSON& data)
        {
            for (unsigned int i = 0; i < count; i++)
            {
                /// Key consists of type and member name
                string key = member_names[i];
                /// Value is obtained directly from the member
                void* member = GetMember(i);
                if (member != nullptr)
                {
                    /// Add the key-value pair to the JSON object
                    data[key] = member_to_string[i](member, member_types[i]);
                }
                else
                {
                    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not get member '%s' at index [%d] during serialisation!", key.c_str(), i);
                }
            }
        }

        /// Sets the values of all members in the local schema hierarchy using a JSON object representation of the schema
        void SerialiseIn(JSON& data)
        {
            for (unsigned int i = 0; i < count; i++)
            {
                string key = string(member_names[i]);
                auto itr = data.find(key);
                if (itr != data.end())
                {
                    bool success = member_from_string[i](GetMember(i), member_types[i], itr->second);
                    if (!success)
                    {
                        SDL_LogWarn(SDL_LOG_CATEGORY_ASSERT, "Failed to serialise member '%s' of type '%s'.", member_names[i], member_types[i]);
                    }
                }
                else
                {
                    SDL_LogWarn(SDL_LOG_CATEGORY_ASSERT, "Could not find member '%s' of type '%s' in provided JSON data during serialisation.", member_names[i], member_types[i]);
                }
            }
        }

        /// Returns the ultimate name of this schema
        static const char* GetSchemaName()
        {
            return schema_name;
        }

    protected:
        static size_t member_byte_offsets[MaximumMembers];

    private:
        static function<bool(void*, const char*, string)> member_from_string[MaximumMembers];
        static function<string(void*, const char*)> member_to_string[MaximumMembers];
        static const char* member_names[MaximumMembers];
        static const char* member_types[MaximumMembers];
        /// The name of the final schema in the local schema hierarchy
        static const char* schema_name;
        /// Total members altogether
        static unsigned int count;

    };

    template<class BaseType, unsigned int MaximumMembers>
    function<bool(void*, const char*, string)> Schema<BaseType, MaximumMembers>::member_from_string[MaximumMembers];

    template<class BaseType, unsigned int MaximumMembers>
    function<string(void*, const char*)> Schema<BaseType, MaximumMembers>::member_to_string[MaximumMembers];

    template<class BaseType, unsigned int MaximumMembers>
    size_t Schema<BaseType, MaximumMembers>::member_byte_offsets[MaximumMembers];

    template<class BaseType, unsigned int MaximumMembers>
    const char* Schema<BaseType, MaximumMembers>::member_names[MaximumMembers];

    template<class BaseType, unsigned int MaximumMembers>
    const char* Schema<BaseType, MaximumMembers>::member_types[MaximumMembers];

    template<class BaseType, unsigned int MaximumMembers>
    const char* Schema<BaseType, MaximumMembers>::schema_name = "";

    template<class BaseType, unsigned int MaximumMembers>
    unsigned int Schema<BaseType, MaximumMembers>::count = 0;

    ///
    /// SchemaRoot
    ///

    class SchemaRoot
    {
    public:
        constexpr static unsigned int GetMemberCount()
        {
            return 0;
        }

        constexpr static void* GetMember(unsigned int index)
        {
            return nullptr;
        }

        constexpr static const char* GetMemberName(unsigned int index)
        {
            return "(null)";
        }

        constexpr static const char* GetMemberType(unsigned int index)
        {
            return "(null)";
        }

        static void SerialiseOut(JSON& data)
        {
        }

        static void SerialiseIn(JSON& data)
        {
        }

        constexpr static const char* GetSchemaName()
        {
            return "";
        }

        constexpr static unsigned int GetLocalSchemaDepth()
        {
            return 0;
        }

    };

    ///
    /// MemberInfo
    ///

    template<typename SchemaType, typename Type, typename strType, typename strName>
    struct MemberInfo
    {
        MemberInfo(unsigned int& m_count, function<bool(void*, const char*, string)> lambdaFromString, function<string(void*, const char*)> lambdaToString, const char* ultimate_name, size_t member_offset)
        {
            ++m_count;
            index = SchemaType::AddMember(strType::str, strName::str, member_offset, lambdaFromString, lambdaToString, ultimate_name);
        }

        inline static const char* type = strType::str;
        inline static const char* name = strName::str;

        static unsigned int GetIndex()
        {
            return index;
        }

        Type operator=(const Type& setDefault)
        {
            default_value = setDefault;
            return default_value;
        }

        Type GetDefaultValue()
        {
            return default_value;
        }

        operator Type()
        {
            return default_value;
        }

    private:
        static unsigned int index;

        /// The default value of the member
        Type default_value;

    };

    template<typename SchemaType, typename Type, typename strType, typename strName>
    unsigned int MemberInfo<SchemaType, Type, strType, strName>::index = 0;

    #define DECLARE_SCHEMA(TYPE, BASE_SCHEMA_TYPE)                                                              \
            private: typedef BASE_SCHEMA_TYPE BaseSchemaType;                                                   \
            inline static unsigned int schema_local_count;                                                      \
            inline static TYPE* schema_layout_ref;                                                              \
            constexpr static const char* schema_local_typename = SID(#TYPE)::str;                               \
            public:                                                                                             \
            static unsigned int GetMemberCount()                                                                \
            {                                                                                                   \
                return schema_local_count + BaseSchemaType::GetMemberCount();                                   \
            }

    /// When you want to specify the maximum number of members of a base schema, use this macro instead e.g.
    /// DECLARE_BASE_SCHEMA(ExampleType, 5) will generate schema code for a base type of Schema<ExampleType, 5>
    /// This is necessary as DECLARE_SCHEMA() can't handle Schema<ExampleType, 5> due to the comma separator.
    #define DECLARE_BASE_SCHEMA(TYPE, MAX_MEMBERS)                                                              \
            private: typedef Schema<TYPE, MAX_MEMBERS> BaseSchemaType;                                          \
            inline static unsigned int schema_local_count;                                                      \
            inline static TYPE* schema_layout_ref;                                                              \
            constexpr static const char* schema_local_typename = SID(#TYPE)::str;                               \
            public:                                                                                             \
            static unsigned int GetMemberCount()                                                                \
            {                                                                                                   \
                return schema_local_count + BaseSchemaType::GetMemberCount();                                   \
            }

    ///
    /// Lambda definitions for serialising a particular member in a string format
    ///

    #define MEMBER_FROM_STRING(TYPE)                                        \
    [](void* member, const char* strtype, string data)                      \
    {                                                                       \
        if (strcmp (strtype, SID(#TYPE )::str ) == 0)                       \
        {                                                                   \
            Utilities::FromString(*(( TYPE *)member), data);                \
            return true;                                                    \
        }                                                                   \
        return false;                                                       \
    }

    #define MEMBER_TO_STRING(TYPE)                                          \
    [](void* member, const char* strtype)                                   \
    {                                                                       \
        if (strcmp(strtype, SID(#TYPE )::str) == 0)                         \
        {                                                                   \
            return Utilities::ToString(*(( TYPE *)member));                 \
        }                                                                   \
        return string("");                                                  \
    }

    /// This uses the wonderful Construct On First Use idiom to ensure that the order of the members is always base class, then derived class
    /// Also checks if the type is a pointer. If so, it gets the custom TO_STRING and FROM_STRING macros.
    #define M(TYPE, NAME)                                                                                                                                       \
            MemberInfo<BaseSchemaType, TYPE , SID(#TYPE ), SID(#NAME ) >& schema_m_##NAME()                                                                     \
            {                                                                                                                                                   \
                static MemberInfo<BaseSchemaType, TYPE , SID(#TYPE ), SID(#NAME ) >* initialised_info =                                                         \
                    new MemberInfo<BaseSchemaType, TYPE , SID(#TYPE ), SID(#NAME ) >(schema_local_count,                                                        \
                                    MEMBER_FROM_STRING( TYPE ), MEMBER_TO_STRING( TYPE ), schema_local_typename, (size_t)((void*)&schema_layout_ref->NAME));    \
                return *initialised_info;                                                                                                                       \
            }                                                                                                                                                   \
            TYPE NAME = schema_m_##NAME()

    #define CONSTRUCT_SCHEMA(BASETYPE, SCHEMA_TYPE)                                                     \
            private: constexpr static unsigned int local_depth = BASETYPE::GetLocalSchemaDepth() + 1;   \
            public:                                                                                     \
            constexpr static unsigned int GetLocalSchemaDepth()                                         \
            {                                                                                           \
                return local_depth;                                                                     \
            }                                                                                           \
            static unsigned int GetMemberCount()                                                        \
            {                                                                                           \
                return SCHEMA_TYPE::GetMemberCount() + BASETYPE::GetMemberCount();                      \
            }                                                                                           \
            static const char* GetMemberName(unsigned int index)                                        \
            {                                                                                           \
                if (index >= BASETYPE::GetMemberCount())                                                \
                {                                                                                       \
                    return SCHEMA_TYPE::GetMemberName(index - BASETYPE::GetMemberCount());              \
                }                                                                                       \
                return BASETYPE::GetMemberName(index);                                                  \
            }                                                                                           \
            static const char* GetMemberType(unsigned int index)                                        \
            {                                                                                           \
                if (index >= BASETYPE::GetMemberCount())                                                \
                {                                                                                       \
                    return SCHEMA_TYPE::GetMemberType(index - BASETYPE::GetMemberCount());              \
                }                                                                                       \
                return BASETYPE::GetMemberType(index);                                                  \
            }                                                                                           \
            void* GetMember(unsigned int index)                                                         \
            {                                                                                           \
                if (index >= BASETYPE::GetMemberCount())                                                \
                {                                                                                       \
                    return SCHEMA_TYPE::GetMember(index - BASETYPE::GetMemberCount());                  \
                }                                                                                       \
                return BASETYPE::GetMember(index);                                                      \
            }                                                                                           \
            void SerialiseIn(JSON& data)                                                                \
            {                                                                                           \
                SCHEMA_TYPE::SerialiseIn(data);                                                         \
                BASETYPE::SerialiseIn(data);                                                            \
            }                                                                                           \
            void SerialiseOut(JSON& data)                                                               \
            {                                                                                           \
                BASETYPE::SerialiseOut(data);                                                           \
                SCHEMA_TYPE::SerialiseOut(data);                                                        \
            }                                                                                           \
            virtual string ToString()                                                                   \
            {                                                                                           \
                JSON data;                                                                              \
                SerialiseOut(data);                                                                     \
                return data.ToString();                                                                 \
            }                                                                                           \
            virtual void FromString(string& str)                                                        \
            {                                                                                           \
                JSON data = JSON(str);                                                                  \
                SerialiseIn(data);                                                                      \
            }

}

#endif // SCHEMAMODEL_H
