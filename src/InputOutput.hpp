#ifndef INC_CLASSES_IO_hpp
#define INC_CLASSES_IO_hpp

#include <string>
#include <vector>
#include <fstream>
#include <cstdarg>

#ifdef __PGI
#include <boost/cstdint.hpp>
using namespace boost;
#else
#include <stdint.h> // (u)int64_t
// #include <cstdint> // requires C++0x
#endif // #ifdef __PGI

#include "tinyxml.hpp"


namespace inputoutput
{
    // See Git_Info.cpp (generated dynamically from Git_Info.cpp_template & Makefile.rules)
    void Log_Git_Info(std::string basename = "");
}

void Print_Double_in_Binary(double d);
void Print_Double_in_Binary(float d);

std::string Find_File(std::string filename, const int max_number_up = 2);
void Find_File(char *filename, const int max_number_up, const int max_string_size);
void Create_Folder_If_Does_Not_Exists(const std::string path);


FILE * Open_File(const std::string &filename, const std::string &mode, const bool quiet = false);

class IO
{
    private:
        bool enable;            // Is output enabled?
        bool compressed;        // Is output compressed?

        double period;          // Period of saving
        double last_saved_time; // Last time which forced a save
        uint64_t nb_saved;      // Number of times saved

        std::fstream fh;        // C++ File handle
        FILE *C_fh;             // Alternate C file handle
        bool using_C_fh;
        void *compressed_fh;
        char *string_to_save;

        std::string filename;   // File name
        char mode;              // Read or write?
        bool binary;            // Binary file?
        bool append;            // Append to file?

        // Do we want to force IO at next iteration?
        bool force_at_next_iteration;
        // Do we want to disable IO at next iteration?
        bool disable_at_next_iteration;

    public:

        void Clear();
        IO();
        IO(const bool _enable);
        ~IO();
        void Init(const double _period, const std::string _filename, const bool _binary = false);
        void Set_Period(const double _period);
        void Set_Filename(const std::string _full_filename);
        void Set_Filename(const std::string _path, const std::string _filename);
        void Disable();
        void Enable();
        bool Open_File(const std::string mode, const bool quiet = false,
                       const bool _using_C_fh = false,
                       const bool check_if_file_exists = true);
        void Close_File();
        uint64_t Get_Nb_Saved() { return nb_saved; }

        void Write(const char *p, size_t size);
        void WriteString(const std::string &format, ...);

        inline bool             Is_Enable()                 { return enable;    }
        inline bool             Is_Compressed()             { return compressed;    }
        inline bool             Is_Open()                   { return (compressed_fh != NULL ? true : (using_C_fh ? ((C_fh != NULL) ? true : false ) : (fh.is_open() ? true : false))); }
        inline std::fstream&    Fh()                        { return fh;        }
        inline FILE *           C_Fh()                      { return C_fh;      }
        inline std::string      Get_Filename()              { return filename;  }
        inline double           Get_Period()                { return period;    }
        inline void             Force_At_Next_Iteration()   { force_at_next_iteration = true; }
        inline void             Disable_At_Next_Iteration() { disable_at_next_iteration = true; }
        inline bool             Is_Forced_At_Next_Iteration()   { return (force_at_next_iteration ? true : false);   }
        inline bool             Is_Disabled_At_Next_Iteration() { return (disable_at_next_iteration ? true : false); }

        void Flush();
        void Format(const int width, const int nb_after_dot, const char type, const char justify='r', const char fill=' ');

        bool Is_Output_Permitted(const double time, const bool dont_set_previous_period = false);

        void Print();
};

class ReadXML
{
    private:
        std::string     name;
        std::string     filename;
        TiXmlNode       *RootNode;  // root
        TiXmlDocument   Document;   // input
        //TiXmlHandle     Handle;     // inputHandle

        void        Split_String(const std::string &to_split, const char delimiter, std::vector<std::string> &split_strings);

    public:
        ReadXML(const std::string name, const std::string _filename);
        void Open(const std::string name, const std::string _filename);
        void Dump(const std::string _filename);

