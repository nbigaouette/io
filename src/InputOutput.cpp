
#include <cstdlib>  // abort()
#include <iostream>
#include <iomanip>  // setw() & setfill()
#include <cmath>    // std::abs()

#include <StdCout.hpp>

#include "Constants.hpp"
#include "InputOutput.hpp"

#define DEBUGP(x)  std_cout << __FILE__ << ":" << __LINE__ << ":\n    " << x;

// **************************************************************
FILE * Open_File(const std::string &filename, const std::string &mode, const bool quiet)
{
    FILE *fh;

    if (!quiet)
        std_cout << "Opening file \"" << filename << "\" in mode '" << mode << "'..." << std::endl;

    fh = fopen(filename.c_str(), mode.c_str());
    if (fh == NULL)
    {
        std::cerr << "Could not open file \"" << filename << "\" in mode '" << mode << "'!" << std::endl;
        std::cerr << "Aborting.\n" << std::flush;
        abort();
    }

    return fh;
}

// **************************************************************
inline std::string Pause(std::string msg = std::string(""))
{
    std::string answer;
    if (msg != std::string(""))
    {
        std_cout << msg << std::endl;
    }
    getline(std::cin, answer);

    return answer;
}

// **************************************************************
IO::IO()
{
    period                  = 0.0;
    last_saved_time         = 0.0;
    force_at_next_iteration = false;
    enable                  = false;
    using_C_fh              = false;
    mode                    = '\0';
    binary                  = false;
}

// **************************************************************
IO::~IO()
{
    this->Close_File();
}

// **************************************************************
void IO::Init(const double _period, const std::string _filename, const bool _binary)
{
    Set_Period(_period);
    filename = _filename;
    binary   = _binary;
}

// **************************************************************
void IO::Set_Period(const double _period)
{
    Disable();

    if (std::abs(_period) >= 1.0e-30)
        enable  = true;

    period = _period;
}

// **************************************************************
void IO::Set_Filename(const std::string _full_filename)
{
    filename = _full_filename;
}

// **************************************************************
void IO::Set_Filename(const std::string _path, const std::string _filename)
{
    filename = _path + "/" + _filename;
}

// **************************************************************
void IO::Disable()
{
    enable = false;
}

// **************************************************************
void IO::Open_File(const std::string full_mode, const bool quiet, const bool _using_C_fh)
{
    assert(enable);

    using_C_fh = _using_C_fh;

    std::ios_base::openmode file_openmode;
    if   (full_mode.find("w") != std::string::npos)
    {
        mode = 'w';
        file_openmode = std::fstream::out;
    }
    else if (full_mode.find("r") != std::string::npos)
    {
        mode = 'r';
        file_openmode = std::fstream::in;
    }
    else
    {
        std_cout << "ERROR: Unknown mode '" << full_mode << "'. Exiting.\n";
        abort();
    }

    if (full_mode.find("b") != std::string::npos)
    {
        binary = true;
        file_openmode |= std::fstream::binary;
    }

    bool retry = true;
    while (retry)
    {
        if (!quiet)
            std_cout << "Opening file \"" << filename << "\" for '" << full_mode << "'...\n";

        if (using_C_fh)
        {
            C_fh = fopen(filename.c_str(), full_mode.c_str());

            // Verify that the file is opened.
            if (C_fh == NULL)
            {
                std::cerr << "Could not open file \"" << filename << "\" for '" << full_mode << "'...\n";
                std::string answer = Pause("Retry? [y/N]");
                if (! (answer == "y" || answer == "yes"))
                {
                    std_cout << std::flush;
                    abort();
                }
            } else {
                retry = false;
            }
        } else {
            fh.open(filename.c_str(), file_openmode);

            // Verify that the file is opened.
            if (!fh.is_open())
            {
                std::cerr << "Could not open file \"" << filename << "\" for '" << full_mode << "'...\n";
                std::string answer = Pause("Retry? [y/N]");
                if (! (answer == "y" || answer == "yes"))
                {
                    std_cout << std::flush;
                    abort();
                }
            } else {
                retry = false;
            }
        }
    }
}

// **************************************************************
bool IO::Is_Output_Permitted(const double time, const bool dont_set_previous_period)
/**
 *
 * @param time: Simulation time [seconds]
 */
{
    if (!enable)
        return false;

    if (force_at_next_iteration)
    {
        std_cout << "Output forced...\n";
        force_at_next_iteration = false;
        return true;
    }

    // Initial save (time == 0.0)
    if (time < 1.0e-10)
        return true;

    // Period of -1 means always save
    if (-1.001 < period && period < -0.999)
        return true;

    // Period of -2 means is handled elsewhere. Used to force
    // output only when ionization occured.
    if (-2.001 < period && period < -1.999)
    {
        return false;
    }

    const double nb_periods = std::floor(time / period);

    const double time_previous_period = nb_periods * period;

    if (time_previous_period > last_saved_time)
    {
        if (!dont_set_previous_period)
            last_saved_time = time_previous_period;
        return true;
    }

    return false;
}

