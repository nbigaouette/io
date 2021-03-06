
#ifdef NETCDF

#include <stdint.h> // (u)int64_t
#include <sys/time.h> // timeval
#include <exception>
#include <list>

#include <StdCout.hpp>

#include "Classes_NetCDF.hpp"
#include "InputOutput.hpp"

template <class Integer>
inline std::string IntToStr(const Integer integer, const int width = 0, const char fill = ' ')
{
    std::ostringstream MyStream;
    if (width != 0)
    {
        MyStream << std::setw(width);
        MyStream << std::setfill(fill);
    }
    MyStream << integer << std::flush;
    return (MyStream.str());
}

#define ERR(e, filename, note)                                                 \
{                                                                              \
    std::string error_msg =                                                    \
          std::string("Classes_NetCDF.cpp ERROR: '")                           \
        + std::string(nc_strerror(e))                                          \
        + std::string(" (") + IntToStr(e) + std::string(")'");                 \
    if (note != "")                                                            \
        error_msg = error_msg + " (" + note + ")";                             \
    error_msg = error_msg + " working with file '" + filename + "'";           \
    std_cout << error_msg << "\n";                                             \
    std_cout.Flush();                                                          \
    throw std::ios_base::failure(error_msg);                                   \
}

// Compressiong options
const int C_shuffle = NC_SHUFFLE;
const int C_deflate = 1;
const int C_deflate_level = 9;

// const bool verbose = true;
const bool verbose = false;

void call_netcdf_and_test_generic(const int &netcdf_retval, const std::string &filename, const std::string note = "");

