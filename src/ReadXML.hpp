#ifndef INC_IONIZATION_LIB_IO_hpp
#define INC_IONIZATION_LIB_IO_hpp

#include <stdint.h> // (u)int64_t

#include "tinyxml.hpp"


// Reading of the XML input file

std::string XML_Get_String( TiXmlNode *root, const std::string &elements);
bool        XML_Get_Bool(   TiXmlNode *root, const std::string &elements);
int         XML_Get_Int(    TiXmlNode *root, const std::string &elements);
long int    XML_Get_LInt(   TiXmlNode *root, const std::string &elements);
uint64_t    XML_Get_UInt64( TiXmlNode *root, const std::string &elements);
int64_t     XML_Get_Int64(  TiXmlNode *root, const std::string &elements);
double      XML_Get_Double( TiXmlNode *root, const std::string &elements);

std::string XML_Get_Attribute_String(TiXmlNode *root, const std::string &attribute);
int         XML_Get_Attribute_Int(   TiXmlNode *root, const std::string &attribute);
double      XML_Get_Attribute_Double(TiXmlNode *root, const std::string &attribute);
double      XML_Get_Attribute_Double(TiXmlNode *root, const std::string &elements, const std::string &attribute);

TiXmlNode * XML_Get_SubNode(TiXmlNode *RootNode, const std::string &celements);
TiXmlNode * XML_Get_SubNode_Matching_Attribute(TiXmlNode *root, const std::string &elements,
                                               const std::string &attribute,
                                               const std::string &attribute_value);
TiXmlNode * XML_Get_SubNode_Matching_Attribute(TiXmlNode *root, const std::string &elements,
                                               const std::string &attribute, const int attribute_value);
TiXmlNode * XML_Get_SubNode_Matching_Attribute(TiXmlNode *root, const std::string &elements,
                                               const std::string &attribute1, const int attribute1_value,
                                               const std::string &attribute2, const int attribute2_value);

std::string XML_Read_Attribute(TiXmlDocument &input, TiXmlNode *root, const std::string &path,
                              const std::string &attribute);
void        XML_Verify_Attribute(TiXmlDocument &input, TiXmlNode *root, const std::string &path,
                                 const std::string &attribute, const std::string &value);
void        XML_Verify_Units(TiXmlDocument &input, TiXmlNode *root, const std::string &path,
                             const std::string &units);
bool        XML_Get_Enable(TiXmlDocument &input, TiXmlNode *root, const std::string &path);
double      Get_Unit_Factor_Length(const std::string &unit);

int XML_Count_Elements(TiXmlNode *root, const std::string &element);


#endif // INC_IONIZATION_LIB_IO_hpp

// ********** End of file ***************************************