        // Get elements
        std::string Get_String( const std::string element, TiXmlNode *subnode = NULL);
        bool        Get_Bool(   const std::string element, TiXmlNode *subnode = NULL);
        double      Get_Double( const std::string element, TiXmlNode *subnode = NULL);
        int         Get_Int(    const std::string element, TiXmlNode *subnode = NULL);
        unsigned int Get_UInt(  const std::string element, TiXmlNode *subnode = NULL);
        long int    Get_LInt(   const std::string element, TiXmlNode *subnode = NULL);
        int64_t     Get_Int64(  const std::string element, TiXmlNode *subnode = NULL);
        uint64_t    Get_UInt64( const std::string element, TiXmlNode *subnode = NULL);
        bool        Get_Enable( const std::string element, TiXmlNode *subnode = NULL);

        double      Get_Double_Time_Second( const std::string element);
        double      Get_Double_Time_AU(     const std::string element);
        double      Get_Double_Length_Meter(const std::string element);
        double      Get_Double_Length_AU(   const std::string element);

        void        Set_String(const std::string element, const std::string value);
        void        Set_Attribute_String(const std::string element, const std::string attribute, const std::string value);

        TiXmlNode * Get_SubNode(const std::string elements, TiXmlNode *subnode = NULL);
        TiXmlNode * Get_SubNode_Matching_Attribute(TiXmlNode *root,
                                                   const std::string &elements,
                                                   const std::string &attribute,
                                                   const int attribute_value);
        TiXmlNode * Get_SubNode_Matching_Attribute(TiXmlNode *root,
                                                   const std::string &elements,
                                                   const std::string &attribute1,
                                                   const int attribute1_value,
                                                   const std::string &attribute2,
                                                   const int attribute2_value);
        TiXmlNode * Get_SubNode_Matching_Attribute(TiXmlNode *root,
                                                   const std::string &elements,
                                                   const std::string &attribute,
                                                   const std::string &attribute_value);

        template <class T>
        void        Set(const std::string element, const T value)
        {
            // Get a string representation of the value
            std::stringstream string_stream;
            string_stream << value;
            // Set the value of the node to be the used seed
            const std::string value_string = string_stream.str();

            Set_String(element, value_string);
        }

        template <class T>
        void        Set_Attribute(const std::string element, const std::string attribute, const T value)
        {
            // Get a string representation of the value
            std::stringstream string_stream;
            string_stream << value;
            // Set the value of the node to be the used seed
            const std::string value_string = string_stream.str();

            Set_Attribute_String(element, attribute, value_string);
        }

        // Get attributes
        std::string Get_Attribute_String(   const std::string element, const std::string attribute, TiXmlNode *subnode = NULL);
        bool        Get_Attribute_Bool(     const std::string element, const std::string attribute, TiXmlNode *subnode = NULL);
        double      Get_Attribute_Double(   const std::string element, const std::string attribute, TiXmlNode *subnode = NULL);
        int         Get_Attribute_Int(      const std::string element, const std::string attribute, TiXmlNode *subnode = NULL);
        long int    Get_Attribute_LInt(     const std::string element, const std::string attribute, TiXmlNode *subnode = NULL);
        int64_t     Get_Attribute_Int64(    const std::string element, const std::string attribute, TiXmlNode *subnode = NULL);
        uint64_t    Get_Attribute_UInt64(   const std::string element, const std::string attribute, TiXmlNode *subnode = NULL);

        std::string Get_Unit(const std::string element);
        void        Verify_Attribute(const std::string element, const std::string attribute, const std::string value);
        void        Verify_Unit(const std::string element, const std::string unit);
        bool        Is_Enable(const std::string element);

        int         Count_Elements(TiXmlNode *root, const std::string &element);

        double      Factor_Unit_To_J(           const std::string element);
        double      Factor_Unit_To_Metre(       const std::string element);
        double      Factor_Unit_To_Second(      const std::string element);
        double      Factor_Unit_To_Length_AU(   const std::string element);
        double      Factor_Unit_To_Time_AU(     const std::string element);
        double      Factor_Unit_To_Intensity_AU(const std::string element);
};

#endif // INC_CLASSES_IO_hpp

// ********** End of file ***************************************
