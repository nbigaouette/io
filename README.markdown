# IO
io, a C++ library to help out in input and output.

# Supports
* NetCDF wrapper
The Network Common Data Form (NetCDF) is, [according to its website](http://www.unidata.ucar.edu/software/netcdf/):
> a set of software libraries and self-describing, machine-independent data
> formats that support the creation, access, and sharing of array-oriented
> scientific data.

It is a great way to store data since it is portable, but most importantly
self-describing. For example, you can store the units of a data set right beside
the actual data. NetCDF files can be compressed on the fly, reducing file size.
Since data is stored in binary form, snapshots from simulations can be perfomed.
The library has two objects to help using NetCDF, mainly **NetCDF_Out** and
**NetCDF_In**. See the example section for usage.

* XML
A class named **ReadXML** allows easy access to XML files using
[https://en.wikipedia.org/wiki/Xpath](XPath). XPath just means nodes are accessed
through their path in the XML hierarchy. This allows using XML files as input
files for a simulation package. Internally, it uses
[http://www.grinninglizard.com/tinyxml/](TinyXML) (under zlib license).
See the example section for usage.

* Input/Output class for simplification of I/O in simulation packages. See the
example section for usage.


# Compilation
Requirements: [https://github.com/nbigaouette/stdcout](stdcout) and
[https://github.com/nbigaouette/assert](assert). To skip these dependencies,
comment the "#include <StdCout.hpp>" and "#include <Assert.hpp>" in the .cpp
files. You'll need to comment out any std_cout.Flush().

To build and install:

``` bash
$ make gcc shared static install
```

By default, it will be installed in ${HOME}/usr. You can install somewhere else
using DESTDIR:

``` bash
$ make gcc shared static install DESTIR=/usr
```

Library name will be "libinputoutput".


# Example
For some examples, see _validation_ folder:

``` bash
$ cd validation
$ make clean_data gcc run
```

which compiles _validation/src/Main.cpp_.

Here's some basic usage examples:

## NetCDF

### Output
Note that Add_Variable*() take the pointer to the actual data, be it the address
of variables or the address to the first element of an array.

``` C++
    const std::string netcdf_file("output/test.cdf");
    NetCDF_Out cdf_file_out(netcdf_file);

    int    int_to_save    = 123456789;
    double double_to_save = 1.23456789;
    float  float_to_save  = 9.87654321;
    float  float_array[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    cdf_file_out.Add_Variable_Scalar("int_to_save",     netcdf_type_int,    &int_to_save, "Int units");
    cdf_file_out.Add_Variable_Scalar("double_to_save",  netcdf_type_double, &double_to_save, "Double units");
    cdf_file_out.Add_Variable_Scalar("float_to_save",   netcdf_type_float,  &float_to_save, "Float units");
    cdf_file_out.Add_Variable_1D("float_array",         netcdf_type_float,   float_array, 5, "Five", "Array units");

    cdf_file_out.Close();
```

### Input
Because NetCDF files are self-describing, just calling Read() is enough. To read
the previously created file (note again the pointer arguments):

``` C++
    const std::string netcdf_file("output/test.cdf");
    NetCDF_In cdf_file_in(netcdf_file);
    cdf_file_in.Read("int_to_save",    &int_to_save);
    cdf_file_in.Read("double_to_save", &double_to_save);
    cdf_file_in.Read("float_to_save",  &float_to_save);
    cdf_file_in.Read("float_array",     float_array); // float_array is already a pointer.
```

## XML
Read data from "input/test.xml" file, validate some attributres, modify one
value and save the result in "output/modified.xml":

``` C++
    const std::string xml_filename("input/test.xml");
    ReadXML xml_input("root_node", xml_filename);

    // Read data
    const double      val1 = xml_input.Get_Double("subnode1/subsubnode_double");
    const int         val2 = xml_input.Get_Int(   "subnode1/subsubnode_int");
    const std::string val3 = xml_input.Get_String("subnode1/polarization");

    // Check the "enable" attribute
    const bool val2_enable = xml_input.Is_Enable("subnode1/subsubnode_int");
    const bool val3_enable = xml_input.Is_Enable("subnode1/polarization");

    // Validate any attributes value
    xml_input.Verify_Attribute("subnode1/polarization", "circular", "yes");
    // The following two give the same
    xml_input.Verify_Attribute("subnode1/subsubnode_double", "unit", "meters");
    xml_input.Verify_Unit("subnode1/subsubnode_double", "meters");

    // Verify_Unit() throws a std::ios_base::failure exception on failure.
    try
    {
        xml_input.Verify_Unit("subnode1/subsubnode_double", "meters2");
    }
    catch (std::ios_base::failure &error)
    {
        std_cout << "Unit verification failed: " << error.what() << "\n";
    }

    // Modify one or more values
    xml_input.Set("subnode1/subsubnode_double", 9.87654321);

    // Dump the modified content to a new file
    xml_input.Dump("output/modified.xml");
```

## IO
The IO class allows to read or write to a file. Its **Init()** function takes
the filename but also the period at which IO is to be performed on the file (-1
to have save every time step):

``` C++
    // Save every "period" to the file. Set period to "-1" to save every time step.
    const double dt     = 0.01;
    const double tmax   = 100.0;
    const double period = 0.3654;
    IO output_file;
    output_file.Init(period, "output/periodic_output.txt");
    output_file.Open_File("w");
    output_file.WriteString("# dt     = %g\n", dt);
    output_file.WriteString("# tmax   = %g\n", tmax);
    output_file.WriteString("# period = %g\n", period);
    for (double time = 0.0 ; time < tmax ; time += dt)
    {
        if (output_file.Is_Output_Permitted(time))
            output_file.WriteString("time = %g\n", time);
    }
    output_file.Close_File();
```

# License

This code is distributed under the terms of the [GNU General Public License v3 (GPLv3)](http://www.gnu.org/licenses/gpl.html) and is Copyright 2011 Nicolas Bigaouette.

