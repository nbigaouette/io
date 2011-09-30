
#include <cstdlib>  // abort()
#include <iostream>
#include <iomanip>  // setw() & setfill()
#include <cmath>    // std::abs()
#include <limits>   // std::numeric_limits<>::max()
#include <climits> // CHAR_BIT
#include <sys/stat.h> // Check if folder exists


#include <StdCout.hpp>

#include "Constants.hpp"
#include "InputOutput.hpp"

#define DEBUGP(x)  std_cout << __FILE__ << ":" << __LINE__ << ":\n    " << x;


#ifdef COMPRESS_OUTPUT
#include <zlib.h>
//#define DEFAULT_BUFFER_SIZE 8192
#define DEFAULT_BUFFER_SIZE 512
#endif // #ifdef COMPRESS_OUTPUT


// **************************************************************
std::string Find_File(std::string filename, const int max_number_up)
{
    struct stat statBuf;
    int nb_up = 0;
    while ((stat(filename.c_str(), &statBuf) != 0) and nb_up < max_number_up)
    {
        // File not found, try one folder up
        filename = "../" + filename;
        ++nb_up;
    }

    // File should have been found by now.
    if (stat(filename.c_str(), &statBuf) != 0)
        std_cout << "ERROR: Cannot find file " << filename << ". Mayeb try increasing max_number_up(="<<max_number_up<<")?\n";

    return filename;
}

// **************************************************************
void Create_Folder_If_Does_Not_Exists(const std::string path)
{
    struct stat statBuf;
    if (stat(path.c_str(), &statBuf) < 0)
    {
        std::string cmd = "mkdir -p " + path;
        std_cout
            << "Folder \"" << path << "\" does not exist!\n"
            << "The following command will create it:\n"
            << "    " << cmd << "\n"
            << "If it fails, create it manually.\n";
        const int return_value = system(cmd.c_str());
        if (return_value != 0)
        {
            std_cout << "ERROR: Folder '" << path << "' could not be created correclty!\n" << std::flush;
            abort();
        }
    }
}

// **************************************************************
void Print_Double_in_Binary(double d)
/**
 * Prints binary representation of a double
 * http://www.exploringbinary.com/displaying-the-raw-fields-of-a-floating-point-number/
 */
{
    uint64_t *double_as_int = (uint64_t *) &d;
    const int bit_size = CHAR_BIT*sizeof(uint64_t);

    // Print bits by bits
    for (int b = 0 ; b <= bit_size-1 ; b++)
    {
        if (b == 1)
            std_cout << " ";    // Space after sign field
        if (b == 12)
            std_cout << " ";    // Space after exponent field

        // Get bit, but in reverse order. On Little Endian machines
        // (most of Intel and such), the byte with lower address
        // is the less significant. Since we want to print from
        // the most significant, we iterate from the end.
        if ((*double_as_int >> ((bit_size-1)-b)) & 1)
            std_cout << "1";
        else
            std_cout << "0";
    }
    //std_cout << "\n";
}

// **************************************************************
void Print_Double_in_Binary(float d)
/**
 * Prints binary representation of a double
 * http://www.exploringbinary.com/displaying-the-raw-fields-of-a-floating-point-number/
 */
{
    uint32_t *float_as_int = (uint32_t *) &d;
    const int bit_size = CHAR_BIT*sizeof(uint32_t);

    // Print bits by bits
    for (int b = 0 ; b <= bit_size-1 ; b++)
    {
        if (b == 1)
            std_cout << " ";    // Space after sign field
        if (b == 9)
            std_cout << " ";    // Space after exponent field

        // Get bit, but in reverse order. On Little Endian machines
        // (most of Intel and such), the byte with lower address
        // is the less significant. Since we want to print from
        // the most significant, we iterate from the end.
        if ((*float_as_int >> ((bit_size-1)-b)) & 1)
            std_cout << "1";
        else
            std_cout << "0";
    }
    //std_cout << "\n";
}

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
void IO::Clear()
{
    enable                  = false;
    period                  = 0.0;
    last_saved_time         = 0.0;
    nb_saved                = 0;
    C_fh                    = NULL;
    using_C_fh              = false;
    mode                    = '\0';
    binary                  = false;
    append                  = false;
    force_at_next_iteration = false;
    disable_at_next_iteration = false;
    compressed              = false;
    compressed_fh           = NULL;
    string_to_save          = NULL;
}

