
#ifdef NETCDF

#include <StdCout.hpp>

#include "Classes_NetCDF.hpp"
#include "InputOutput.hpp"

#define ERR(e) {std_cout << "Error: " << nc_strerror(e) << " ("<<e<<")\n" << std::flush; abort();}

// Compressiong options
#ifdef NETCDF4_COMPRESSED
const int C_shuffle = NC_SHUFFLE;
const int C_deflate = 1;
const int C_deflate_level = 9;
#endif // #ifdef NETCDF4_COMPRESSED

// const bool verbose = true;
const bool verbose = false;

// **************************************************************
void call_netcdf_and_test(const int netcdf_retval)
{
    if (netcdf_retval)
        ERR(netcdf_retval);
}

// **************************************************************
template <class T>
int Find_In_Vector(std::vector<T> vector_to_search, const T &value)
{
    for (size_t i = 0 ; i < vector_to_search.size() ; i++)
    {
        if (vector_to_search[i] == value)
            return i;
    }
    return -1;
}

// **************************************************************
void NetCDF_Dimensions::Add(std::string _name, int _value)
{
    names.push_back(_name);
    Ns.push_back(_value);
    ids.push_back(-1);
}

// **************************************************************
void NetCDF_Dimensions::Print() const
{
    assert(Ns.size() != 0);

    std_cout << "        NetCDF_Dimensions::Print():\n";
    for (size_t i = 0 ; i < Ns.size() ; i++)
    {
        std_cout
            << "            "
            << "   Ns["<<i<<"] = " << Ns[i]
            << "   ids["<<i<<"] = " << ids[i]
            << "   names["<<i<<"] = " << names[i]
            << "\n";
    }
}

// **************************************************************
size_t NetCDF_Dimensions::size() const
{
    assert(Ns.size() == ids.size());
    assert(ids.size() == names.size());
    return Ns.size();
}

// **************************************************************
NetCDF_Variable::NetCDF_Variable()
{
    ncid            = -1;
    name            = "";
    pointer         = NULL;
    type_index      = -1;
    netcdf_type     = -1;
    is_committed    = false;
    is_compressed   = false;
}

// **************************************************************
template <class T>
void NetCDF_Variable::Init(const int &_ncid, const std::string &_name,
                           const T *const _pointer,
                           const int _type_index, const bool compress)
{
    ncid            = _ncid;
    name            = _name;
    pointer         = _pointer;
    type_index      = _type_index;
    netcdf_type     = netcdf_types[type_index];
    is_committed    = false;
#ifdef NETCDF4_COMPRESSED
    is_compressed   = compress;
#else
    is_compressed   = false;
#endif
}

// **************************************************************
void NetCDF_Variable::Set_Dimension(const NetCDF_Dimensions &user_dims,
                                    const std::map<std::string, int> &commited_dimensions_ids)
{
    if (verbose)
        std_cout << "    NetCDF_Variable::Set_Dimension:\n";
    // Store in the object "dimensions" the dimension combination for a variable
    for (size_t i = 0 ; i < user_dims.names.size() ; i ++)
    {
        // Store temporary references
        const std::string &dim_name = user_dims.names[i];
        const int &dim_N            = user_dims.Ns[i];

        if (verbose)
            std_cout << "        dim_name = " << dim_name << "  dim_N = " << dim_N << std::flush;

        // Find where the commited dimension id is located
        const std::map<std::string, int>::const_iterator it_committed = commited_dimensions_ids.find(dim_name);
        assert(it_committed != commited_dimensions_ids.end());

        const int &dim_id = it_committed->second;

        if (verbose)
            std_cout << "  dim_id = " << dim_id << "\n";

        // Now store it
        dimensions.Ns.push_back(dim_N);
        dimensions.names.push_back(dim_name);
        dimensions.ids.push_back(dim_id);
    }
}

// **************************************************************
void NetCDF_Variable::Units(const std::string units)
{
    call_netcdf_and_test(nc_put_att_text(ncid, varid, "units", units.size(), units.c_str()));
}

