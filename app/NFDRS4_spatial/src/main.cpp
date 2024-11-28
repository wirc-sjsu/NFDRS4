#include <iostream>
#include <netcdf>
#include <vector>
#include "nfdrs4.h"
#include "timer.h"

using namespace std;

constexpr int NO_DATA = -1;
constexpr const char *INPUT_NFDRS = "../data/input_nfdrs.nc";
constexpr const char *OUTPUT_DFM = "../data/output_dfm.nc";
constexpr const char *OUTPUT_NFDRS = "../data/output_nfdrs.nc";

struct GridNFDRSData
{
    size_t T, N, M;
    vector<int> year, month, day, hour;
    vector<double> lat, annAvgPrec;
    vector<int> fModels, slopeClass;
    vector<double> temp, rh, ppt;
    vector<int> snowDay;
    vector<double> windSpeed;
    vector<int> isBurnable;
    // DFM data
    vector<double> MC1, MC10, MC100, MC1000, fuelTemp;

    // Constructor for filtered data
    explicit GridNFDRSData(size_t T, size_t N, size_t M)
        : T(T), N(N), M(M),
          year(T), month(T), day(T), hour(T),
          lat(N * M), annAvgPrec(N * M),
          fModels(N * M), slopeClass(N * M),
          temp(T * N * M), rh(T * N * M), ppt(T * N * M),
          snowDay(T * N * M), windSpeed(T * N * M),
          isBurnable(N * M),
          MC1(T * N * M), MC10(T * N * M), MC100(T * N * M), 
          MC1000(T * N * M), fuelTemp(T * N * M)
    {
    }
};

