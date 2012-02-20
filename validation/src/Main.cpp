/***************************************************************
 *
 *
 ***************************************************************/

#include <cstdlib>
#include <iostream>

#include <InputOutput.hpp>
#include <Classes_NetCDF.hpp>

// **************************************************************
int main(int argc, char *argv[])
{
    // **********************************************************
    // Open log file
    std_cout.open("output/output.log");

    // **********************************************************
    // IO class
    IO test(true);
    test.Set_Filename("output/test.txt");
    test.Open_File("wz");

    const int n = 10000;
    double * array = new double[n];
    for (int i = 0 ; i < n ; i++)
    {
        array[i] = double(i);
        //std_cout << "array[i="<<i<<"] = " << array[i] << "\n";
        test.WriteString("%g\n", array[i]);
    }
    test.Close_File();
    delete[] array;

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


    // **********************************************************
    // NetCDF class
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

    // Now read data back
    NetCDF_In cdf_file_in(netcdf_file);
    cdf_file_in.Read("int_to_save",    &int_to_save);
    cdf_file_in.Read("double_to_save", &double_to_save);
    cdf_file_in.Read("float_to_save",  &float_to_save);
    cdf_file_in.Read("float_array",     float_array); // float_array is already a pointer.


    // **********************************************************
    // XML class
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
        xml_input.Verify_Unit("subnode1/subsubnode_double", "wrong_unit");
    }
    catch (std::ios_base::failure &error)
    {
        std_cout << "Unit verification failed: " << error.what() << "\n";
    }

    // Modify one or more values
    xml_input.Set("subnode1/subsubnode_double", 9.87654321);

    // Dump the modified content to a new file
    xml_input.Dump("output/modified.xml");


    // **********************************************************
    std_cout << "Done!\n";

    return EXIT_SUCCESS;
}