// **************************************************************
void NetCDF_Variable::Commit()
{
    if (verbose)
    {
        std_cout << "Commiting variable '" << name.c_str() << "' in file id '" << ncid << "' of type '" << type_index << "' ("<<netcdf_types_string[type_index]<<"), nb of dimension(s) '" << int(dimensions.ids.size()) << "'  (&(dimensions.ids[0]) = " << &(dimensions.ids[0]) << ")\n";
        for (int i = 0 ; i < int(dimensions.ids.size()) ; i++)
        {
            std_cout << "    Dimension: i=" << i << "  name = " << dimensions.names[i] << "  N = " << dimensions.Ns[i] << "  id = " << (&(dimensions.ids[0]))[i] << "\n";
        }
    }

    int *const tmp_ids = (int *) calloc(dimensions.ids.size(), sizeof(int));
    for (int i = 0 ; i < int(dimensions.ids.size()) ; i++)
    {
        tmp_ids[i] = dimensions.ids[i];
    }

    call_netcdf_and_test(
        nc_def_var(
            ncid,                       // File id
            name.c_str(),               // Variable's name
            (nc_type) netcdf_type,      // Variable's type
            int(dimensions.ids.size()), // Number of dimension
            &(dimensions.ids[0]),       // Array (contiguous) of dimension ids
            &varid                      // Variable id (function's output)
        )
    );

    if (verbose)
        std_cout << "  Variable committed. varid = '" << varid << "'\n";

    free(tmp_ids);

#ifdef NETCDF4_COMPRESSED
    if (is_compressed)
    {
        // Compression cn reduce a file size by a factor of 15!
        //call_netcdf_and_test(nc_def_var_chunking(ncid, varid, 0, &chunks[0]));
        call_netcdf_and_test(nc_def_var_deflate(ncid, varid, C_shuffle, C_deflate, C_deflate_level));
    }
#endif // #ifdef NETCDF4_COMPRESSED

    is_committed = true;
}

// **************************************************************
void NetCDF_Variable::Write()
{
    if (not is_committed)
        Commit();

#ifdef NC_NETCDF4
    call_netcdf_and_test(nc_put_var(ncid, varid, pointer));
#else
    // Stupid NetCDF v3.6 does not know about nc_put_var()
    if      (type_index == netcdf_type_bool)
        call_netcdf_and_test( nc_put_var_schar( ncid, varid,    (const signed char *)pointer) );
    else if (type_index == netcdf_type_byte)
        call_netcdf_and_test( nc_put_var_schar( ncid, varid,    (const signed char *)pointer) );
    else if (type_index == netcdf_type_ubyte)
        call_netcdf_and_test( nc_put_var_uchar( ncid, varid,    (const unsigned char *)pointer) );
    else if (type_index == netcdf_type_char)
        call_netcdf_and_test( nc_put_var_text(  ncid, varid,    (const char *)pointer) );
    else if (type_index == netcdf_type_short)
        call_netcdf_and_test( nc_put_var_short( ncid, varid,    (const short int *)pointer) );
    else if (type_index == netcdf_type_ushort)
        call_netcdf_and_test( nc_put_var_ushort(ncid, varid,    (const unsigned short int *)pointer) );
    else if (type_index == netcdf_type_int)
        call_netcdf_and_test( nc_put_var_int(   ncid, varid,    (const int *)pointer) );
    else if (type_index == netcdf_type_uint)
        call_netcdf_and_test( nc_put_var_uint(  ncid, varid,    (const unsigned int *)pointer) );
    else if (type_index == netcdf_type_uint64)
    {
        std_cout << "ERROR in NetCDF_Variable::Write(): NetCDF v3 does not support uint64 type.\n" << std::flush;
        abort();
    }
    else if (type_index == netcdf_type_int64)
    {
        std_cout << "ERROR in NetCDF_Variable::Write(): NetCDF v3 does not support int64 type.\n" << std::flush;
        abort();
    }
    else if (type_index == netcdf_type_float)
        call_netcdf_and_test( nc_put_var_float( ncid, varid,    (const float *) pointer) );
    else if (type_index == netcdf_type_double)
        call_netcdf_and_test( nc_put_var_double(ncid, varid,    (const double *)pointer) );
    else if (type_index == netcdf_type_fdouble)
    {
        std_cout << "ERROR in NetCDF_Variable::Write(): The variable's type is still 'fdouble' when NetCDF_Out::Add_Variable() should have changed it!\n" << std::flush;
        abort();
    }

    else if (type_index == netcdf_type_string)
        call_netcdf_and_test( nc_put_var_text(  ncid, varid,    (const char *)pointer) );
    else
    {
        std_cout << "ERROR in NetCDF_Variable::Write(): Variable type not supported! (type_index=="<<type_index<<")\n" << std::flush;
        abort();
    }
#endif // #ifdef NC_NETCDF4
}