GridNFDRSData ReadNetCDF(const string &input_nfdrs = INPUT_NFDRS, const string &output_dfm = OUTPUT_DFM)
{
    try
    {
        // Open the NetCDF file
        netCDF::NcFile nfdrs(input_nfdrs, netCDF::NcFile::read);
        netCDF::NcFile dfm(output_dfm, netCDF::NcFile::read);

        // Get the dimensions
        netCDF::NcDim timeDim = nfdrs.getDim("time");
        netCDF::NcDim southNorthDim = nfdrs.getDim("south_north");
        netCDF::NcDim westEastDim = nfdrs.getDim("west_east");

        size_t T = timeDim.getSize();
        size_t N = southNorthDim.getSize();
        size_t M = westEastDim.getSize();

        GridNFDRSData data(T, N, M);

        // Get the variables
        netCDF::NcVar yearVar = nfdrs.getVar("Year");
        netCDF::NcVar monthVar = nfdrs.getVar("Month");
        netCDF::NcVar dayVar = nfdrs.getVar("Day");
        netCDF::NcVar hourVar = nfdrs.getVar("Hour");
        netCDF::NcVar latVar = nfdrs.getVar("Latitude");
        netCDF::NcVar fModelsVar = nfdrs.getVar("FuelModel");
        netCDF::NcVar slopeClassVar = nfdrs.getVar("SlopeClass");
        netCDF::NcVar annAvgPrecVar = nfdrs.getVar("AnnAvgPPT");
        netCDF::NcVar isBurnableVar = nfdrs.getVar("isBurnable");
        netCDF::NcVar tempVar = nfdrs.getVar("Temp");
        netCDF::NcVar rhVar = nfdrs.getVar("RH");
        netCDF::NcVar pptVar = nfdrs.getVar("PPT");
        netCDF::NcVar snowDayVar = nfdrs.getVar("SnowDay");
        netCDF::NcVar windSpeedVar = nfdrs.getVar("WindSpeed");

        netCDF::NcVar mc1Var = dfm.getVar("MC1");
        netCDF::NcVar mc10Var = dfm.getVar("MC10");
        netCDF::NcVar mc100Var = dfm.getVar("MC100");
        netCDF::NcVar mc1000Var = dfm.getVar("MC1000");
        netCDF::NcVar fuelTempVar = dfm.getVar("FuelTemp");

        // Read the data from the variables into the vectors
        yearVar.getVar(&data.year[0]);
        monthVar.getVar(&data.month[0]);
        dayVar.getVar(&data.day[0]);
        hourVar.getVar(&data.hour[0]);
        latVar.getVar(&data.lat[0]);
        fModelsVar.getVar(&data.fModels[0]);
        slopeClassVar.getVar(&data.slopeClass[0]);
        annAvgPrecVar.getVar(&data.annAvgPrec[0]);
        isBurnableVar.getVar(&data.isBurnable[0]);
        tempVar.getVar(&data.temp[0]);
        rhVar.getVar(&data.rh[0]);
        pptVar.getVar(&data.ppt[0]);
        snowDayVar.getVar(&data.snowDay[0]);
        windSpeedVar.getVar(&data.windSpeed[0]);

        mc1Var.getVar(&data.MC1[0]);
        mc10Var.getVar(&data.MC10[0]);
        mc100Var.getVar(&data.MC100[0]);
        mc1000Var.getVar(&data.MC1000[0]);
        fuelTempVar.getVar(&data.fuelTemp[0]);

        // Close the NetCDF file
        nfdrs.close();
        dfm.close();

        return data;
    }
    catch (const netCDF::exceptions::NcException &e)
    {
        cerr << "NetCDF Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

int main()
{
    cout << "Reading data..." << endl;
    // Read Input Data
    GridNFDRSData data = ReadNetCDF();
    
    cout << "Initialize models..." << endl;
    // Define Dimension Sizes
    size_t T = data.T;
    size_t N = data.N;
    size_t M = data.M;
    size_t spatialSize = N * M;

    // Initialize Output Data Arrays
    vector<double> KBDI(T * N * M);
    vector<double> GSI(T * N * M); 
    vector<double> MCWOOD(T * N * M); 
    vector<double> MCHERB(T * N * M); 
    vector<double> SC(T * N * M);
    vector<double> ERC(T * N * M);
    vector<double> BI(T * N * M);
    vector<double> IC(T * N * M);
    
    // Define Fuel Behaviour Model Mapping
    std::map<int, char> fModelMap = {
        {1, 'V'},
        {2, 'W'},
        {3, 'X'},
        {4, 'Y'},
        {5, 'Z'}
    };

    // Initialize NFDRS objects
    std::vector<NFDRS4> NFDRSGrid;
    for (size_t i = 0; i < spatialSize; ++i)
    {
        // Only define NFDRS class at burnable locations
        if ( data.isBurnable[i] == 1 )
        {
            // Fuel Model in the input data
            int fModel = data.fModels[i];
            // Search for the Fuel Model in the Map
            if (fModelMap.find(fModel) != fModelMap.end()) {
                // Define NFDRS fuel model class
                char fModelClass = fModelMap[fModel];
                // Initialize NFDRS class
                NFDRSGrid.emplace_back(
                    data.lat[i], fModelClass, data.slopeClass[i], data.annAvgPrec[i], true, true, false
                );
            } else {
                printf("WARNING: fuel model out of range");
            }
        }  
    }
    
    // Timestep loop
    for (size_t t = 0; t < T; ++t)
    {
        Timer timer;
        printf("\nTimestep: %zu/%zu...", (t + 1), T);
        int c = 0;
        // Spatial loop
        for (size_t i = 0; i < spatialSize; ++i)
        {
            size_t idx = t * spatialSize + i;
            if ( data.isBurnable[i] == 1 )
            {
                // Update NFDRS state
                NFDRSGrid[c].Update(
                    data.year[t], data.month[t], data.day[t], data.hour[t], 
                    data.temp[idx], data.rh[idx], data.ppt[idx], data.windSpeed[idx], 
                    data.snowDay[idx], data.MC1[idx], data.MC10[idx], data.MC100[idx], 
                    data.MC1000[idx], data.fuelTemp[idx]
                );
                // Get output variables
                KBDI[idx] = NFDRSGrid[c].KBDI;
                GSI[idx] = NFDRSGrid[c].m_GSI;
                MCWOOD[idx] = NFDRSGrid[c].MCWOOD;
                MCHERB[idx] = NFDRSGrid[c].MCHERB;
                SC[idx] = NFDRSGrid[c].SC;
                ERC[idx] = NFDRSGrid[c].ERC;
                BI[idx] = NFDRSGrid[c].BI;
                IC[idx] = NFDRSGrid[c].IC;
                c += 1;
            } else {
                // Fill output with no data value
                KBDI[idx] = NO_DATA;
                GSI[idx] = NO_DATA;
                MCWOOD[idx] = NO_DATA;
                MCHERB[idx] = NO_DATA;
                SC[idx] = NO_DATA;
                ERC[idx] = NO_DATA;
                BI[idx] = NO_DATA;
                IC[idx] = NO_DATA;
            }
        }
        printf("\tDone\n");
    }
    
    // Write output into NetCDF file
    // Open NetCDF file
    netCDF::NcFile outputFile(OUTPUT_NFDRS, netCDF::NcFile::replace);
    // Get dimensions
    auto timeDim = outputFile.addDim("time", T);
    auto southNorthDim = outputFile.addDim("south_north", N);
    auto westEastDim = outputFile.addDim("west_east", M);
    // Get variables
    auto KBDIData = outputFile.addVar("KBDI", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto GSIData = outputFile.addVar("GSI", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto MCWOODData = outputFile.addVar("MCWOOD", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto MCHERBData = outputFile.addVar("MCHERB", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto SCData = outputFile.addVar("SC", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto ERCData = outputFile.addVar("ERC", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto BIData = outputFile.addVar("BI", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto ICData = outputFile.addVar("IC", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    // Write variables from output data
    KBDIData.putVar(&KBDI[0]);
    GSIData.putVar(&GSI[0]);
    MCWOODData.putVar(&MCWOOD[0]);
    MCHERBData.putVar(&MCHERB[0]);
    SCData.putVar(&SC[0]);
    ERCData.putVar(&ERC[0]);
    BIData.putVar(&BI[0]);
    ICData.putVar(&IC[0]);
    // Close NetCDF file
    outputFile.close();

    return 0;
}
