#include <iostream>
#include <netcdf>
#include <vector>
#include "nfdrs4.h"
#include "timer.h"

using namespace std;

constexpr int NO_DATA = -9999;
constexpr const char *INPUT_PARAMS = "./params.txt";
constexpr const char *INPUT_NFDRS = "./input_nfdrs.nc";
constexpr const char *OUTPUT_DFM = "./output_dfm.nc";

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
    Timer t;
    cout << "Reading data..." << endl;
    GridNFDRSData data = ReadNetCDF();

    size_t T = data.T;
    size_t N = data.N;
    size_t M = data.M;
    size_t spatialSize = N * M;

    // Output Data
    vector<double> GSI(T * N * M); 
    vector<double> MCWOOD(T * N * M); 
    vector<double> MCHERB(T * N * M); 
    vector<double> KBDI(T * N * M);
    vector<double> SC(T * N * M);
    vector<double> ERC(T * N * M);
    vector<double> BI(T * N * M);
    vector<double> IC(T * N * M);
    
    // Initialize NFDRS objects
    std::vector<NFDRS4> NFDRSGrid;
    for (size_t i = 0; i < spatialSize; ++i)
    {
        if ( data.isBurnable[i] == 1 )
        {
            NFDRSGrid.emplace_back(
                data.lat[i], 'A', data.slopeClass[i], data.annAvgPrec[i], true, true, false
            );  
        }  
    }
        
    // TODO: Process data as needed
    for (size_t t = 0; t < T; ++t)
    {
        for (size_t i = 0; i < spatialSize; ++i)
        {
            if ( data.isBurnable[i] == 1 )
            {
                size_t idx = t * spatialSize + i;
                // print the data
                cout << "Year: " << data.year[t] << endl;
                cout << "Month: " << data.month[t] << endl;
                cout << "Day: " << data.day[t] << endl;
                cout << "Hour: " << data.hour[t] << endl;
                cout << "Lat: " << data.lat[i] << endl;
                cout << "FuelModel: " << data.fModels[i] << endl;
                cout << "SlopeClass: " << data.slopeClass[i] << endl;
                cout << "AnnAvgPrec: " << data.annAvgPrec[i] << endl;
                cout << "isBurnable: " << data.isBurnable[i] << endl;
                cout << "Temp: " << data.temp[idx] << endl;
                cout << "RH: " << data.rh[idx] << endl;
                cout << "PPT: " << data.ppt[idx] << endl;
                cout << "SnowDay: " << data.snowDay[idx] << endl;
                // cout << "WindSpeed: " << data.windSpeed[idx] << endl;

                cout << "MC1: " << data.MC1[idx] << endl;
                cout << "MC10: " << data.MC10[idx] << endl;
                cout << "MC100: " << data.MC100[idx] << endl;
                cout << "MC1000: " << data.MC1000[idx] << endl;
                cout << "FuelTemp: " << data.fuelTemp[idx] << endl;

                goto A;
            }
        }
    }
    A:
    // Output

    return 0;
}