// **************************************************************
void IO::Close_File()
{
    if (using_C_fh)
    {
        if (C_fh != NULL)
            fclose(C_fh);
        C_fh = NULL;
    } else {
        if (fh.is_open())
            fh.close();
    }
}

// **************************************************************
void IO::Flush()
{
    if (using_C_fh)
    {
        fflush(C_fh);
    } else {
        if (fh.is_open() && mode == 'w')
            fh.flush();
    }
}

// **************************************************************
void IO::Format(const int width,
                    const int nb_after_dot,
                    const char type,
                    const char justify,
                    const char fill)
/**
 * Set the output format. Similar to printf() format.
 * @param width         Output's width (in characters)
 * @param nb_after_dot  Number of digits after decimal
 * @param type          Character describing the type
 * @param justify       Left (l) or right (r) justification.
 *                      Default to right.
 * @param fill          Filling character (default to space)
 *
 * Examples:
 *      Format(3, 0, 'd')       equivalent to printf("%3d", [...])
 *      Format(11, 4, 'f')      equivalent to printf("%11.4f", [...])
 *      Format(-1, 4, 'f')      equivalent to printf("%.4f", [...])
 *      Format(12, 6, 'g', 'l') equivalent to printf("%-12.6g", [...])
 */
{
    assert(!using_C_fh);

    if (width > 0)
        fh << std::setw(width);

    if (type != 'd')
    {
        // If not an integer, set the precision.
        fh << std::setprecision(nb_after_dot);
    }
    if (justify == 'r')
    {
        fh << std::right;
    } else {
        fh << std::left;
    }
    fh << std::setfill(fill);
}

// **************************************************************
void IO::Print()
{
    std_cout
        << "Output:" << std::endl
        << "    filename:                " << filename << std::endl
        << "    enable:                  " << (enable ? "yes" : "no ") << std::endl
        << "    period:                  " << period << std::endl
        << "    last_saved_time:         " << last_saved_time << std::endl
        << "    Style:                   " << (using_C_fh ? "C" : "C++") << std::endl
        << "    is_open():               " << (Is_Open() ? "yes" : "no") << std::endl
        << "    mode:                    " << (Is_Open() ? mode : '-') << std::endl
        << "    binary:                  " << (binary ? "yes" : "no ") << std::endl
        << "    force_at_next_iteration: " << (force_at_next_iteration ? "yes" : "no ") << std::endl
    ;
}

// **************************************************************
ReadXML::ReadXML(const std::string name, const std::string _filename)
{
    this->Open(name, _filename);
}

// **************************************************************
void ReadXML::Open(const std::string _name, const std::string _filename)
{
    name        = _name;
    filename    = _filename;

    // Open XML file
    Document = TiXmlDocument(filename);
    //Handle   = TiXmlHandle(&Document);

    // Load it in memory and check
    if (!Document.LoadFile())
    {
        std_cout << "Error reading XML file '" << filename << "'\n";
        abort();
    }
    else
        std_cout << "Reading XML file '" << filename << "'\n";

    RootNode = Document.FirstChild(name);

}

// **************************************************************
void ReadXML::Dump(const std::string _filename)
/**
 * Dump XML input file
 */
{
    std_cout << "Dumping XML input file to " << _filename << "\n";
    if (!Document.SaveFile(_filename))
    {
        std_cout << "Can't save XML file to " << _filename << "!\n" << std::flush;
        abort();
    }
}

// **************************************************************
void ReadXML::Split_String(const std::string &to_split, const char delimiter, std::vector<std::string> &split_strings)
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
TiXmlNode * ReadXML::Get_SubNode(const std::string elements)
{
    // Temporary node
    TiXmlNode *subnode = RootNode;

    // Split the XML path string into many substrings, saved in a vector
    std::vector<std::string> subelements;
    Split_String(elements, '/', subelements);

    for (std::vector<std::string>::iterator it = subelements.begin() ; it < subelements.end() ; it++ )
    {
        // For each substrings, get the first child
        subnode = subnode->FirstChild(it->c_str());
    }

    return subnode;
}