// **************************************************************
void NetCDF_Variable::Print() const
{
    std_cout
        << "    NetCDF_Variable::Print():\n"
        << "        ncid:           " << ncid << "\n"
        << "        varid:          " << varid << "\n"
        << "        name:           " << name << "\n"
        << "        pointer:        " << pointer << "\n"
        << "        type_index:     " << type_index << " (" << netcdf_types_string[type_index] << ")\n"
        << "        netcdf_type:    " << netcdf_type << "\n"
        << "        is_committed:   " << (is_committed ? "true " : "false") << "\n"
        << "        is_compressed:  " << (is_compressed ? "true " : "false") << "\n";
    dimensions.Print();
}

// **************************************************************
NetCDF_Out::NetCDF_Out()
{
    is_opened    = false;
    is_committed = false;
    is_written   = false;
}

// **************************************************************
NetCDF_Out::NetCDF_Out(const std::string _path, const std::string _filename, const bool netcdf4)
{
    Open(_path, _filename, netcdf4);
}

// **************************************************************
NetCDF_Out::NetCDF_Out(std::string _filename, const bool netcdf4)
{
    // Extract the path from filename
    std::string path;
    size_t found = _filename.rfind("/");
    if (found != std::string::npos)
    {
        // Character "/" was found. So it's a path: extract it
        path = _filename;
        path.replace(found, _filename.length()-found, "");

        // Remove path from filename ("+1" to remove the trailing "/")
        _filename.replace(_filename.find(path), path.length()+1, "");
    }
    else
    {
        path = ".";
    }

    Open(path, _filename, netcdf4);
}

// **************************************************************
NetCDF_Out::~NetCDF_Out()
{
    Close();
}

// **************************************************************
void NetCDF_Out::Open(const std::string _path, const std::string _filename, const bool netcdf4)
{
    is_opened    = false;
    is_committed = false;
    is_written   = false;

    filename = _path + "/" + _filename;
    is_netcdf4 = netcdf4;

    // Make sure output folder exists
    Create_Folder_If_Does_Not_Exists(_path);

    // Open file
    if (is_netcdf4)
    {
#ifdef NETCDF4_COMPRESSED
        call_netcdf_and_test(nc_create(filename.c_str(), NC_NETCDF4, &ncid));
#else
        call_netcdf_and_test(nc_create(filename.c_str(), NC_64BIT_OFFSET, &ncid));
#endif
    }
    else
    {
        call_netcdf_and_test(nc_create(filename.c_str(), NC_CLOBBER, &ncid));
    }

    is_opened    = true;

    if (verbose)
        std_cout << "File '" << filename << "' opened with id '" << ncid << "'.\n";
}

