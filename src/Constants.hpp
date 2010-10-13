#ifndef INC_LIBPOTENTIALS_CONSTANTS_hpp
#define INC_LIBPOTENTIALS_CONSTANTS_hpp

#include <cmath>

namespace libtinyxml
{

    // The base potential well giving the bottom of the potential well.
    //const double base_pot_well_depth = 26.0; //[eV]

    // **************************************************************

    const double zero              = 0.0;
    const double one               = 1.0;
    const double two               = 2.0;
    const double three             = 3.0;
    const double four              = 4.0;
    const double seven             = 7.0;
    const double eight             = 8.0;

    const double Pi             = std::acos(-one);
    const double twoPi          = two * Pi;
    const double sqrt_Pi        = std::sqrt(Pi);
    const double sqrt_2         = std::sqrt(two);
    const double sqrt_2_over_pi = std::sqrt(two / Pi);
    const double one_over_Pi    = one / Pi;
    const double two_over_sqrt_Pi = two / sqrt_Pi;

    const double two_over_three = two / three;
    const double three_over_two = three / two;

    const double one_over_three = one / three;
    const double one_over_four  = one / four;
    const double one_over_seven = one / seven;

    const double one_over_four_Pi = one_over_four * one_over_Pi;
    const double sqrt_one_over_four_Pi = std::sqrt(one_over_four_Pi);

    const double four_over_twenty_seven = four / 27.0;
    const double eight_over_three       = eight / three;
    const double three_over_eight       = three / eight;
    const double three_over_sixteen     = three / 16.0;

    const int ZERO              = 0;
    const int ONE               = 1;
    const int TWO               = 2;

    // **************************************************************
    // ********** http://physics.nist.gov/cuu/Constants *************
    // **************************************************************


    // **************************************************************
    // ********** SI

    const double co             = double(299792458.0);          // Speed of ligth in vacuum [m/s]
    const double co2            = co*co;                // [m^2/s^2]
    const double inv_co         = one / co;             // 1/co [s/m]
    const double inv_co2        = one / (co * co);      // 1/co^2 [s^2/m^2]
    const double mu0            = double(4.0e-7) * Pi;          // Magnetic constant (vacuum) [N/A^2]
    const double eps0           = inv_co2 / mu0;        // Electric constant (vacuum) [F/m]
    const double eta0           = mu0 * co;             // Impedance of vacuum [Ohm]
    const double one_over_4Pieps0                       // Coulomb's law
                                = one_over_four_Pi / eps0;// constant
                                                        // [V.m.C^-1]

    const double e0             = double(1.602176462e-19);      // Unitary charge [C]
    const double qe             = -e0;                  // Electron charge [C]
    const double qp             =  e0;                  // Proton charge [C]
    const double mp             = double(1.67262158e-27);       // Proton mass [kg]
    const double mp_eV          = mp * co*co / e0;      // Proton mass energy (mc^2) [eV]
    const double mp_MeV         = mp_eV / double(1.0e6);        // Proton mass energy (mc^2) [MeV]
    const double me             = double(9.10938188e-31);       // Electron mass [kg]
    const double me_eV          = me * co*co / e0;      // Electron mass energy (mc^2) [eV]
    const double me_MeV         = me_eV / double(1.0e6);        // Electron mass energy (mc^2) [MeV]
    const double one_over_me    = one / me;             // [1/kg]
    const double one_over_mp    = one / mp;             // [1/kg]

    const double planck         = double(6.62606896e-34);       // Planck constant [J . s]
    const double hbar           = planck / twoPi;       // Planck cst / 2 pi [J . s]
    const double a0             = hbar*hbar / (one_over_4Pieps0 * me * e0*e0);
                                                        // Bohr radius [m]
    const double Na             = double(6.02214179e23);        // Avogadro constant [mol^-1]

    const double kB             = double(1.3806504e-23);        // Boltzmann constant  J . K^-1

    // **************************************************************
    // ********** Atomic Units

    const double Eh             = hbar*hbar/(me*a0*a0); // ...of energy (Hartree) [J]
    const double au_energy      = Eh;                   // ...of energy (Hartree) [J]
    const double au_time        = hbar / Eh;            // ...of time [s]
    const double au_electric_field = Eh / (e0 * a0);    // ...of electric field [V . m^-1]
    const double w_au           = one / au_time;        // ...of frequency [???]
    const double alpha          = double(7.29735257e-3);        //fine-structure constant

    // **************************************************************
    // ********** Conversions

    const double Eh_to_eV       = Eh / e0;              // eV . Eh^-1
    const double eV_to_Eh       = one / Eh_to_eV;       // Eh . eV^-1
    const double eV_to_J        = e0;                   // J . eV^-1
    const double J_to_eV        = one / eV_to_J;        // eV . J^-1
    const double J_to_Eh        = J_to_eV * eV_to_Eh;   // Eh . J^-1
    const double Eh_to_J        = one / J_to_Eh;        // J . Eh^-1