// **************************************************************
TiXmlNode * ReadXML::Get_SubNode(TiXmlNode *RootNode, const std::string &celements)
{
    // Temporary node
    TiXmlNode *subnode = RootNode;

    // Split the XML path string into many substrings, saved in a vector
    std::vector<std::string> subelements;
    Split_String(celements, '/', subelements);

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

std::string ReadXML::Get_String(const std::string element)
{
    std::string buff;
    TiXmlNode *subnode = Get_SubNode(element);
    if (subnode != NULL)
        buff = std::string(subnode->FirstChild()->Value());
    else
    {
        std_cout << "ReadXML::Get_String() failed!\n";
        std_cout << "Element \"" << element << "\" not found!\n" << std::flush;
        abort();
    }

    return buff;
}

// **************************************************************
bool ReadXML::Get_Bool(const std::string element)
{
    std::string temp = Get_String(element);

    bool return_value;

    if      (temp == "yes" || temp == "on")
        return_value = true;
    else if (temp == "no"  || temp == "off")
        return_value = false;
    else
    {
        std_cout
            << "Error interpreting boolean value of '" << element << "'" << std::endl
            << "String read was '" << temp << "'\n" << std::flush;
        abort();
    }

    return return_value;
}

// **************************************************************
bool ReadXML::Get_Enable(const std::string element)
{
    std::string temp = Get_Attribute_String(element, "enable");

    bool return_value;

    if      (temp == "yes" || temp == "on")
        return_value = true;
    else if (temp == "no"  || temp == "off")
        return_value = false;
    else
    {
        std_cout
            << "Error interpreting boolean 'enable' value of '" << element << "'" << std::endl
            << "String read was '" << temp << "'\n" << std::flush;
        abort();
    }

    return return_value;
}

// **************************************************************
double ReadXML::Get_Double(const std::string element)
{
    return double(atof(Get_String(element).c_str()));
}

// **************************************************************
int ReadXML::Get_Int(const std::string element)
{
    return atoi(Get_String(element).c_str());
}

// **************************************************************
long int ReadXML::Get_LInt(const std::string element)
{
    return atol(Get_String(element).c_str());
}

// **************************************************************
int64_t ReadXML::Get_Int64(const std::string element)
{
    return int64_t(atof(Get_String(element).c_str()));
}

// **************************************************************
uint64_t ReadXML::Get_UInt64(const std::string element)
{
    const double tmp_double = double(atof(Get_String(element).c_str()));
    assert(tmp_double >= 0.0);
    return uint64_t(tmp_double);
}

// **************************************************************
double ReadXML::Get_Double_Time_Second(const std::string element)
{
    return (Factor_Unit_To_Second(element) * Get_Double(element));
}

// **************************************************************
double ReadXML::Get_Double_Length_Meter(const std::string element)
{
    return (Factor_Unit_To_Metre(element) * Get_Double(element));
}

// **************************************************************
double ReadXML::Get_Double_Time_AU(const std::string element)
{
    return (Factor_Unit_To_Time_AU(element) * Get_Double(element));
}

// **************************************************************
double ReadXML::Get_Double_Length_AU(const std::string element)
{
    return (Factor_Unit_To_Length_AU(element) * Get_Double(element));
}

// **************************************************************
std::string ReadXML::Get_Attribute_String(const std::string element, const std::string attribute)
{
    TiXmlNode *SubNode = Get_SubNode(element);
    if (SubNode == NULL)
    {
        std_cout << "ReadXML::Get_Attribute_String() failed!\n";
        std_cout << "Element \"" << element << "\" not found!\n" << std::flush;
        abort();
    }

    const char *found_attribute = SubNode->ToElement()->Attribute(attribute.c_str());
    if (found_attribute == NULL)
    {
        std_cout << "ERROR: Attribute '" << attribute << "' in '" << element << "' not found!\n";
        abort();
    }

    return std::string(found_attribute);
}

// **************************************************************
bool ReadXML::Get_Attribute_Bool(const std::string element, const std::string attribute)
{
    std::string temp = Get_Attribute_String(element, attribute);

    bool return_value;

    if      (temp == "yes" || temp == "on")
        return_value = true;
    else if (temp == "no"  || temp == "off")
        return_value = false;
    else
    {
        std_cout
            << "Error intepreting boolean value of '" << element << "'" << std::endl
            << "String read was '" << temp << "'\n" << std::flush;
        abort();
    }

    return return_value;
}

// **************************************************************
double ReadXML::Get_Attribute_Double(const std::string element, const std::string attribute)
{
    return double(atof(Get_Attribute_String(element, attribute).c_str()));
}

// **************************************************************
int ReadXML::Get_Attribute_Int(const std::string element, const std::string attribute)
{
    return atoi(Get_Attribute_String(element, attribute).c_str());
}

// **************************************************************
long int ReadXML::Get_Attribute_LInt(const std::string element, const std::string attribute)
{
    return atol(Get_Attribute_String(element, attribute).c_str());
}

// **************************************************************
int64_t ReadXML::Get_Attribute_Int64(const std::string element, const std::string attribute)
{
    return int64_t(atof(Get_Attribute_String(element, attribute).c_str()));
}

// **************************************************************
uint64_t ReadXML::Get_Attribute_UInt64(const std::string element, const std::string attribute)
{
    const double tmp_double = double(atof(Get_Attribute_String(element, attribute).c_str()));
    assert(tmp_double >= 0.0);
    return uint64_t(tmp_double);
}

// **************************************************************
std::string ReadXML::Get_Unit(const std::string element)
{
    return Get_Attribute_String(element, "unit");
}

// **************************************************************
void ReadXML::Set_String(const std::string element, const std::string value)
{
    // Get the subnode
    TiXmlNode *subnode = Get_SubNode(element);
    // Set value
    subnode->FirstChild()->SetValue(value);
}

// **************************************************************
int ReadXML::Count_Elements(TiXmlNode *root, const std::string &element)
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
void ReadXML::Verify_Attribute(const std::string element, const std::string attribute, const std::string value)
{
    const std::string read_attribute_value = Get_Attribute_String(element, attribute);

    if (read_attribute_value != value)
    {
        std_cout
            << std::endl << std::endl
            << "ERROR in XML input file!" << std::endl
            << "Attribute \"" << attribute << "\" in XML path \"" << element
            << "\" read (" << read_attribute_value << ")" << std::endl
            << "does not match the needed value (" << value << ")" << std::endl << std::endl << std::flush;
        abort();
    }
}

// **************************************************************
void ReadXML::Verify_Unit(const std::string element, const std::string unit)
{
    Verify_Attribute(element, "unit", unit);
}

// **************************************************************
bool ReadXML::Is_Enable(const std::string element)
{
    return Get_Attribute_Bool(element, "enable");
}

// **************************************************************
double ReadXML::Factor_Unit_To_Metre(const std::string element)
{
    const std::string unit = Get_Unit(element);
    double unit_factor = 0.0;

    if      (unit == "au")      unit_factor = libtinyxml::bohr_to_m;
    else if (unit == "bohr")    unit_factor = libtinyxml::bohr_to_m;
    else if (unit == "angstrom")unit_factor = libtinyxml::angstrom_to_m;
    else if (unit == "nm")      unit_factor = libtinyxml::nm_to_m;
    else if (unit == "mum")     unit_factor = double(1.0e-6);
    else if (unit == "mm")      unit_factor = double(1.0e-3);
    else if (unit == "cm")      unit_factor = double(1.0e-2);
    else if (unit == "m")       unit_factor = double(1.0);
    else
    {
        std_cout
            << "Error in reading XML input file:\n"
            << "Unit of " << element << " needs to be either:\n"
            << "    au\n    bohr\n    angstrom\n    nm\n    mum\n    mm\n    cm\n    m\n"
            << "Aborting\n";
        abort();
    }

    return unit_factor;
}

// **************************************************************
double ReadXML::Factor_Unit_To_Second(const std::string element)
{
    const std::string unit = Get_Unit(element);
    double unit_factor = 0.0;

    if      (unit == "au")  unit_factor = libtinyxml::au_to_si_time;
    else if (unit == "as")  unit_factor = libtinyxml::as_to_s;
    else if (unit == "fs")  unit_factor = libtinyxml::fs_to_s;
    else if (unit == "ps")  unit_factor = libtinyxml::ps_to_s;
    else if (unit == "ns")  unit_factor = libtinyxml::ns_to_s;
    else if (unit == "mus") unit_factor = libtinyxml::mus_to_s;
    else if (unit == "ms")  unit_factor = libtinyxml::ms_to_s;
    else if (unit == "s")   unit_factor = 1.0;
    else
    {
        std_cout
            << "Error in reading XML input file:\n"
            << "Unit of " << element << " needs to be either:\n"
            << "    au\n    as\n    fs\n    ps\n    ns\n    mus\n    ms\n    s\n"
            << "Aborting\n";
        abort();
    }

    return unit_factor;
}

// **************************************************************
double ReadXML::Factor_Unit_To_Length_AU(const std::string element)
{
    return libtinyxml::si_to_au_length * Factor_Unit_To_Metre(element);
}

// **************************************************************
double ReadXML::Factor_Unit_To_Time_AU(const std::string element)
{
    return libtinyxml::si_to_au_time * Factor_Unit_To_Second(element);
}

// ********** End of file ***************************************