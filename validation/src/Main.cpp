/***************************************************************
 *
 *
 ***************************************************************/

#include <cstdlib>
#include <iostream>

#include <InputOutput.hpp>

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
    return EXIT_SUCCESS;
}