    const double m_to_angstrom  = double(1.0e10);               // Å . m^-1
    const double angstrom_to_m  = one / m_to_angstrom;  // m . Å^-1
    const double bohr_to_m      = a0;                   // m . bohr^-1
    const double m_to_bohr      = one / bohr_to_m;      // bohr . m^-1
    const double m_to_cm        = double(100.0);                // cm . m^-1
    const double cm_to_m        = one / m_to_cm;        // cm . m^-1
    const double m_to_mm        = 1.0e3;                // mm . m^-1
    const double mm_to_m        = 1.0 / m_to_mm;        // m . mm^-1
    const double m_to_mum       = 1.0e6;                // mum . m^-1
    const double mum_to_m       = 1.0 / m_to_mum;       // m . mum^-1
    const double m_to_nm        = double(1.0e9);                // nm . m^-1
    const double nm_to_m        = one / m_to_nm;        // m . nm^-1
    const double bohr_to_angstrom = bohr_to_m * m_to_angstrom;  // Å . bohr^-1
    const double angstrom_to_bohr = one / bohr_to_angstrom;     // bohr . Å^-1

    const double as_to_s        = double(1.0e-18);              // s . as^-1
    const double s_to_as        = one / as_to_s;        // as . s^-1
    const double fs_to_s        = double(1.0e-15);              // s . fs^-1
    const double s_to_fs        = one / fs_to_s;        // fs . s^-1
    const double ps_to_s        = double(1.0e-12);              // s . ps^-1
    const double s_to_ps        = one / ps_to_s;        // ps . s^-1
    const double ns_to_s        = double(1.0e-9);               // s . ns^-1
    const double s_to_ns        = one / ns_to_s;        // ns . s^-1
    const double mus_to_s       = double(1.0e-6);               // s . mus^-1
    const double s_to_mus       = one / mus_to_s;       // mus . s^-1
    const double ms_to_s        = double(1.0e-3);               // s . ms^-1
    const double s_to_ms        = one / ms_to_s;        // ms . s^-1

    const double deg_to_rad     = two * Pi / double(360.0);     // radians . degrees^-1
    const double rad_to_deg     = one / deg_to_rad;     // degrees . radians^-1

    const double byte_to_KiB    = one / double(1024.0);
    const double byte_to_MiB    = byte_to_KiB / double(1024.0);
    const double byte_to_GiB    = byte_to_MiB / double(1024.0);

    // ********** SI  <--> Atomic Units Conversions
    const double au_to_si_mass  = me;                   // kg . au^-1
    const double si_to_au_mass  = one / au_to_si_mass;  // au . kg^-1

    const double au_to_si_length= a0;                   // m . bohr^-1
    const double si_to_au_length= one / au_to_si_length;// bohr . m^-1

    const double au_to_si_charge= e0;                   // C . au^-1
    const double si_to_au_charge= one / au_to_si_charge;// au . C^-1

    const double au_to_si_energy= Eh;                   // J . au^-1
    const double si_to_au_energy= one / au_to_si_energy;// au . J^-1

    const double au_to_si_k     = one_over_4Pieps0;     // C^-2 . N . m^2 . au^-1
    const double si_to_au_k     = one / au_to_si_energy;// au . C^2 . N^-1 . m^-2

    const double au_to_si_time  = hbar / Eh;            // s . au^-1
    const double si_to_au_time  = one / au_to_si_time;  // au . s^-1

    const double au_to_si_force = Eh / a0;              // N . au^-1
    const double si_to_au_force = one / au_to_si_force; // au . N^-1

    const double au_to_si_field = au_electric_field;    // V . m^-1 . au^-1
    const double si_to_au_field = one / au_to_si_field; // au . m . V^-1

    const double au_to_si_pot   = au_electric_field * a0; // V . au^-1
    const double si_to_au_pot   = one / au_to_si_pot;   // au . V^-1

    const double au_to_si_vel   = Eh * a0 / hbar;       // m.s^-1 . au^-1
    const double si_to_au_vel   = one / au_to_si_vel;   // au . m.s^-1


    // ********** Other Usefull Conversions
    // Cross sections (area): Barn (b), Megabarnes (Mb) http://en.wikipedia.org/wiki/Barn_(unit)
    const double b_to_m2        = double(1.0e-28);              // m^2 . b^-1
    const double m2_to_b        = one / b_to_m2;        // b . m^-2
    const double Mb_to_m2       = double(1.0e6) * b_to_m2;      // m^2 . Mb^-1
    const double m2_to_Mb       = one / Mb_to_m2;       // Mb . m^-2
    const double m2_to_atomic_area = si_to_au_length * si_to_au_length; // bohr^2 . m^-2
    const double atomic_area_to_m2 = one / m2_to_atomic_area;           // m^2 . bohr^-2
    const double Mb_to_atomic_area = Mb_to_m2 * m2_to_atomic_area;      // bohr^2 . Mb^-1
    const double atomic_area_to_Mb = one / Mb_to_atomic_area;           // Mb . bohr^-2

    // **************************************************************
    // **********  Print all constants
    void Print_Constants();
}

#endif // INC_LIBPOTENTIALS_CONSTANTS_hpp

// ********** End of File ***************************************
