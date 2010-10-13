/****************************************************************

    Routines for reading XML input files

****************************************************************/

// using namespace std;

#include <iostream>
#include <stdint.h> // (u)int64_t
#include <vector>

#include <StdCout.hpp>
// #include "General.hpp"
#include "tinyxml.hpp"
#include "ReadXML.hpp"

#include "Constants.hpp"
using namespace libtinyxml;

#define DEBUGP(x)  std_cout << __FILE__ << ":" << __LINE__ << ":\n    " << x;

// **************************************************************
// ********** Local functions prototypes ************************
// **************************************************************
void XML_SplitString(const std::string &to_split, const char delimiter,
                     std::vector<std::string> &split_strings);

int XML_Count_Elements(TiXmlNode *root, const std::string &element);

// **************************************************************
// ********** Accessible functions implementations **************
// **************************************************************


// **************************************************************
TiXmlNode * XML_Get_SubNode_Matching_Attribute(TiXmlNode *root,
                                               const std::string &elements,
                                               const std::string &attribute,
                                               const int attribute_value)
{
    TiXmlNode *subnode = root->FirstChild(elements);

    int int_read = -1;
    do {
        if (
            subnode->ToElement()->QueryIntAttribute(attribute, &int_read)
            != TIXML_SUCCESS
            )
        {
            DEBUGP("ERROR: XML_Get_SubNode_Matching_Attribute() failed\n");
            std_cout
                << "root: <" << root->Value() << "> (" << root << "), "
                << "elements: " << elements << ", "
                << "attribute: " << attribute << ", "
                << "attribute_value: " << attribute_value
                << std::endl << std::flush;
            abort();
        }
    } while (
        int_read != attribute_value &&
        (subnode = root->IterateChildren(subnode)) != 0
    );
    if (subnode == NULL)
    {
        std_cout << std::flush;
        abort();
    }
    else
        return subnode;
}

// **************************************************************
TiXmlNode * XML_Get_SubNode_Matching_Attribute(TiXmlNode *root,
                                               const std::string &elements,
                                               const std::string &attribute1,
                                               const int attribute1_value,
                                               const std::string &attribute2,
                                               const int attribute2_value)
{
    TiXmlNode *subnode = root->FirstChild(elements);

    int int_read1 = -1;
    int int_read2 = -1;
    do {
        if (
            subnode->ToElement()->QueryIntAttribute(attribute1, &int_read1)
            != TIXML_SUCCESS
        ||
            subnode->ToElement()->QueryIntAttribute(attribute2, &int_read2)
            != TIXML_SUCCESS
        )
        {
            DEBUGP("ERROR: XML_Get_SubNode_Matching_Attribute() (2) failed\n");
            std_cout
                << "root: <" << root->Value() << "> (" << root << "), "
                << "elements: " << elements << ", "
                << "attribute1: " << attribute1 << ", "
                << "attribute1_value: " << attribute1_value << ", "
                << "attribute2: " << attribute2 << ", "
                << "attribute2_value: " << attribute2_value << ", "
                << "(*subnode)->Value(): " << subnode->Value()
                << std::endl << std::flush;
            abort();
        }
    } while (
        !(int_read1 == attribute1_value && int_read2 == attribute2_value) &&
        ((subnode = root->IterateChildren(subnode)) != 0) &&
        (subnode->Value() == elements)
    );
    if (subnode == NULL)
    {
        std_cout << std::flush;
        abort();
    }
    else
        return subnode;
}

// **************************************************************
TiXmlNode * XML_Get_SubNode_Matching_Attribute(TiXmlNode *root,
                                               const std::string &elements,
                                               const std::string &attribute,
                                               const std::string &attribute_value)
{
    // Get the subnode pointed by "elements"
    TiXmlNode *subnode = XML_Get_SubNode(root, elements);

    // This is the word after the last "/" in the "elements" string
    std::string string_last_element = std::string(subnode->Value());

    // subnode is actually at the "string_last_element" node.
    // We want its parent.
    subnode = subnode->Parent();

    // Go through all child and find the right one with the matching attribute
    for (TiXmlNode *child = subnode->FirstChild() ; child ; child = child->NextSibling() )
    {
        // Skip anything with wrong value (for example, comments)
        if (std::string(child->Value()) != string_last_element) continue;
        // Check if attribute's value of current child match the wanted one's value
        if ( std::string(child->ToElement()->Attribute(attribute.c_str())) == attribute_value )
            subnode = child;
    }

    if (subnode == NULL)
    {
        std_cout << std::flush;
        abort();
    }
    else
        return subnode;
}