// **************************************************************
IO::IO()
{
    Clear();
}

// **************************************************************
IO::IO(const bool _enable)
{
    Clear();
    enable                  = _enable;
}

// **************************************************************
IO::~IO()
{
    this->Close_File();
}

// **************************************************************
void IO::Init(const double _period, const std::string _filename, const bool _binary)
{
    Clear();
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
void IO::Enable()
{
    enable = true;
}

// **************************************************************
void IO::Disable()
{
    enable = false;
}

// **************************************************************
bool IO::Open_File(const std::string full_mode, const bool quiet,
                   const bool _using_C_fh, const bool check_if_file_exists)
{
    assert(enable);
    compressed = false;

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
    else if (full_mode.find("a") != std::string::npos)
    {
        append = true;
        mode = 'a';
        file_openmode = std::fstream::out | std::fstream::app;
    }
    else
    {
        std_cout << "ERROR: Unknown mode '" << full_mode << "'. Exiting.\n" << std::flush;
        abort();
    }

    if (full_mode.find("z") != std::string::npos)
    {
#ifdef COMPRESS_OUTPUT
        compressed = true;
        filename += ".gz";
#else // #ifdef COMPRESS_OUTPUT
        // If library not compiled with compression, disable flag.
        compressed = false;
        std_cout << "Compression for file '" << filename << "' disabled. Please compile io.git with -DCOMPRESS_OUTPUT if you want compression.\n";
#endif // #ifdef COMPRESS_OUTPUT
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

        if (Is_Compressed())
        {
#ifdef COMPRESS_OUTPUT
            gzFile tmp_file = gzopen(filename.c_str(), "wb");
            gzbuffer(tmp_file, DEFAULT_BUFFER_SIZE);
            compressed_fh = (void *) tmp_file;
            retry = false;
#endif // #ifdef COMPRESS_OUTPUT
        }
        else if (using_C_fh)
        {
            C_fh = fopen(filename.c_str(), full_mode.c_str());

            // Verify that the file is opened.
            if (C_fh == NULL)
            {
                if (check_if_file_exists)
                {
                    std::cerr << "Could not open file \"" << filename << "\" for '" << full_mode << "'...\n";
                    std::string answer = Pause("Retry? [y/N]");
                    if (! (answer == "y" || answer == "yes"))
                    {
                        std_cout << std::flush;
                        abort();
                    }
                } else
                    return false;
            } else {
                retry = false;
            }
        } else {
            fh.open(filename.c_str(), file_openmode);

            // Verify that the file is opened.
            if (!fh.is_open())
            {
                if (check_if_file_exists)
                {
                    std::cerr << "Could not open file \"" << filename << "\" for '" << full_mode << "'...\n";
                    std::string answer = Pause("Retry? [y/N]");
                    if (! (answer == "y" || answer == "yes"))
                    {
                        std_cout << std::flush;
                        abort();
                    }
                } else
                    return false;
            } else {
                retry = false;
            }
        }
    }

    return true;
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

    if (disable_at_next_iteration)
    {
        disable_at_next_iteration = false;
        return false;
    }

    if (force_at_next_iteration)
    {
        std_cout << "Output forced for " << filename << "...\n";
        force_at_next_iteration = false;
        nb_saved++;
        return true;
    }

    // Initial save (time == 0.0)
    // FLT_MIN ~ 10^-38
    if (time < 1.0e-36)
    {
        nb_saved++;
        return true;
    }

    // Period of -1 means always save
    if (-1.001 < period && period < -0.999)
    {
        nb_saved++;
        return true;
    }

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
        {
            nb_saved++;
            last_saved_time = time_previous_period;
        }
        return true;
    }

    return false;
}

