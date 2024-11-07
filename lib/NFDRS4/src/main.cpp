#include <iostream>
#include <vector>
#include <netcdf>
#include <string>
#include "nfdrs4.h"

using namespace std;

constexpr const char *INPUT_DIR = "../../data/";
constexpr const char *OUTPUT_DIR = "../../data/";
constexpr const int m_regObsHour = 13;


struct GridNFDRSData
{
    size_t T, N, M; // T timesteps, NxM Grid
    vector<int> year, month, day, hour;
    vector<double> julian, lat;
    vector<double> temp, rh;
    vector<double> minTemp, maxTemp;
    vector<double> minRH;
    vector<double> prec24;
    vector<double> annAvgPrec;
    vector<bool> snowDay;

    vector<double> windSpeed;
    vector<int> slopeClass;

    explicit GridNFDRSData(size_t T, size_t N, size_t M)
        : T(T), N(N), M(M),
          year(T), month(T), day(T), hour(T),
          julian(T), lat(N * M), temp(T * N * M), 
          rh(T * N * M), minTemp(T * N * M), maxTemp(T * N * M),
          minRH(T * N * M), prec24(T * N * M), annAvgPrec(N * M), 
          snowDay(T * N * M),
          windSpeed(T * N * M), slopeClass(T * N * M)
    {
    }
};

struct GridDFMData
{
    size_t T, N, M; // T timesteps, NxM Grid
    vector<double> oneHourMedians;
    vector<double> tenHourMedians;
    vector<double> hundredHourMedians;
    vector<double> thousandHourMedians;
    vector<double> fuelTemperatures;

    explicit GridDFMData(size_t T, size_t N, size_t M)
        : T(T), N(N), M(M),
        oneHourMedians(T * N * M),
        tenHourMedians(T * N * M),
        hundredHourMedians(T * N * M),
        thousandHourMedians(T * N * M),
        fuelTemperatures(T * N * M)
    {
    }
};