// **************************************************************
template <class T>
void NetCDF_Out::Add_Variable(const std::string name, const int type_index,
                              const T *const pointer,
                              const NetCDF_Dimensions dims,
                              const std::string units)
{
    assert(is_opened);
    assert(pointer != NULL);

    if (verbose)
        std_cout << "NetCDF_Out::Add_Variable() Adding variable '" << name << "' of type '" << type_index << "' (" << netcdf_types_string[type_index] << ") to  file '" << filename << "'...\n";

    // Create empty variable
    variables[name] = NetCDF_Variable();
    variables[name].Init(ncid, name, pointer, type_index, is_netcdf4);

    // fdouble is not defined here. Codes can define it as "float" or "double". Since the
    // function definition for Add_Variable() is compiled before knowing which one will
    // be used, we need to treat this type differently.
    if (type_index == netcdf_type_fdouble)
    {
        // Set the right type_index
        if (sizeof(T) == sizeof(float))
            variables[name].type_index = netcdf_type_float;
        else if (sizeof(T) == sizeof(double))
            variables[name].type_index = netcdf_type_double;
        else
        {
            std_cout << "ERROR: Wrong template for NetCDF_Variable::NetCDF_Variable()?\n" << std::flush;
            abort();
        }
        // One the type_index is set, update the netcdf_type
        variables[name].netcdf_type = netcdf_types[variables[name].type_index];
    }


    // Commit every dimensions, but only if it's not yet committed
    for (size_t i = 0 ; i < dims.size() ; i++)
    {
        if (verbose)
            std_cout << "    NetCDF_Out::Add_Variable() 0. dimension: name="<<dims.names[i]<<"  N="<<dims.Ns[i]<<"\n";

        // Search for already saved/commited dimensions
        const std::map<std::string, int>::iterator it_search = dimensions_val.find(dims.names[i]);

        // If not found any, create the new dimension
        if (it_search == dimensions_val.end())
        {
            // Store temporary references
            const std::string &dim_name = dims.names[i];
            const int &dim_N            = dims.Ns[i];

            if (verbose)
                std_cout << "    NetCDF_Out::Add_Variable() 1. dimension: name="<<dim_name<<"  N="<<dim_N<<" not found! Adding...\n";

            // Set the stored values
            dimensions_val[dim_name] = dim_N;
            dimensions_ids[dim_name] = -1; // Default value, will be changed next

            // Commit this dimension (set dimensions_ids[dim_name])
            call_netcdf_and_test(
                nc_def_dim(
                    ncid,                       // File id
                    dim_name.c_str(),           // Name of dimension
                    dim_N,                      // Number of elements in that dimension
                    &(dimensions_ids[dim_name]) // Dimension commit id pointer
                )
            );

            if (verbose)
                std_cout << "    NetCDF_Out::Add_Variable() 2. id committed: " << dimensions_ids[dim_name] << "\n";
        }
        else
        {
            if (verbose)
                std_cout << "    NetCDF_Out::Add_Variable() 3. Dimension named '" << dims.names[i] << "' already present. Skipping.\n";
        }
    }

    // Set the variable's dimension
    variables[name].Set_Dimension(dims, dimensions_ids);

    // Commit the variable
    variables[name].Commit();

    if (units != "")
        variables[name].Units(units);
}

// **************************************************************
template <class T>
void NetCDF_Out::Add_Variable_Scalar(const std::string name, const int type_index,
                                     const T *const pointer, const std::string units)
{
    assert(is_opened);

    NetCDF_Dimensions scalar;
    scalar.Add("scalar", 1);
    Add_Variable<T>(name, type_index, pointer, scalar, units);
}

// **************************************************************
template <class T>
void NetCDF_Out::Add_Variable_1D(const std::string name, const int type_index,
                        const T *const pointer, const int N, const std::string dim_name,
                        const std::string units)
{
    assert(is_opened);

    NetCDF_Dimensions tmp_dims;
    tmp_dims.Add(dim_name, N);
    Add_Variable<T>(name, type_index, pointer, tmp_dims, units);
}

// **************************************************************
void NetCDF_Out::Add_Variable(const std::string name,
                              const std::string string_to_save)
{
    assert(is_opened);

    const int N = string_to_save.size();

    std::ostringstream string_length;
    string_length << "string" << N;
    Add_Variable_1D<char>(name, netcdf_type_char, string_to_save.c_str(), N, string_length.str());
}

// **************************************************************
void NetCDF_Out::Commit()
{
    if (verbose and is_opened)
        std_cout << "NetCDF_Out::Commit(this="<<this<<","<<filename<<", is_committed="<<(is_committed?"true ":"false")<<")..." << "\n";

    // Not needed for NetCDF4 (?)
    // End define mode. This tells netCDF we are done defining metadata.
    if (not is_committed)
        call_netcdf_and_test(nc_enddef(ncid));

    is_committed = true;
}

