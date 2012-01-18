#ifndef INC_CLASSES_NETCDF_hpp
#define INC_CLASSES_NETCDF_hpp

#ifdef NETCDF

#include <netcdf.h>
#include <stdint.h> // (u)int64_t

#include <string>
#include <vector>
#include <map>
#include <set>


#define NC_FDOUBLE -1000

// http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c/NetCDF_002d4-Atomic-Types.html#NetCDF_002d4-Atomic-Types
// Type                C #define   Bits
// byte                NC_BYTE      8
// unsigned byte       NC_UBYTE^    8
// char                NC_CHAR      8
// short               NC_SHORT     16
// unsigned short      NC_USHORT^   16
// int                 NC_INT       32
// unsigned int        NC_UINT^     32
// unsigned long long  NC_UINT64^   64
// long long           NC_INT64^    64
// float               NC_FLOAT     32
// double              NC_DOUBLE    64
// char **             NC_STRING^  string length + 1

#define netcdf_type_nb          14

#define netcdf_type_bool         0
#define netcdf_type_byte         1
#define netcdf_type_ubyte        2
#define netcdf_type_char         3
#define netcdf_type_short        4
#define netcdf_type_ushort       5
#define netcdf_type_int          6
#define netcdf_type_uint         7
#define netcdf_type_uint64       8
#define netcdf_type_int64        9
#define netcdf_type_float       10
#define netcdf_type_double      11
#define netcdf_type_fdouble     12
#define netcdf_type_string      13

const char netcdf_types_string[netcdf_type_nb][11] = {
    "bool\0     ",
    "byte\0     ",
    "ubyte\0    ",
    "char\0     ",
    "short\0    ",
    "ushort\0   ",
    "int\0      ",
    "uint\0     ",
    "uint64\0   ",
    "int64\0    ",
    "float\0    ",
    "double\0   ",
    "fdouble\0  ",
    "string\0   "
};

const nc_type netcdf_types[netcdf_type_nb] = {
    NC_BYTE, // bool
    NC_BYTE,
    NC_UBYTE,
    NC_CHAR,
    NC_SHORT,
    NC_USHORT,
    NC_INT,
    NC_UINT,
    NC_UINT64,
    NC_INT64,
    NC_FLOAT,
    NC_DOUBLE,
    NC_FDOUBLE,
    NC_STRING
};


class NetCDF_Dimensions
{
public:
    std::vector<int>         Ns;    // Have to be contiguous
    std::vector<int>         ids;   // Have to be contiguous
    std::vector<std::string> names;
    void Add(std::string _name, int _value);
    void Print() const;
    size_t size() const;
};

class NetCDF_Variable
{
private:
    int ncid;                               // Associated NetCDF file id
    int varid;                              // Variable id
    const void *pointer;                    // Pointer to (read-only) memory
    std::string name;                       // Name
    bool is_committed;                      // Before writting, variable must be committed.
    bool is_compressed;
    NetCDF_Dimensions dimensions;
    void call_netcdf_and_test(const int netcdf_retval, const std::string note = "");

public:
    int type_index;                         // netcdf_type_*
    nc_type netcdf_type;                 // NetCDF variable type, value from nc_types const array.

    NetCDF_Variable();
    template <class T>
    void Init(const int &_ncid, const std::string &_name,
              const T *const _pointer,
              const int _type_index,
              const bool compress = true);
    void Set_Dimension(const NetCDF_Dimensions &user_dims,
                       const std::map<std::string, int> &commited_dimensions_ids);
    void Units(const std::string units);
    void Commit();
    void Write();
    void Print() const;
};

class NetCDF_Out
{
private:
    std::string filename;
    int ncid;
    std::map<std::string, NetCDF_Variable> variables;
    bool is_netcdf4;
    bool is_opened;
    bool is_committed;
    bool is_written;
    std::map<std::string, int> dimensions_val;
    std::map<std::string, int> dimensions_ids;

    std::set<uint64_t> previous_variables_ptr;
    void call_netcdf_and_test(const int netcdf_retval, const std::string note = "");

public:

    NetCDF_Out();
    NetCDF_Out(const std::string _path, const std::string _filename, const bool netcdf4 = true);
    NetCDF_Out(std::string _filename, const bool netcdf4 = true);
    ~NetCDF_Out();
    void Open(const std::string _path, const std::string _filename, const bool netcdf4 = true);

    template <class T>
    void Add_Variable(const std::string name, const int type_index,
                      const T *const pointer,
                      NetCDF_Dimensions dims,
                      const std::string units = "");
    template <class T>
    void Add_Variable_Scalar(const std::string name, const int type_index,
                             const T *const pointer,
                             const std::string units = "");
    template <class T>
    void Add_Variable_1D(const std::string name, const int type_index,
                         const T *const pointer, const int N,
                         const std::string dim_name,
                         const std::string units = "");
    void Add_Variable_2D(const std::string name, const int type_index,
                         const float *const pointer,
                         const int N, const int M,
                         const std::string dim_name_N, const std::string dim_name_M,
                         const std::string units = "");
    void Add_Variable(const std::string name,
                      const std::string string_to_save);
    void Commit();
    void Write();
    void Close();
    void Print() const;
};

class NetCDF_In
{
private:
    std::string filename;
    int ncid;
    bool is_opened;
    void call_netcdf_and_test(const int netcdf_retval, const std::string note = "");
public:

    NetCDF_In();
    NetCDF_In(const std::string _filename);
    ~NetCDF_In();
    void Open(const std::string _filename);
    void Read(const std::string variable_name, void * const pointer);
    void Read(const std::string variable_name, std::string &content);
    void Close();

};

#endif // #ifdef NETCDF

#endif // INC_CLASSES_NETCDF_hpp

// ********** End of file ***************************************