// *****************************************************************************
namespace Classes_NetCDF
{
    // *************************************************************************
    void Wait(const double duration_sec)
    {
        double delay = 0.0;
        timeval initial, now;
        gettimeofday(&initial, NULL);
        while (delay <= duration_sec)
        {
            gettimeofday(&now, NULL);
            // Transform time into double delay
            delay = double(now.tv_sec - initial.tv_sec) + 1.0e-6*double(now.tv_usec - initial.tv_usec);
            //printf("Delay = %.6f   max = %.6f\n", delay, duration_sec);
        }
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
}

// **************************************************************
void call_netcdf_and_test_generic(const int &netcdf_retval, const std::string &filename, const std::string note)
{
    if (netcdf_retval)
        ERR(netcdf_retval, filename, note);
}

// **************************************************************
void NetCDF_Variable::call_netcdf_and_test(const int netcdf_retval, const std::string note)
{
    // Get filename
    int return_value;
    char filename[2048];
    return_value = nc_inq_path(ncid, NULL, filename);
    if (return_value != NC_NOERR)
    {
        call_netcdf_and_test_generic(netcdf_retval, filename, note);
    }
}

// **************************************************************
void NetCDF_In::call_netcdf_and_test(const int netcdf_retval, const std::string note)
{
    call_netcdf_and_test_generic(netcdf_retval, filename, note);
}

// **************************************************************
void NetCDF_Out::call_netcdf_and_test(const int netcdf_retval, const std::string note)
{
    call_netcdf_and_test_generic(netcdf_retval, filename, note);
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
    is_compressed   = compress;
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
    call_netcdf_and_test(nc_put_att_text(ncid, varid, "units", units.size(), units.c_str()), "nc_put_att_text(units), variable name: " + name);
}

// **************************************************************
void NetCDF_Variable::Commit()
{
    if (verbose)
    {
        std_cout << "Commiting variable '" << name.c_str() << "' in file id '" << ncid << "' of type '" << type_index << "' ("<<netcdf_types_string[type_index]<<"), nb of dimension(s) '" << int(dimensions.ids.size()) << "'  (&(dimensions.ids[0]) = " << &(dimensions.ids[0]) << ")\n";
        for (int i = 0 ; i < int(dimensions.ids.size()) ; i++)
        {
            std_cout << "    Dimension: i=" << i << "  name = " << dimensions.names[i] << "  N = ";
            if (dimensions.Ns[i] < 0) std_cout << "UNLIMITED (" << -dimensions.Ns[i] << ")";
            else                      std_cout << dimensions.Ns[i];
            std_cout << "  id = " << (&(dimensions.ids[0]))[i] << "\n";
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
        ),
        "nc_def_var() (NetCDF_Variable::Commit()), variable name: " + name
    );

    if (verbose)
        std_cout << "  Variable committed. varid = '" << varid << "'\n";

    free(tmp_ids);

    if (is_compressed)
    {
        if (dimensions.ids.size() > 1 and dimensions.Ns[0] > 1)
        {
            // Compression can reduce a file size by a factor of 15!
            //call_netcdf_and_test(nc_def_var_chunking(ncid, varid, 0, &chunks[0]), "nc_def_var_chunking() (NetCDF_Variable::Commit()), variable name: " + variable_name);
            call_netcdf_and_test(nc_def_var_deflate(ncid, varid, C_shuffle, C_deflate, C_deflate_level), "nc_def_var_deflate() (NetCDF_Variable::Commit()), variable name: " + name);
        }
    }

    is_committed = true;
}

// **************************************************************
void NetCDF_Variable::Write()
{
    if (not is_committed)
        Commit();

    if (netcdf_type == NC_CHAR)
    {
        assert(dimensions.Ns.size() == 1);
        assert(dimensions.Ns[0] < 0);

        // Save array of characters (a string) one character at a time...
        for (int i = 0 ; i < -dimensions.Ns[0] ; i++)
        {
            size_t start = i;
            size_t count = 1;
            call_netcdf_and_test(
                nc_put_vara(ncid, varid, &start, &count, ((char *)pointer) + i),
                "nc_put_vara(), variable name: " + name);
        }
    }
    else
    {
        call_netcdf_and_test(nc_put_var(ncid, varid, pointer), "nc_put_var(), variable name: " + name);
    }
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
    const int max_nb_try = 5;
    int nb_try = 1;
    const int netcdf_filetype = (is_netcdf4 ? NC_NETCDF4 : NC_CLOBBER);

    while (nc_create(filename.c_str(), netcdf_filetype, &ncid) != NC_NOERR)
    {
        // Sleep 5 seconds before retrying
        std_cout << "WARNING: Could not open file \"" << filename << "\" for writting (" << nb_try << "/" << max_nb_try << "). " << std::flush;
        if (nb_try+1 > max_nb_try)
        {
            throw std::ios_base::failure("Could not open file \"" + filename + "\" for writting.");
        }
        else
        {
            std_cout << "Sleeping 5 seconds before re-trying...\n" << std::flush;
            nb_try++;
            Classes_NetCDF::Wait(5.0);
        }
    }

    is_opened    = true;

    // Disable filling. We will be writting right away, so filling is useless.
    int old_modep;
    call_netcdf_and_test( nc_set_fill(ncid, NC_NOFILL, &old_modep), "nc_set_fill()");

    if (verbose)
        std_cout << "File '" << filename << "' opened for writting with id '" << ncid << "'.\n";
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
        log("NetCDF_Out::Add_Variable() Adding variable '%s' (%p) of type '%d' (%s) to  file '%s'...\n",
            name.c_str(), pointer, type_index, netcdf_types_string[type_index], filename.c_str());

    // The actual write to the file will take place at Close() (or Write() more precisely) and
    // not when calling Add_Variable(). This is because all metadata of the file needs to be
    // set before writting data.
    // So make sure the pointer used for a new variable has not been used by another variable.
    const std::set<uint64_t>::iterator it = previous_variables_ptr.find(uint64_t(pointer));
    if (it == previous_variables_ptr.end())
    {
        // Not found: add it to the set then.
        previous_variables_ptr.insert(uint64_t(pointer));
    }
    else
    {
        log("ERROR: Two variables were added using NetCDF_Out::Add_Variable() using the same pointer!\n");
        log("       This will not work as NetCDF_Out::Add_Variable() just creates the variable's metada.\n");
        log("       The data will only be written to disk when NetCDF_Out::Close() is called (or at\n");
        log("       destructor). If two variables use the same pointer, the same data will be written for\n");
        log("       both variable!\n");
        log("       Variable names:\n");
        log("            Inserting:        %p %s\n", pointer, name.c_str());
        log("            Already inserted: %p\n", *it);
        log("       Please fix your code.\n");
        std_cout << std::flush;
        Classes_NetCDF::Pause();
        abort();
    }

    // Create empty variable
    variables[name] = NetCDF_Variable();
    variables[name].Init(ncid, name, pointer, type_index, is_netcdf4);

    // fdouble is not defined here. Codes can define it as "float" or "double". Since the
    // function definition for Add_Variable() is compiled before knowing which one will
    // be used, we need to treat this type differently.
    if (type_index == netcdf_type_fdouble)
    {
        // Set the right type_index
        if      (sizeof(T) == sizeof(float))
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
        {
            std_cout << "    NetCDF_Out::Add_Variable() 0. dimension: name="<<dims.names[i]<<"  N=";
            if (dims.Ns[i] < 0) std_cout << "UNLIMITED (" << -dims.Ns[i] << ")";
            else                std_cout << dims.Ns[i];
            std_cout <<"\n";
        }

        assert(dims.Ns[i] != 0 and dims.Ns[i] != NC_UNLIMITED);

        // Search for already saved/commited dimensions
        const std::map<std::string, size_t>::iterator it_search = dimensions_val.find(dims.names[i]);

        // If not found any, create the new dimension
        if (it_search == dimensions_val.end())
        {
            // Store temporary references
            const std::string dim_name  = dims.names[i];
            const size_t dim_N          = (dims.Ns[i] < 0 ? NC_UNLIMITED : dims.Ns[i]);

            if (verbose)
            {
                std_cout << "    NetCDF_Out::Add_Variable() 1. dimension: name="<<dim_name<<"  N=";
                if (dim_N == NC_UNLIMITED)  std_cout << "UNLIMITED (" << -dims.Ns[i] << ")";
                else                        std_cout << dim_N;
                std_cout <<" not found! Adding...\n";
            }

            // Set the stored values
            dimensions_val[dim_name] = dim_N;
            dimensions_ids[dim_name] = -1; // Default value, will be changed next

            if (type_index == netcdf_type_char)
            {
                assert(dims.Ns[i] < 0);
                assert(dim_N == NC_UNLIMITED);
            }

            // Commit this dimension (set dimensions_ids[dim_name])
            call_netcdf_and_test(
                nc_def_dim(
                    ncid,                       // File id
                    dim_name.c_str(),           // Name of dimension
                    dim_N,                      // Number of elements in that dimension
                    &(dimensions_ids[dim_name]) // Dimension commit id pointer
                ),
                "nc_def_dim() (NetCDF_Out::Add_Variable()), dimension name: " + dim_name
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
void NetCDF_Out::Add_Variable_2D(const std::string name, const int type_index,
                        const float *const pointer,
                        const int N, const int M,
                        const std::string dim_name_N, const std::string dim_name_M,
                        const std::string units)
{
    assert(is_opened);

    assert(N >= 0);
    assert(M >= 0);

    NetCDF_Dimensions tmp_dims;
    tmp_dims.Add(dim_name_N, N);
    tmp_dims.Add(dim_name_M, M);
    Add_Variable<float>(name, type_index, pointer, tmp_dims, units);
}

// **************************************************************
void NetCDF_Out::Add_Variable(const std::string name,
                              const std::string string_to_save)
{
    assert(is_opened);

    const int N = int(string_to_save.size());

    Add_Variable_1D<char>(name, netcdf_type_char, string_to_save.c_str(), -N, "len_" + name);
}

// **************************************************************
void NetCDF_Out::Commit()
{
    if (verbose and is_opened)
        std_cout << "NetCDF_Out::Commit(this="<<this<<","<<filename<<", is_committed="<<(is_committed?"true ":"false")<<")..." << "\n";

    // Not needed for NetCDF4 (?)
    // End define mode. This tells netCDF we are done defining metadata.
    if (not is_committed)
        call_netcdf_and_test(nc_enddef(ncid), "nc_enddef() (NetCDF_Out::Commit())");

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
        call_netcdf_and_test(nc_close(ncid), "nc_close() (NetCDF_Out::Close())");

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
    for (std::map<std::string, size_t>::const_iterator it = dimensions_val.begin() ; it != dimensions_val.end() ; it++ )
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

    const int max_nb_try = 5;
    int nb_try = 1;

    while (nc_open(filename.c_str(), NC_NOWRITE, &ncid) != NC_NOERR)
    {
        // Sleep 5 seconds before retrying
        std_cout << "WARNING: Could not open file \"" << filename << "\" for reading (" << nb_try << "/" << max_nb_try << "). " << std::flush;
        if (nb_try+1 > max_nb_try)
        {
            throw std::ios_base::failure("Could not open file \"" + filename + "\" for reading");
        }
        else
        {
            std_cout << "Sleeping 5 seconds before re-trying...\n" << std::flush;
            nb_try++;
            Classes_NetCDF::Wait(5.0);
        }
    }

    is_opened = true;

    if (verbose)
        std_cout << "File '" << filename << "' opened for reading with id '" << ncid << "'.\n";
}

// **************************************************************
NetCDF_In::~NetCDF_In()
{
    Close();
}

// **************************************************************
std::vector<std::string> Split(const std::string &s, const std::string delimiters = ";, ")
{
    // Initialize a list of a single element which contains the string to split
    std::list<std::string> elements(1,s);

    // Loop over all delimiters
    for (size_t i = 0 ; i < delimiters.size() ; i++)
    {
        // Store a temporary object. Necessary since the actual list will change when we split it.
        std::list<std::string> tmp_elements = elements;

        // Iterate over the list of strings and split each of them
        for (std::list<std::string>::iterator it = elements.begin() ; it != elements.end() ; it++)
        {
            // Store the string to split
            std::stringstream ss(*it);
            std::string item;

            // Remove the string from the temporary list
            tmp_elements.remove(*it);

            // Split the string into substrings and add them to the temporary list
            while (std::getline(ss, item, delimiters[i]))
            {
                tmp_elements.push_back(item);
            }
        }

        // Restore the list
        elements = tmp_elements;
    }

    // We wanted a vector to simplify looping outside the function.
    std::vector<std::string> substrings;
    for (std::list<std::string>::iterator it = elements.begin() ; it != elements.end() ; it++)
    {
        substrings.push_back(*it);
    }

    return substrings;
}

// **************************************************************
void NetCDF_In::Read(const std::string variable_name, void * const pointer)
{
    assert(is_opened);
    assert(pointer != NULL);

    // Split variable name
    std::vector<std::string> possible_variable_names = Split(variable_name);

    int varid;
    int return_value;
    bool read_successful = false;

    while (possible_variable_names.size() > 0)
    {
        return_value = nc_inq_varid(ncid, possible_variable_names.back().c_str(), &varid);
        // Reading fail, try next possible variable name.
        if (return_value != NC_NOERR)
        {
            possible_variable_names.pop_back();
            continue;
        }

        return_value = nc_get_var(ncid, varid, pointer);
        // Reading fail, try next possible variable name.
        if (return_value != NC_NOERR)
        {
            possible_variable_names.pop_back();
            continue;
        }

        read_successful = true;
        break;
    }

    if (not read_successful)
    {
        const std::string msg("Classes_NetCDF.cpp ERROR: Could not find any variable names from these possibilities: " + variable_name);
        std_cout << msg << "\n" << "Aborting.\n";
        std_cout.Flush();
        throw std::ios_base::failure(msg);
    }
}

// **************************************************************
void NetCDF_In::Read(const std::string variable_name, std::string &content)
{
    assert(is_opened);

    int varid;

    // Get the varid of the data variable, based on its name.
    call_netcdf_and_test( nc_inq_varid(ncid, variable_name.c_str(), &varid), "nc_inq_varid(), variable name: " + variable_name );

    char *content_temp = (char *)calloc(4096, sizeof(char));
    call_netcdf_and_test( nc_get_var(ncid, varid, (void *) content_temp ), "nc_get_var(), variable name: " + variable_name );
    content = std::string(content_temp);
    free(content_temp);
}

// **************************************************************
void NetCDF_In::Close()
{
    if (is_opened)
        call_netcdf_and_test(nc_close(ncid), "nc_close() (NetCDF_In::Close())");
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
#ifdef __i386__
// 'unsigned long int' is the same as uint64_t on x86_64, but not on i686!
template void NetCDF_Out::Add_Variable_Scalar<unsigned long int>(const std::string name, const int type_index,
                                                            const unsigned long int *const pointer,
                                                            const std::string units);
#endif // #ifdef __i386__
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