// **************************************************************
void IO::Close_File()
{
    if (Is_Compressed())
    {
#ifdef COMPRESS_OUTPUT
        if (compressed_fh != NULL)
            gzclose((gzFile *) compressed_fh);
        compressed_fh = NULL;
#else // #ifdef COMPRESS_OUTPUT
        std_cout << "Can't be here!!! (" << __FILE__ << " line " << __LINE__ << "). Aborting.\n" << std::flush;
        abort();
#endif // #ifdef COMPRESS_OUTPUT
    }
    else if (using_C_fh)
    {
        if (C_fh != NULL)
            fclose(C_fh);
        C_fh = NULL;
    } else {
        if (fh.is_open())
            fh.close();
    }

    if (string_to_save != NULL)
        delete[] string_to_save;
    string_to_save = NULL;
}

// **************************************************************
void IO::Write(const char *p, size_t size)
{
    assert(Is_Open());
    assert(Is_Enable());

    if (Is_Compressed())
    {
#ifdef COMPRESS_OUTPUT
        const int error_code = gzwrite(compressed_fh, p, (unsigned int)size);
        assert(error_code != 0);
#else
        std_cout << "Can't be here!!! (" << __FILE__ << " line " << __LINE__ << "). Aborting.\n" << std::flush;
        abort();
#endif // #ifdef COMPRESS_OUTPUT
    }
    else if (using_C_fh)
    {
        fwrite(p, size, 1, C_fh);
    }
    else
    {
        fh.write(p, size);
    }
}

// **************************************************************
void IO::WriteString(const std::string &format, ...)
{
    assert(Is_Open());

    va_list args;
    va_start(args, format);

    if (Is_Compressed() or !using_C_fh)
    {
        if (string_to_save == NULL)
        {
            //std_cout << "Allocating space for string_to_save...\n";
            string_to_save = new char[1024];
        }

        int result = vsprintf(string_to_save, format.c_str(), args);
        if (result < 0)
        {
            std_cout << "Couldn't call vsprintf! Aborting.\n" << std::flush;
            abort();
        }
        va_end(args);

        if (Is_Compressed())
        {
#ifdef COMPRESS_OUTPUT
        const int error_code = gzwrite(compressed_fh, string_to_save, (unsigned int)strlen(string_to_save));
        assert(error_code != 0);
#else
        std_cout << "Can't be here!!! (" << __FILE__ << " line " << __LINE__ << "). Aborting.\n" << std::flush;
        abort();
#endif // #ifdef COMPRESS_OUTPUT
        }
        else
        {
            fh << string_to_save;
        }
    }
    else
    {
        vfprintf(C_fh, format.c_str(), args);
    }
}