// Function to read variables from output_dfm.nc
GridDFMData ReadDFMOutput(const string &filename)
{
    cout << "Reading " << filename << endl;
    GridDFMData data(0, 0, 0);
    auto& [T, N, M, oneHourMedians, tenHourMedians, hundredHourMedians, thousandHourMedians, fuelTemperatures] = data;
    try
    {
        netCDF::NcFile dfmFile(filename, netCDF::NcFile::read);

        // Get the dimensions
        netCDF::NcDim timeDim = dfmFile.getDim("time");
        netCDF::NcDim southNorthDim = dfmFile.getDim("south_north");
        netCDF::NcDim westEastDim = dfmFile.getDim("west_east");

        T = timeDim.getSize();
        N = southNorthDim.getSize();
        M = westEastDim.getSize();

        size_t totalSize = T * N * M;

        oneHourMedians.resize(totalSize);
        fuelTemperatures.resize(totalSize);
        tenHourMedians.resize(totalSize);
        hundredHourMedians.resize(totalSize);
        thousandHourMedians.resize(totalSize);

        // Get the variables
        netCDF::NcVar oneHourMediansVar = dfmFile.getVar("MC1");
        netCDF::NcVar fuelTemperaturesVar = dfmFile.getVar("FuelTemp");
        netCDF::NcVar tenHourMediansVar = dfmFile.getVar("MC10");
        netCDF::NcVar hundredHourMediansVar = dfmFile.getVar("MC100");
        netCDF::NcVar thousandHourMediansVar = dfmFile.getVar("MC1000");

        // Read the data
        oneHourMediansVar.getVar(&oneHourMedians[0]);
        fuelTemperaturesVar.getVar(&fuelTemperatures[0]);
        tenHourMediansVar.getVar(&tenHourMedians[0]);
        hundredHourMediansVar.getVar(&hundredHourMedians[0]);
        thousandHourMediansVar.getVar(&thousandHourMedians[0]);

        dfmFile.close();
        cout << "Done\n"
             << endl;
    }
    catch (netCDF::exceptions::NcException &e)
    {
        cerr << "NetCDF Error while reading " << filename << ": " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    return data;
}

// Function to read variables from output_lfm.nc
void ReadLFMOutput(const string &filename,
                   vector<double> &woodyFM,
                   vector<double> &herbaceousFM,
                   vector<double> &gsi,
                   size_t &T, size_t &N, size_t &M)
{
    try
    {
        netCDF::NcFile lfmFile(filename, netCDF::NcFile::read);

        // Get the dimensions
        netCDF::NcDim timeDim = lfmFile.getDim("time");
        netCDF::NcDim southNorthDim = lfmFile.getDim("south_north");
        netCDF::NcDim westEastDim = lfmFile.getDim("west_east");

        T = timeDim.getSize();
        N = southNorthDim.getSize();
        M = westEastDim.getSize();

        size_t totalSize = T * N * M;

        woodyFM.resize(totalSize);
        herbaceousFM.resize(totalSize);
        gsi.resize(totalSize);

        // Get the variables
        netCDF::NcVar woodyFMVar = lfmFile.getVar("MCWOOD");
        netCDF::NcVar herbaceousFMVar = lfmFile.getVar("MCHERB");
        netCDF::NcVar gsiVar = lfmFile.getVar("GSI");

        // Read the data
        woodyFMVar.getVar(&woodyFM[0]);
        herbaceousFMVar.getVar(&herbaceousFM[0]);
        gsiVar.getVar(&gsi[0]);

        lfmFile.close();
        cout << "Done\n" << endl;
    }
    catch (netCDF::exceptions::NcException &e)
    {
        cerr << "NetCDF Error while reading " << filename << ": " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

// Function to read variables from input_calc.nc
GridNFDRSData ReadNFDRSInput(const string &filename)
{
    cout << "Reading " << filename << endl;
    GridDFMData data(0, 0, 0);
    auto& [T, N, M, year, month, day, hour, julian, lat, temp, rh, minTemp, maxTemp, minRH, prec24, annAvgPrec, snowDay, windSpeed, slopeClass] = data;
    try
    {
        netCDF::NcFile calcFile(filename, netCDF::NcFile::read);

        // Get the dimensions
        netCDF::NcDim timeDim = calcFile.getDim("time");
        netCDF::NcDim southNorthDim = calcFile.getDim("south_north");
        netCDF::NcDim westEastDim = calcFile.getDim("west_east");

        T = timeDim.getSize();
        N = southNorthDim.getSize();
        M = westEastDim.getSize();

        size_t totalSize = T * N * M;

        year.resize(T);
        month.resize(T);
        day.resize(T);
        hour.resize(T);
        julian.resize(T);
        lat.resize(N * M);
        temp.resize(T * N * M);
        rh.resize(T * N * M);
        minTemp.resize(T * N * M);
        maxTemp.resize(T * N * M);
        minRH.resize(T * N * M);
        prec24.resize(T * N * M);
        annAvgPrec.resize(N * M);
        snowDay.resize(T * N * M);
        windSpeed.resize(T * N * M);
        slopeClass.resize(T * N * M);

        // Get the variables
        netCDF::NcVar yearVar = calcFile.getVar("Year");
        netCDF::NcVar monthVar = calcFile.getVar("Month");
        netCDF::NcVar dayVar = calcFile.getVar("Day");
        netCDF::NcVar hourVar = calcFile.getVar("Hour");
        netCDF::NcVar julianVar = calcFile.getVar("Julian");
        netCDF::NcVar latVar = calcFile.getVar("Latitude");
        netCDF::NcVar tempVar = calcFile.getVar("Temp");
        netCDF::NcVar rhVar = calcFile.getVar("RH");
        netCDF::NcVar minTempVar = calcFile.getVar("MinTemp");
        netCDF::NcVar maxTempVar = calcFile.getVar("MaxTemp");
        netCDF::NcVar minRHVar = calcFile.getVar("MinRH");
        netCDF::NcVar prec24Var = calcFile.getVar("PPT24");
        netCDF::NcVar annAvgPrecVar = calcFile.getVar("AnnAvgPrec");
        netCDF::NcVar snowDayVar = calcFile.getVar("SnowDay");
        netCDF::NcVar windSpeedVar = calcFile.getVar("windSpeed");
        netCDF::NcVar slopeClassVar = calcFile.getVar("slopeClass");

        // Read the data
        yearVar.getVar(&year[0]);
        monthVar.getVar(&month[0]);
        dayVar.getVar(&day[0]);
        hourVar.getVar(&hour[0]);
        julianVar.getVar(&julian[0]);
        latVar.getVar(&lat[0]);
        tempVar.getVar(&temp[0]);
        rhVar.getVar(&rh[0]);
        minTempVar.getVar(&minTemp[0]);
        maxTempVar.getVar(&maxTemp[0]);
        minRHVar.getVar(&minRH[0]);
        prec24Var.getVar(&prec24[0]);
        annAvgPrecVar.getVar(&annAvgPrec[0]);
        snowDayVar.getVar(&snowDay[0]);
        windSpeedVar.getVar(&windSpeed[0]);
        slopeClassVar.getVar(&slopeClass[0]);

        calcFile.close();
        cout << "Done\n"
             << endl;

    }
    catch (netCDF::exceptions::NcException &e)
    {
        cerr << "NetCDF Error while reading " << filename << ": " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    return data;
}

int main()
{
    auto DFM_data = ReadDFMOutput(INPUT_DIR + string("output_dfm.nc"));
    auto NFDRS_data = ReadNFDRSInput(INPUT_DIR + string("input_nfdrs.nc"));

    if ((DFM_data.T / 24) != NFDRS_data.T || DFM_data.N != NFDRS_data.N || DFM_data.M != NFDRS_data.M)
    {
        cerr << "Error: output_dfm.nc and input_nfdrs.nc dimensions don't match." << endl;
        return EXIT_FAILURE;
    }

    const auto& T = DFM_data.T, D = NFDRS_data.T;
    const auto& N = DFM_data.N;
    const auto& M = DFM_data.M;

    // Initialize variables
    vector<int> CummPrecip(N * M, 0);
    vector<int> nConsecutiveSnowDays(N * M, 0);

    // Initialize Live Fuel Moisture models
    vector<LiveFuelMoisture> GsiFM(N * M);
    vector<LiveFuelMoisture> HerbFM(N * M);
    vector<LiveFuelMoisture> WoodyFM(N * M);

    // Output Data
    vector<double> GSI_array(D * N * M);
    vector<double> MCHERB_array(D * N * M);
    vector<double> MCWOOD_array(D * N * M);

    vector<NFDRS4> nfdrs(T * N * M);
    vector<double> fSC(T * N * M), fERC(T * N * M), fBI(T * N * M), fIC(T * N * M), KBDI(T * N * M);

    for (size_t t = 0; t < T; ++t)
    {
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < M; ++j)
            {
                int day = t / 4;
                size_t index = t * N * M + i * M + j;
                size_t day_index = day * N * M + i * M + j;

                // output_dfm.nc variables
                double MC1 = DFM_data.oneHourMedians[index];
                double MC10 = DFM_data.tenHourMedians[index];
                double MC100 = DFM_data.hundredHourMedians[index];
                double MC1000 = DFM_data.thousandHourMedians[index];
                double FuelTemperature = DFM_data.fuelTemperatures[index];
                
                // output_lfm.nc variables
                // double MCWOOD = woodyFM[day_index];
                // double MCHERB = herbaceousFM[day_index];

                // input_calc.nc variables
                double WS = NFDRS_data.windSpeed[index];
                int SlopeClass = NFDRS_data.slopeClass[index];
                // int Hour = hour[index];

                // input_lfm.nc variables
                double PrecipAmt = NFDRS_data.prec24[day_index];
                double MaxTemp = NFDRS_data.maxTemp[day_index];
                double AnnAvgPrec = NFDRS_data.annAvgPrec[day_index];
                int Hour = NFDRS_data.hour[index];

                if (Hour == m_regObsHour)
                {
                    nfdrs[index].iCalcKBDI(PrecipAmt, MaxTemp, nfdrs[index].GetCummPrecip(), nfdrs[index].GetYKBDI(), AnnAvgPrec);
                }
                KBDI[index] = nfdrs[index].GetYKBDI();
                nfdrs[index].iSetFuelMoistures(MC1, MC10, MC100, MC1000, MCWOOD, MCHERB, FuelTemperature);
	            nfdrs[index].iCalcIndexes((int)WS, SlopeClass, &fSC[index], &fERC[index], &fBI[index], &fIC[index]);   
            }
        }
    }

    // Write outputs to a NetCDF file or process further

    return EXIT_SUCCESS;
}