// **************************************************************
void NetCDF_Out::Write()
{
    if (verbose and is_opened)
        std_cout << "NetCDF_Out::Write(this="<<this<<","<<filename<<")..." << "\n";

    Commit();

    for (std::map<std::string, NetCDF_Variable>::iterator it = variables.begin() ; it != variables.end(); it++ )
        it->second.Write();

    is_written = true;
}

// **************************************************************
void NetCDF_Out::Close()
{
    if (verbose)
        std_cout << "NetCDF_Out::Close(this="<<this<<","<<filename<<")..." << "\n";

    if (not is_committed)
        Commit();

    if (not is_written)
        Write();

    /* Close the file. This frees up any internal netCDF resources
     * associated with the file, and flushes any buffers. */
    if (is_opened)
        call_netcdf_and_test(nc_close(ncid));

    is_opened = false;
}

// **************************************************************
void NetCDF_Out::Print() const
{
    std_cout
        << "NetCDF_Out::Print():\n"
        << "    filename:       " << filename << "\n"
        << "    ncid:           " << ncid << "\n"
        << "    is_netcdf4:     " << (is_netcdf4 ? "true " : "false") << "\n"
        << "    is_opened:      " << (is_opened ? "true " : "false") << "\n"
        << "    is_committed:   " << (is_committed ? "true " : "false") << "\n"
        << "    is_written:     " << (is_written ? "true " : "false") << "\n";
    for (std::map<std::string, NetCDF_Variable>::const_iterator it = variables.begin() ; it != variables.end() ; it++ )
    {
        it->second.Print();
    }
    std_cout << "    dimensions_val:\n";
    for (std::map<std::string, int>::const_iterator it = dimensions_val.begin() ; it != dimensions_val.end() ; it++ )
    {
        std_cout
            << "        (name,N,id) = ("
            << it->first << ","
            << it->second << ","
            << dimensions_ids.at(it->first) << ")\n";
    }
}

// **************************************************************
NetCDF_In::NetCDF_In()
{
    is_opened = false;
}

// **************************************************************
NetCDF_In::NetCDF_In(const std::string _filename)
{
    is_opened = false;

    Open(_filename);
}

// **************************************************************
void NetCDF_In::Open(const std::string _filename)
{
    assert(not is_opened);

    filename = _filename;

    call_netcdf_and_test( nc_open(filename.c_str(), NC_NOWRITE, &ncid) );

    is_opened = true;
}

// **************************************************************
NetCDF_In::~NetCDF_In()
{
    Close();
}

// **************************************************************
void NetCDF_In::Read(const std::string variable_name, void * const pointer)
{
    assert(is_opened);
    assert(pointer != NULL);

    int varid;

    // Get the varid of the data variable, based on its name.
    call_netcdf_and_test( nc_inq_varid(ncid, variable_name.c_str(), &varid) );

   // Read the data.
// #ifdef NetCDF_36
//     std_cout << "ERROR in NetCDF_In::Read(): NetCDF v3.6 version of NetCDF_In::Read() not written!\n" << std::flush;
//     abort();
// #else
//     call_netcdf_and_test( nc_get_var(ncid, varid, pointer) );
// #endif // #ifdef NetCDF_36

// #ifdef NC_NETCDF4
#ifdef NetCDF_version4
    call_netcdf_and_test( nc_get_var(ncid, varid, pointer) );
#else
    std_cout << "ERROR in NetCDF_In::Read(): NetCDF v3.6 version of NetCDF_In::Read() not written!\n" << std::flush;
    abort();
#endif // #ifdef NC_NETCDF4
}

// **************************************************************
void NetCDF_In::Read(const std::string variable_name, std::string &content)
{
    assert(is_opened);

    // Read the character array size (its dimension), allocate enough room in the
    // string and then read the content.

    // Get the varid of the data variable, based on its name.
    int varid;
    call_netcdf_and_test( nc_inq_varid(ncid, variable_name.c_str(), &varid) );

#ifdef NetCDF_version4

    // FIXME: Read the string size from the variable's dimension,
    //        See nc_inq_vardimid() and nc_inq_varndims()
    //        For now, just use a temporary that is big enough.
    const int max_string_size = 4096;
    char *cstring = (char *) calloc(max_string_size+1, sizeof(char));
    assert(cstring != NULL);

    call_netcdf_and_test( nc_get_var(ncid, varid, cstring) );
    content = std::string(cstring);
#else
    std_cout << "ERROR in NetCDF_In::Read(): NetCDF v3.6 version of NetCDF_In::Read() not written (std::string version)!\n" << std::flush;
    abort();
#endif // #ifdef NC_NETCDF4
}