// **************************************************************
void XML_SplitString(const std::string &to_split, const char delimiter,
                     std::vector<std::string> &split_strings)
{
    split_strings.clear();

    // Find first delimiter
    size_t slash_pos0 = 0;
    // Find next delimiter character
    size_t slash_pos1 = to_split.find(delimiter);

    // Search string only if it contains a delimiter character
    while ( slash_pos1 != std::string::npos )
    {
        // Add the substring to the vector, omitting the delimiter
        split_strings.push_back(to_split.substr(slash_pos0, slash_pos1-slash_pos0));

        slash_pos0 = slash_pos1+1;
        slash_pos1 = to_split.find(delimiter, slash_pos0);
    }

    // The last substring (or the only one) is always skipped, so add it here
    split_strings.push_back(to_split.substr(slash_pos0, slash_pos1-slash_pos0));
}

// **************************************************************
TiXmlNode * XML_Get_SubNode(TiXmlNode *RootNode, const std::string &celements)
{
    // Temporary node
    TiXmlNode *subnode = RootNode;

    // Split the XML path string into many substrings, saved in a vector
    std::vector<std::string> subelements;
    XML_SplitString(celements, '/', subelements);

    for (std::vector<std::string>::iterator it = subelements.begin() ; it < subelements.end() ; it++ )
    {
        if (subnode == NULL)
        {
            std_cout
                << "XML_Get_SubNode() failed!\n"
                << "Element \"" << celements << "\" not found!\n" << std::flush;
                abort();
        }
        // For each substrings, get the first child
        subnode = subnode->FirstChild(it->c_str());
    }

    return subnode;
}

// **************************************************************
std::string XML_Get_String(TiXmlNode *root, const std::string &elements)
{
    std::string buff;
    TiXmlNode *subnode = XML_Get_SubNode(root, elements.c_str());
    if (subnode != NULL)
        buff = std::string(subnode->FirstChild()->Value());
    else
    {
        std_cout
            << "XML_Get_String() failed!\n"
            << "Element \"" << elements << "\" not found!\n" << std::flush;
        abort();
    }

    return buff;
}

// **************************************************************
int XML_Get_Attribute_Int(TiXmlNode *root, const std::string &attribute)
{
    int read_attribute = -1;
    if (
        root->ToElement()->QueryIntAttribute(attribute, &read_attribute)
        != TIXML_SUCCESS
        )
    {
        DEBUGP("ERROR: XML_Get_Attribute_Int() failed\n");
        std_cout
            << "root: <" << root->Value() << "> (" << root << "), "
            << "attribute: " << attribute
            << std::endl << std::flush;
        abort();
    }
    return read_attribute;
}

// **************************************************************
double XML_Get_Attribute_Double(TiXmlNode *root, const std::string &elements,
                                const std::string &attribute)
{
    TiXmlNode *subnode = XML_Get_SubNode(root, elements);
    return XML_Get_Attribute_Double(subnode, attribute);
}

// **************************************************************
double XML_Get_Attribute_Double(TiXmlNode *root, const std::string &attribute)
{
    double read_attribute = -1.0;
    if (
        root->ToElement()->QueryDoubleAttribute(attribute, &read_attribute)
        != TIXML_SUCCESS
        )
    {
        DEBUGP("ERROR: XML_Get_Attribute_Double() failed\n");
        std_cout
            << "root: <" << root->Value() << "> (" << root << "), "
            << "attribute: " << attribute
            << std::endl << std::flush;
        abort();
    }
    return read_attribute;
}

// **************************************************************
std::string XML_Get_Attribute_String(TiXmlNode *root, const std::string &attribute)
{
    return std::string(root->ToElement()->Attribute(attribute.c_str()));
}

// **************************************************************
bool XML_Get_Bool(TiXmlNode *root, const std::string &elements)
{
    std::string temp = XML_Get_String(root, elements);

    bool return_value;

    if      (temp == "yes" || temp == "on")
        return_value = true;
    else if (temp == "no"  || temp == "off")
        return_value = false;
    else
    {
        DEBUGP("");
        std_cout
            << "Error intepreting boolean value of '" << elements << "'" << std::endl
            << "String read was '" << temp << "'\n" << std::flush;
        abort();
    }

    return return_value;
}

// **************************************************************
int XML_Get_Int(TiXmlNode *root, const std::string &elements)
{
    std::string temp = XML_Get_String(root, elements);
    return atoi(temp.c_str());
}

// **************************************************************
long int XML_Get_LInt(TiXmlNode *root, const std::string &elements)
{
    std::string temp = XML_Get_String(root, elements);
    return atol(temp.c_str());
}

