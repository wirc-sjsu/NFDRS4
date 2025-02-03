#include <iostream>
#include <netcdf>
#include <vector>
#include <nfdrs4.h>

#include "timer.h"
#include "args.hxx"

using namespace std;

constexpr int NO_DATA = -1;
constexpr const char *DEFAULT_INPUT_NFDRS = "../../data/input_nfdrs.nc";
constexpr const char *DEFAULT_OUTPUT_NFDRS = "../../data/output_nfdrs.nc";

struct StaticNFDRSData
{
    size_t N, M;
    vector<int> isBurnable;
    vector<double> lat, annAvgPrec;
    vector<int> fModels, slopeClass;

    explicit StaticNFDRSData(size_t N, size_t M)
        : N(N), M(M),
          isBurnable(N * M),
          lat(N * M),
          annAvgPrec(N * M),
          fModels(N * M),
          slopeClass(N * M) {}
};

StaticNFDRSData ReadStaticNFDRS(const string &input_nfdrs)
{
    try
    {
        netCDF::NcFile nfdrs(input_nfdrs, netCDF::NcFile::read);

        // Get dimensions
        netCDF::NcDim southNorthDim = nfdrs.getDim("south_north");
        netCDF::NcDim westEastDim = nfdrs.getDim("west_east");
        size_t N = southNorthDim.getSize();
        size_t M = westEastDim.getSize();

        // Create StaticNFDRSData object
        StaticNFDRSData data(N, M);

        // Read static variables
        netCDF::NcVar isBurnableVar = nfdrs.getVar("isBurnable");
        netCDF::NcVar latVar = nfdrs.getVar("Latitude");
        netCDF::NcVar annAvgPrecVar = nfdrs.getVar("AnnAvgPPT");
        netCDF::NcVar fModelsVar = nfdrs.getVar("FuelModel");
        netCDF::NcVar slopeClassVar = nfdrs.getVar("SlopeClass");

        isBurnableVar.getVar(&data.isBurnable[0]);
        latVar.getVar(&data.lat[0]);
        annAvgPrecVar.getVar(&data.annAvgPrec[0]);
        fModelsVar.getVar(&data.fModels[0]);
        slopeClassVar.getVar(&data.slopeClass[0]);

        nfdrs.close();

        return data;
    }
    catch (const netCDF::exceptions::NcException &e)
    {
        cerr << "NetCDF Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

struct DynamicNFDRSData
{
    size_t N, M;
    int year, month, day, hour;
    vector<double> temp, rh, ppt, windSpeed, sr;
    vector<int> snowDay;
    vector<double> MC1, MC10, MC100, MC1000, fuelTemp;

    explicit DynamicNFDRSData(size_t N, size_t M, bool runDFM = false)
        : N(N), M(M),
          year(0), month(0), day(0), hour(0),
          temp(N * M), rh(N * M), ppt(N * M),
          windSpeed(N * M), sr(N * M),
          snowDay(N * M)
    {
        if (!runDFM)
        {
            MC1.resize(N * M);
            MC10.resize(N * M);
            MC100.resize(N * M);
            MC1000.resize(N * M);
            fuelTemp.resize(N * M);
        }
    }
};

DynamicNFDRSData ReadDynamicNFDRS(
    const string &input_nfdrs, const string &output_dfm,
    size_t N, size_t M, size_t t, bool runDFM)
{
    try
    {
        netCDF::NcFile nfdrs(input_nfdrs, netCDF::NcFile::read);

        // Load 1D variables for timestep t
        vector<size_t> start1 = {t};
        vector<size_t> count1 = {1};

        // Load 3D variables for timestep t
        vector<size_t> start = {t, 0, 0};
        vector<size_t> count = {1, N, M};

        // Create DynamicNFDRSData object
        DynamicNFDRSData data(N, M, runDFM);

        // Dynamic variables
        netCDF::NcVar yearVar = nfdrs.getVar("Year");
        netCDF::NcVar monthVar = nfdrs.getVar("Month");
        netCDF::NcVar dayVar = nfdrs.getVar("Day");
        netCDF::NcVar hourVar = nfdrs.getVar("Hour");
        netCDF::NcVar tempVar = nfdrs.getVar("Temp");
        netCDF::NcVar rhVar = nfdrs.getVar("RH");
        netCDF::NcVar pptVar = nfdrs.getVar("PPT");
        netCDF::NcVar snowDayVar = nfdrs.getVar("SnowDay");
        netCDF::NcVar windSpeedVar = nfdrs.getVar("WindSpeed");
        netCDF::NcVar srVar = nfdrs.getVar("SR");

        yearVar.getVar(start1, count1, &data.year);
        monthVar.getVar(start1, count1, &data.month);
        dayVar.getVar(start1, count1, &data.day);
        hourVar.getVar(start1, count1, &data.hour);

        tempVar.getVar(start, count, &data.temp[0]);
        rhVar.getVar(start, count, &data.rh[0]);
        pptVar.getVar(start, count, &data.ppt[0]);
        snowDayVar.getVar(start, count, &data.snowDay[0]);
        windSpeedVar.getVar(start, count, &data.windSpeed[0]);
        srVar.getVar(start, count, &data.sr[0]);

        nfdrs.close();

        if (!runDFM)
        {
            netCDF::NcFile dfm(output_dfm, netCDF::NcFile::read);
            // DFM variables
            netCDF::NcVar mc1Var = dfm.getVar("MC1");
            netCDF::NcVar mc10Var = dfm.getVar("MC10");
            netCDF::NcVar mc100Var = dfm.getVar("MC100");
            netCDF::NcVar mc1000Var = dfm.getVar("MC1000");
            netCDF::NcVar fuelTempVar = dfm.getVar("FuelTemp");

            mc1Var.getVar(start, count, &data.MC1[0]);
            mc10Var.getVar(start, count, &data.MC10[0]);
            mc100Var.getVar(start, count, &data.MC100[0]);
            mc1000Var.getVar(start, count, &data.MC1000[0]);
            fuelTempVar.getVar(start, count, &data.fuelTemp[0]);

            dfm.close();
        }

        return data;
    }
    catch (const netCDF::exceptions::NcException &e)
    {
        cerr << "NetCDF Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    args::ArgumentParser parser("NFDRS4 Spatial CLI", "By WIRC-SJSU.");
    args::HelpFlag help(parser, "help", "Display help menu", {'h', "help"});
    args::ValueFlag<string> inputNFDRS(parser, "path", "Input NFDRS4 file path", {'i', "input-nfdrs-path"});
    args::ValueFlag<string> outputNFDRS(parser, "path", "Output NFDRS4 file path", {'o', "output-nfdrs-path"});
    args::ValueFlag<string> outputDFM(parser, "path", "DFM file path", {'d', "dfm-path"});

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help &)
    {
        cout << parser;
        return EXIT_SUCCESS;
    }
    catch (const args::ParseError &e)
    {
        cerr << e.what() << endl;
        cerr << parser;
        return EXIT_FAILURE;
    }

    bool runDFM = !outputDFM;

    if (runDFM)
    {
        cout << "Running DFM \n"
             << endl;
    }
    else
    {
        cout << "Not running DFM \n"
             << endl;
    }

    // Determine input file path
    string input_nfdrs = inputNFDRS ? args::get(inputNFDRS) : DEFAULT_INPUT_NFDRS;
    string output_nfdrs = outputNFDRS ? args::get(outputNFDRS) : DEFAULT_OUTPUT_NFDRS;
    string output_dfm = outputDFM ? args::get(outputDFM) : "NO FILE PROVIDED";

    cout << "Input NFDRS file path " << input_nfdrs << endl;
    cout << "Output NFDRS file path " << output_nfdrs << endl;
    if (outputDFM)
    {
        cout << "DFM file path: " << output_dfm << endl;
    }

    cout << "\nReading static data..." << endl;
    StaticNFDRSData staticData = ReadStaticNFDRS(input_nfdrs);

    size_t N = staticData.N;
    size_t M = staticData.M;
    size_t spatialSize = N * M;

    cout << "Reading dynamic data and processing..." << endl;

    // Get time dimension
    netCDF::NcFile nfdrs(input_nfdrs, netCDF::NcFile::read);
    size_t T = nfdrs.getDim("time").getSize();
    nfdrs.close();

    // T, N, M
    cout << "Time steps: " << T << endl;
    cout << "Spatial size: " << N << " x " << M << endl;

    vector<double> MC1, MC10, MC100, MC1000, fuelTemp;

    if (runDFM)
    {
        // Allocate memory for DFM variables
        MC1.resize(T * spatialSize);
        MC10.resize(T * spatialSize);
        MC100.resize(T * spatialSize);
        MC1000.resize(T * spatialSize);
        fuelTemp.resize(T * spatialSize);
    }

    // Initialize output arrays with NO_DATA
    vector<double> KBDI(T * spatialSize, NO_DATA);
    vector<double> GSI(T * spatialSize, NO_DATA);
    vector<double> MCWOOD(T * spatialSize, NO_DATA);
    vector<double> MCHERB(T * spatialSize, NO_DATA);
    vector<double> SC(T * spatialSize, NO_DATA);
    vector<double> ERC(T * spatialSize, NO_DATA);
    vector<double> BI(T * spatialSize, NO_DATA);
    vector<double> IC(T * spatialSize, NO_DATA);

    // Define Fuel Behaviour Model Mapping
    map<int, char> fModelMap = {
        {1, 'V'},
        {2, 'W'},
        {3, 'X'},
        {4, 'Y'},
        {5, 'Z'}
    };

    // Initialize NFDRS objects for burnable locations
    vector<NFDRS4> NFDRSGrid;
    NFDRSGrid.reserve(spatialSize);
    vector<size_t> burnableIndices;
    for (size_t i = 0; i < spatialSize; ++i)
    {
        if (staticData.isBurnable[i] == 1)
        {
            int fModel = staticData.fModels[i];
            char fModelClass = fModelMap[fModel];
            NFDRSGrid.emplace_back(staticData.lat[i], fModelClass, staticData.slopeClass[i],
                                   staticData.annAvgPrec[i], true, true, false);
            burnableIndices.push_back(i);
        }
    }

    for (size_t t = 0; t < T; ++t)
    {
        Timer timer;
        printf("Timestep: %zu/%zu...\n", t + 1, T);

        // Read dynamic data for timestep t
        DynamicNFDRSData dynamicData = ReadDynamicNFDRS(input_nfdrs, output_dfm, N, M, t, runDFM);

        // Process timestep
        size_t c = 0;
        for (size_t i : burnableIndices)
        {
            size_t idx = i;                    // Spatial index
            size_t tidx = t * spatialSize + i; // Time-space index
            
            if (runDFM)
            {
                // Run DFM on CPU
                NFDRSGrid[c].Update(
                    dynamicData.year, dynamicData.month, dynamicData.day, dynamicData.hour,
                    dynamicData.temp[idx], dynamicData.rh[idx], dynamicData.ppt[idx],
                    dynamicData.sr[idx], dynamicData.windSpeed[idx], dynamicData.snowDay[idx]);

                // Save DFM variables MC1, MC10, MC100, MC1000, FuelTemp
                MC1[tidx] = NFDRSGrid[c].MC1;
                MC10[tidx] = NFDRSGrid[c].MC10;
                MC100[tidx] = NFDRSGrid[c].MC100;
                MC1000[tidx] = NFDRSGrid[c].MC1000;
                fuelTemp[tidx] = NFDRSGrid[c].FuelTemperature;
            }
            else
            {
                // DFM already ran on GPU - use result from output_dfm
                NFDRSGrid[c].Update(
                    dynamicData.year, dynamicData.month, dynamicData.day, dynamicData.hour,
                    dynamicData.temp[idx], dynamicData.rh[idx], dynamicData.ppt[idx],
                    dynamicData.windSpeed[idx], dynamicData.snowDay[idx],
                    dynamicData.MC1[idx], dynamicData.MC10[idx], dynamicData.MC100[idx],
                    dynamicData.MC1000[idx], dynamicData.fuelTemp[idx]);
            }

            // Save results to output arrays
            KBDI[tidx] = NFDRSGrid[c].KBDI;
            GSI[tidx] = NFDRSGrid[c].m_GSI;
            MCWOOD[tidx] = NFDRSGrid[c].MCWOOD;
            MCHERB[tidx] = NFDRSGrid[c].MCHERB;
            SC[tidx] = NFDRSGrid[c].SC;
            ERC[tidx] = NFDRSGrid[c].ERC;
            BI[tidx] = NFDRSGrid[c].BI;
            IC[tidx] = NFDRSGrid[c].IC;
            c++;
        }
        printf("Done.\n");
    }

    // Write results to NetCDF file
    netCDF::NcFile outputFile(output_nfdrs, netCDF::NcFile::replace);

    auto timeDim = outputFile.addDim("time", T);
    auto southNorthDim = outputFile.addDim("south_north", N);
    auto westEastDim = outputFile.addDim("west_east", M);

    auto KBDIData = outputFile.addVar("KBDI", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto GSIData = outputFile.addVar("GSI", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto MCWOODData = outputFile.addVar("MCWOOD", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto MCHERBData = outputFile.addVar("MCHERB", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto SCData = outputFile.addVar("SC", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto ERCData = outputFile.addVar("ERC", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto BIData = outputFile.addVar("BI", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
    auto ICData = outputFile.addVar("IC", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});

    // Write MC1, MC10, MC100, MC1000, FuelTemp to netcdf file
    if(runDFM)
    {
        auto MC1Data = outputFile.addVar("MC1", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
        auto MC10Data = outputFile.addVar("MC10", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
        auto MC100Data = outputFile.addVar("MC100", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
        auto MC1000Data = outputFile.addVar("MC1000", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});
        auto fuelTempData = outputFile.addVar("FuelTemp", netCDF::ncDouble, {timeDim, southNorthDim, westEastDim});

        MC1Data.putVar(&MC1[0]);
        MC10Data.putVar(&MC10[0]);
        MC100Data.putVar(&MC100[0]);
        MC1000Data.putVar(&MC1000[0]);
        fuelTempData.putVar(&fuelTemp[0]);
    }

    KBDIData.putVar(&KBDI[0]);
    GSIData.putVar(&GSI[0]);
    MCWOODData.putVar(&MCWOOD[0]);
    MCHERBData.putVar(&MCHERB[0]);
    SCData.putVar(&SC[0]);
    ERCData.putVar(&ERC[0]);
    BIData.putVar(&BI[0]);
    ICData.putVar(&IC[0]);

    outputFile.close();

    return EXIT_SUCCESS;
}