// **************************************************************
void IO::Flush()
{
    if (Is_Compressed())
    {
#ifdef COMPRESS_OUTPUT
        gzflush(compressed_fh, Z_FINISH);
#else // #ifdef COMPRESS_OUTPUT
        std_cout << "Can't be here!!! (" << __FILE__ << " line " << __LINE__ << "). Aborting.\n" << std::flush;
        abort();
#endif // #ifdef COMPRESS_OUTPUT
    }
    else if (using_C_fh)
    {
        fflush(C_fh);
    } else {
        if (fh.is_open() && (mode == 'w' || mode == 'a'))
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
ReadXML::ReadXML(const std::string _name, const std::string _filename)
{
    this->Open(_name, _filename);
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
        std_cout << "Error reading XML file '" << filename << "'\n" << std::flush;
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
TiXmlNode * ReadXML::Get_SubNode(TiXmlNode *subnode, const std::string &celements)
{
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

// **************************************************************
TiXmlNode * ReadXML::Get_SubNode_Matching_Attribute(TiXmlNode *root,
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
            DEBUGP("ERROR: ReadXML::Get_SubNode_Matching_Attribute() failed!\n");
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
        DEBUGP("ERROR: ReadXML::Get_SubNode_Matching_Attribute() failed!\n");
        std_cout << "elements = " << elements << "\n"
                 << "attribute = " << attribute << "\n"
                 << "attribute_value = " << attribute_value << "\n"
                 << std::flush;
        abort();
    }
    return subnode;
}

// **************************************************************
TiXmlNode * ReadXML::Get_SubNode_Matching_Attribute(TiXmlNode *root,
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
            DEBUGP("ERROR: ReadXML::Get_SubNode_Matching_Attribute() (2) failed\n");
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
        DEBUGP("ERROR: ReadXML::Get_SubNode_Matching_Attribute() failed!\n");
        std_cout << "elements = " << elements << "\n"
                 << "attribute1 = " << attribute1 << "\n"
                 << "attribute1_value = " << attribute1_value << "\n"
                 << "attribute2 = " << attribute2 << "\n"
                 << "attribute2_value = " << attribute2_value << "\n"
                 << std::flush;
        abort();
    }
    return subnode;
}

// **************************************************************
TiXmlNode * ReadXML::Get_SubNode_Matching_Attribute(TiXmlNode *root,
                                                    const std::string &elements,
                                                    const std::string &attribute,
                                                    const std::string &attribute_value)
{
    // Get the subnode pointed by "elements"
    TiXmlNode *subnode = Get_SubNode(root, elements);

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
        DEBUGP("ERROR: ReadXML::Get_SubNode_Matching_Attribute() failed!\n");
        std_cout << "elements = " << elements << "\n"
                 << "attribute = " << attribute << "\n"
                 << "attribute_value = " << attribute_value << "\n"
                 << std::flush;
        abort();
    }
    return subnode;
}

// **************************************************************
std::string ReadXML::Get_String(const std::string element, TiXmlNode *subnode)
{
    if (subnode == NULL)
        subnode = Get_SubNode(element);

    std::string buff;

    if (subnode != NULL)
    {
        if (subnode->FirstChild() != NULL)
            buff = std::string(subnode->FirstChild()->Value());
        else
        {
            std_cout << "ReadXML::Get_String() failed!\n";
            std_cout << "Element \"" << element << "\" is empty!\n" << std::flush;
            abort();
        }
    }
    else
    {
        std_cout << "ReadXML::Get_String() failed!\n";
        std_cout << "Element \"" << element << "\" not found!\n" << std::flush;
        abort();
    }

    return buff;
}

// **************************************************************
bool ReadXML::Get_Bool(const std::string element, TiXmlNode *subnode)
{
    std::string temp = Get_String(element, subnode);

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
bool ReadXML::Get_Enable(const std::string element, TiXmlNode *subnode)
{
    std::string temp = Get_Attribute_String(element, "enable", subnode);

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
double ReadXML::Get_Double(const std::string element, TiXmlNode *subnode)
{
    return double(atof(Get_String(element, subnode).c_str()));
}

// **************************************************************
int ReadXML::Get_Int(const std::string element, TiXmlNode *subnode)
{
    return atoi(Get_String(element, subnode).c_str());
}

// **************************************************************
unsigned int ReadXML::Get_UInt(const std::string element, TiXmlNode *subnode)
{
    // By reading a 64-bit int, we make sure it does not overflow
    // the unsigned value. Then we cast and return.
    int64_t tmp = Get_Int64(element, subnode);
    assert(tmp < int64_t(std::numeric_limits<unsigned int>::max()));
    return (unsigned int)(tmp);
}

// **************************************************************
long int ReadXML::Get_LInt(const std::string element, TiXmlNode *subnode)
{
    return atol(Get_String(element, subnode).c_str());
}

// **************************************************************
int64_t ReadXML::Get_Int64(const std::string element, TiXmlNode *subnode)
{
    return int64_t(atof(Get_String(element, subnode).c_str()));
}

// **************************************************************
uint64_t ReadXML::Get_UInt64(const std::string element, TiXmlNode *subnode)
{
    const double tmp_double = double(atof(Get_String(element, subnode).c_str()));
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
std::string ReadXML::Get_Attribute_String(const std::string element, const std::string attribute, TiXmlNode *subnode)
{
    if (element == "" && subnode == NULL)
    {
        std_cout << "ReadXML::Get_Attribute_String() failed!\n";
        std_cout << "When providing an empty element, a subnode pointer need to be passed!\n" << std::flush;
        abort();
    }

    if (subnode == NULL)
        subnode = Get_SubNode(element);

    if (subnode == NULL)
    {
        std_cout << "ReadXML::Get_Attribute_String() failed!\n";
        std_cout << "Element \"" << element << "\" not found!\n" << std::flush;
        abort();
    }

    const char *found_attribute = subnode->ToElement()->Attribute(attribute.c_str());
    if (found_attribute == NULL)
    {
        std_cout << "ERROR: Attribute '" << attribute << "' in '" << element << "' not found!\n" << std::flush;
        abort();
    }

    return std::string(found_attribute);
}

// **************************************************************
bool ReadXML::Get_Attribute_Bool(const std::string element, const std::string attribute, TiXmlNode *subnode)
{
    std::string temp = Get_Attribute_String(element, attribute, subnode);

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
double ReadXML::Get_Attribute_Double(const std::string element, const std::string attribute, TiXmlNode *subnode)
{
    return double(atof(Get_Attribute_String(element, attribute, subnode).c_str()));
}

// **************************************************************
int ReadXML::Get_Attribute_Int(const std::string element, const std::string attribute, TiXmlNode *subnode)
{
    return atoi(Get_Attribute_String(element, attribute, subnode).c_str());
}

// **************************************************************
long int ReadXML::Get_Attribute_LInt(const std::string element, const std::string attribute, TiXmlNode *subnode)
{
    return atol(Get_Attribute_String(element, attribute, subnode).c_str());
}

// **************************************************************
int64_t ReadXML::Get_Attribute_Int64(const std::string element, const std::string attribute, TiXmlNode *subnode)
{
    return int64_t(atof(Get_Attribute_String(element, attribute, subnode).c_str()));
}

// **************************************************************
uint64_t ReadXML::Get_Attribute_UInt64(const std::string element, const std::string attribute, TiXmlNode *subnode)
{
    const double tmp_double = double(atof(Get_Attribute_String(element, attribute, subnode).c_str()));
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
            << "Aborting\n" << std::flush;
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
    else if (unit == "zs")  unit_factor = libtinyxml::zs_to_s;
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
            << "    au\n    zs\n    as\n    fs\n    ps\n    ns\n    mus\n    ms\n    s\n"
            << "Aborting\n" << std::flush;
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

// **************************************************************
double ReadXML::Factor_Unit_To_Intensity_AU(const std::string element)
{
    const std::string unit = Get_Unit(element);
    double unit_factor = 0.0;

    if      (unit == "W/cm^2")  unit_factor = ((libtinyxml::si_to_au_energy/libtinyxml::si_to_au_time) / (pow(libtinyxml::cm_to_m*libtinyxml::m_to_bohr, 2)));
    else if (unit == "W.cm-^2") unit_factor = ((libtinyxml::si_to_au_energy/libtinyxml::si_to_au_time) / (pow(libtinyxml::cm_to_m*libtinyxml::m_to_bohr, 2)));
    else if (unit == "W/m^2")   unit_factor = ((libtinyxml::si_to_au_energy/libtinyxml::si_to_au_time) / (pow(                    libtinyxml::m_to_bohr, 2)));
    else if (unit == "W.m-^2")  unit_factor = ((libtinyxml::si_to_au_energy/libtinyxml::si_to_au_time) / (pow(                    libtinyxml::m_to_bohr, 2)));
    else
    {
        std_cout
            << "Error in reading XML input file:\n"
            << "Unit of " << element << " needs to be either:\n"
            << "    W/cm^2\n    W.cm-^2\n    W/m^2\n    W.m-^2\n"
            << "Aborting\n" << std::flush;
        abort();
    }

    return unit_factor;
}

// ********** End of file ***************************************
