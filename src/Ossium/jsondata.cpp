#include <fstream>

#include "jsondata.h"
#include "basics.h"

namespace Ossium
{

    ///
    /// JString
    ///

    bool JString::IsInt()
    {
        return functions::IsInt((string)(*this));
    }
    bool JString::IsFloat()
    {
        return functions::IsFloat((string)(*this));
    }
    bool JString::IsNumber()
    {
        return functions::IsNumber((string)(*this));
    }
    bool JString::IsBool()
    {
        return functions::IsBool((string)(*this));
    }
    bool JString::IsArray()
    {
        return ((string)(*this)).length() > 0 && ((string)(*this))[0] == '[' && ((string)(*this))[((string)(*this)).length() - 1] == ']';
    }
    bool JString::IsJSON()
    {
        return ((string)(*this)).length() > 0 && ((string)(*this))[0] == '{' && ((string)(*this))[((string)(*this)).length() - 1] == '}';
    }
    bool JString::IsString()
    {
        return !IsNumber() && !IsBool() && !IsArray() && !IsJSON();
    }

    float JString::ToFloat()
    {
        return functions::ToFloat((string)(*this));
    }
    int JString::ToInt()
    {
        return functions::ToInt((string)(*this));
    }
    bool JString::ToBool()
    {
        return functions::ToBool((string)(*this));
    }
    vector<JString> JString::ToArray()
    {
        vector<JString> dataArray;
        if (IsArray())
        {
            int objCount = 0;
            int arrayCount = 0;
            bool parsingString = false;
            JString value;
            for (unsigned int i = 0, counti = length(); i < counti; i++)
            {
                if ((*this)[i] == '"' && (!parsingString || (i < 1 || (*this)[i - 1] != '\\')))
                {
                    parsingString = !parsingString;
                }
                if (!parsingString)
                {
                    if ((*this)[i] == ',' && arrayCount == 1 && objCount == 0)
                    {
                        value = strip(value, '\n');
                        value = strip(value);
                        dataArray.push_back(value);
                        value = (string)"";
                        continue;
                    }
                    else if ((*this)[i] == '[')
                    {
                        arrayCount++;
                    }
                    else if ((*this)[i] == ']')
                    {
                        arrayCount--;
                        if (arrayCount < 1)
                        {
                            break;
                        }
                    }
                    else if ((*this)[i] == '{')
                    {
                        objCount++;
                    }
                    else if ((*this)[i] == '}')
                    {
                        objCount--;
                    }
                }
                value += (*this)[i];
            }
        }
        else
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_ASSERT, "Attempted to convert JString value into array, but the value is not an array!");
        }
        return dataArray;
    }
    JSON* JString::ToJSON()
    {
        return new JSON((string)(*this));
    }

    ///
    /// JSON
    ///

    JSON::JSON()
    {
    }

    JSON::JSON(string raw, unsigned int startIndex)
    {
        Parse(raw, startIndex);
    }

    bool JSON::Import(string path)
    {
        ifstream file(path.c_str());
        string toParse = functions::ToString(file);
        file.close();
        if (!Parse(toParse))
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_ASSERT, "Failed to load JSON file '%s'!", path.c_str());
            return false;
        }
        SDL_Log("Imported JSON '%s' successfully.", path.c_str());
        return true;
    }

    void JSON::Export(string path)
    {
        ofstream file(path.c_str());
        file << ToString();
        file.close();
    }

    string JSON::ToString()
    {
        stringstream jsonStream;
        jsonStream.str("");
        string json;
        jsonStream << "{";
        if (!empty())
        {
            for (auto itr = begin(); itr != end();)
            {
                jsonStream << endl;
                if (itr->second.IsString())
                {
                    jsonStream << "    " << "\"" << itr->first << "\"" << " : " << "\"" << itr->second << "\"";
                }
                else
                {
                    jsonStream << "    " << "\"" << itr->first << "\"" << " : " << itr->second;
                }
                if (++itr != end())
                {
                    jsonStream << ",";
                }
            }
        }
        jsonStream << endl << "}" << endl;
        return jsonStream.str();
    }

    bool JSON::Parse(string& json, unsigned int startIndex)
    {
        bool open = false;
        bool keymode = false;
        bool parsingString = false;
        bool isJson = false;
        string key = "";
        string value = "";
        for (unsigned int i = startIndex, counti = json.length(); i < counti; i++)
        {
            if (!open)
            {
                if (json[i] == '{')
                {
                    open = true;
                    keymode = true;
                    isJson = true;
                }
            }
            else
            {
                if (keymode)
                {
                    if (parsingString)
                    {
                        if (json[i] == '"' && ((i > 0 && json[i - 1] != '\\') || (i > 1 && json[i - 2] == '\\')))
                        {
                            parsingString = false;
                        }
                        else
                        {
                            key += json[i];
                        }
                    }
                    else if (json[i] == '"')
                    {
                        parsingString = true;
                    }
                    else if (json[i] == ':')
                    {
                        keymode = false;
                    }
                    else if (json[i] == '}')
                    {
                        open = false;
                    }
                }
                else if (parsingString)
                {
                    if (json[i] == '"' && ((i > 0 && json[i - 1] != '\\') || (i > 1 && json[i - 2] == '\\')))
                    {
                        parsingString = false;
                    }
                    else
                    {
                        value += json[i];
                    }
                }
                else if (json[i] == '"')
                {
                    parsingString = true;
                }
                else if (json[i] == '{')
                {
                    /// Parse JSON into a single string
                    int objDepth = 0;
                    for (; i < counti; i++)
                    {
                        if (json[i] == '{')
                        {
                            objDepth++;
                        }
                        else if (json[i] == '}')
                        {
                            objDepth--;
                            if (objDepth < 1)
                            {
                                value += json[i];
                                break;
                            }
                        }
                        value += json[i];
                    }
                }
                else if (json[i] == '[')
                {
                    /// Parse array into a single string
                    int arrayDepth = 0;
                    for (; i < counti; i++)
                    {
                        if (json[i] == '[')
                        {
                            arrayDepth++;
                        }
                        else if (json[i] == ']')
                        {
                            arrayDepth--;
                            if (arrayDepth < 1)
                            {
                                value += json[i];
                                break;
                            }
                        }
                        value += json[i];
                    }
                }
                else if (json[i] != ' ')
                {
                    if (json[i] == ',' || json[i] == '}')
                    {
                        value = strip(value, '\n');
                        value = strip(value);
                        value = strip(value, '"');
                        /// Add to data lookup
                        (*this)[key] = value;
                        /// Reset temporary values
                        key = "";
                        value = "";
                        if (json[i] == '}')
                        {
                            open = false;
                            break;
                        }
                        keymode = true;
                    }
                    else
                    {
                        value += json[i];
                    }
                }
            }
        }
        if (open || !isJson)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Failed to parse JSON correctly due to bad formatting.");
            return false;
        }
        return true;
    }

}
