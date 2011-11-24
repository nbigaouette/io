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

    delete[] array;

    test.Close_File();


    NetCDF_Out cdf_file("output/test.cdf");

    int    int_to_save    = 123456789;
    double double_to_save = 1.23456789;
    float  float_to_save  = 9.87654321;
    float  float_array[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    cdf_file.Add_Variable_Scalar("int_to_save",     netcdf_type_int,    &int_to_save, "Int units");
    cdf_file.Add_Variable_Scalar("double_to_save",  netcdf_type_double, &double_to_save, "Double units");
    cdf_file.Add_Variable_Scalar("float_to_save",   netcdf_type_float,  &float_to_save, "Float units");
    cdf_file.Add_Variable_1D("float_array",         netcdf_type_float,   float_array, 5, "Five", "Array units");

    cdf_file.Close();


    return EXIT_SUCCESS;
}