// **************************************************************
uint64_t XML_Get_UInt64(TiXmlNode *root, const std::string &elements)
{
    std::string temp = XML_Get_String(root, elements);
    return uint64_t(atof(temp.c_str()));
}

// **************************************************************
int64_t  XML_Get_Int64(TiXmlNode *root, const std::string &elements)
{
    std::string temp = XML_Get_String(root, elements);
    return int64_t(atof(temp.c_str()));
}

// **************************************************************
double XML_Get_Double(TiXmlNode *root, const std::string &elements)
{
    std::string temp = XML_Get_String(root, elements);
    return atof(temp.c_str());
}

// **************************************************************
int XML_Count_Elements(TiXmlNode *root, const std::string &element)
{
    int count = 0;

    // The pointer needs to be null initially for IterateChildren()
    // to work.
    TiXmlNode *subnode = NULL;
    subnode = root->IterateChildren(subnode);
    while ( subnode != 0 )
    {
        // If the name of the subnode is really the element we want
        // to count, increment the counter.
        if (std::string(subnode->Value()) == element)
            count++;
        // Then go to next children of the parent
        subnode = root->IterateChildren(subnode);
    }
    return count;
}

// **************************************************************
std::string XML_Read_Attribute(TiXmlDocument &input, TiXmlNode *root,
                              const std::string &tmp_string, const std::string &attribute)
{
    std::string path_string(tmp_string);

    TiXmlElement *xml_element = input.RootElement();
    std::string sub_path_string;

    while (true)
    {
        std::string::size_type stit = path_string.find_first_of("/");
        if (stit == path_string.size()-1)
            path_string = path_string.substr(0, path_string.size()-1);
        else
        {
            sub_path_string = path_string.substr(0, stit);
            path_string = path_string.substr(stit+1);
            xml_element = xml_element->FirstChildElement(sub_path_string.c_str());
            if (sub_path_string == path_string) break;
        }
    }
    if (xml_element == NULL)
    {
        std_cout
            << "ERROR: XML_Read_Attribute() could not find attribute!" << std::endl
            << "tmp_string = " << tmp_string << std::endl
            << "xml_element = " << xml_element << std::endl
            << "attribute = " << attribute << std::endl << std::flush;
        abort();
    }
    return std::string(xml_element->Attribute(attribute.c_str()));
}

// **************************************************************
void XML_Verify_Attribute(TiXmlDocument &input, TiXmlNode *root,
                          const std::string &path, const std::string &attribute,
                          const std::string &value)
{
    std::string wanted_attribute_value(value);
    std::string read_attribute_value = XML_Read_Attribute(input, root, path, attribute);

    if (read_attribute_value != wanted_attribute_value)
    {
        DEBUGP("");
        std_cout
            << std::endl << std::endl
            << "ERROR in XML input file!" << std::endl
            << "Attribute \"" << attribute << "\" in XML path \"" << path
            << "\" read (" << read_attribute_value << ")" << std::endl
            << "does not match the needed value (" << value << ")" << std::endl << std::endl << std::flush;
        abort();
    }
}

// **************************************************************
void XML_Verify_Units(TiXmlDocument &input, TiXmlNode *root,
                      const std::string &path, const std::string &units)
{
    XML_Verify_Attribute(input, root, path, "unit", units);
}

// **************************************************************
bool XML_Get_Enable(TiXmlDocument &input, TiXmlNode *root, const std::string &path)
{
    std::string tmp_string = XML_Read_Attribute(input, root, path, "enable");
    return ((tmp_string == "yes" || tmp_string == "on") ? true : false );
}

// **************************************************************
double Get_Unit_Factor_Length(const std::string &unit)
{
    double unit_factor = 0.0;

    if      (unit == "bohr")    unit_factor = 1.0;
    else if (unit == "angstrom")unit_factor = angstrom_to_m*m_to_bohr;
    else if (unit == "nm")      unit_factor = nm_to_m*m_to_bohr;
    else if (unit == "mum")     unit_factor = mum_to_m*m_to_bohr;
    else if (unit == "mm")      unit_factor = mm_to_m*m_to_bohr;
    else if (unit == "cm")      unit_factor = cm_to_m*m_to_bohr;
    else if (unit == "m")       unit_factor = m_to_bohr;
    else
    {
        std_cout
            << "Error in reading XML input file:" << std::endl
            << "Unit of particles/box_size needs to be either:" << std::endl
            << "    bohr\n    angstrom\n    nm\n    mum\n    mm\n    cm\n    m\n" << std::endl
            << "Aborting.\n" << std::flush;
        abort();
    }

    return unit_factor;
}

// ********** End of file ***************************************