// **************************************************************
void NetCDF_In::Close()
{
    if (is_opened)
        call_netcdf_and_test(nc_close(ncid));
    is_opened = false;
}


// **************************************************************
// Templates specializations

// NetCDF_Out::Add_Variable()
template void NetCDF_Out::Add_Variable<bool>(               const std::string name, const int type_index,
                                                            const bool *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
// FIXME: Specialize a template for NC_BYTE
// FIXME: Specialize a template for NC_UBYTE
template void NetCDF_Out::Add_Variable<char>(               const std::string name, const int type_index,
                                                            const char *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<short int>(          const std::string name, const int type_index,
                                                            const short int *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<unsigned short int>( const std::string name, const int type_index,
                                                            const unsigned short int *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<int>(                const std::string name, const int type_index,
                                                            const int *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<unsigned int>(       const std::string name, const int type_index,
                                                            const unsigned int *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<uint64_t>(           const std::string name, const int type_index,
                                                            const uint64_t *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<int64_t>(            const std::string name, const int type_index,
                                                            const int64_t *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<float>(              const std::string name, const int type_index,
                                                            const float *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable<double>(             const std::string name, const int type_index,
                                                            const double *const pointer,
                                                            const NetCDF_Dimensions dims,
                                                            const std::string units);
// FIXME: Specialize a template for NC_STRING

// NetCDF_Out::Add_Variable_Scalar()
template void NetCDF_Out::Add_Variable_Scalar<bool>(        const std::string name, const int type_index,
                                                            const bool *const pointer,
                                                            const std::string units);
// FIXME: Specialize a template for NC_BYTE
// FIXME: Specialize a template for NC_UBYTE
template void NetCDF_Out::Add_Variable_Scalar<char>(        const std::string name, const int type_index,
                                                            const char *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<short int>(   const std::string name, const int type_index,
                                                            const short int *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<unsigned short int>(const std::string name, const int type_index,
                                                            const unsigned short int *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<int>(         const std::string name, const int type_index,
                                                            const int *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<unsigned int>(const std::string name, const int type_index,
                                                            const unsigned int *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<uint64_t>(    const std::string name, const int type_index,
                                                            const uint64_t *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<int64_t>(     const std::string name, const int type_index,
                                                            const int64_t *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<float>(       const std::string name, const int type_index,
                                                            const float *const pointer,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_Scalar<double>(      const std::string name, const int type_index,
                                                            const double *const pointer,
                                                            const std::string units);
// FIXME: Specialize a template for NC_STRING

// NetCDF_Out::Add_Variable_1D()
template void NetCDF_Out::Add_Variable_1D<bool>(            const std::string name, const int type_index,
                                                            const bool *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
// FIXME: Specialize a template for NC_BYTE
// FIXME: Specialize a template for NC_UBYTE
template void NetCDF_Out::Add_Variable_1D<char>(            const std::string name, const int type_index,
                                                            const char *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<short int>(       const std::string name, const int type_index,
                                                            const short int *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<unsigned short int>(const std::string name, const int type_index,
                                                            const unsigned short int *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<int>(             const std::string name, const int type_index,
                                                            const int *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<unsigned int>(    const std::string name, const int type_index,
                                                            const unsigned int *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<uint64_t>(        const std::string name, const int type_index,
                                                            const uint64_t *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<int64_t>(         const std::string name, const int type_index,
                                                            const int64_t *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<float>(           const std::string name, const int type_index,
                                                            const float *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
template void NetCDF_Out::Add_Variable_1D<double>(          const std::string name, const int type_index,
                                                            const double *const pointer, const int N,
                                                            const std::string dim_name,
                                                            const std::string units);
// FIXME: Specialize a template for NC_STRING

#endif // #ifdef NETCDF

// ********** End of file ***************************************

