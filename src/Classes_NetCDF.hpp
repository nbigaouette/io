#ifndef INC_CLASSES_NETCDF_hpp
#define INC_CLASSES_NETCDF_hpp

#ifdef NETCDF

#include <netcdf.h>

#include <string>
#include <vector>
#include <map>


#define NC_FDOUBLE -1000

#define netcdf_type_nb      5

#define netcdf_type_int     0
#define netcdf_type_fdouble 1
#define netcdf_type_float   2
#define netcdf_type_double  3
#define netcdf_type_bool    4

const char netcdf_types_string[netcdf_type_nb][8] = {
    "int\0   ",
    "fdouble",
    "float\0 ",
    "double\0",
    "bool\0  "
};

// NetCDF v3.6 defines "nc_type" as an enum, which screws everything.
// NetCDF 4 does the right thing of using a typdef.
// So detect which one the code is compiled with.
#ifdef NC_NETCDF4
#define NetCDF_version4
typedef nc_type my_nc_type;
#else // #ifndef NC_NETCDF4
typedef int my_nc_type;
#endif // #ifndef nc_type

const my_nc_type netcdf_types[netcdf_type_nb] = {
    NC_INT,
    NC_FDOUBLE,
    NC_FLOAT,
    NC_DOUBLE,
    NC_BYTE // bool
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

public:
    int type_index;                         // netcdf_type_*
    my_nc_type netcdf_type;                 // NetCDF variable type, value from nc_types const array.

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
public:

    NetCDF_In();
    NetCDF_In(const std::string _filename);
    ~NetCDF_In();
    void Open(const std::string _filename);
    void Read(const std::string variable_name, void * const pointer);
    void Close();

};

#endif // #ifdef NETCDF

#endif // INC_CLASSES_NETCDF_hpp

// ********** End of file ***************************************
