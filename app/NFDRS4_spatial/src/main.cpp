#include <iostream>
#include <netcdf>

using namespace std;

constexpr const int NO_DATA = -9999;
constexpr const char *INPUT_FILE = "../data/input_nfdrs.nc";
constexpr const char *INPUT_DFM_FILE = "../data/input_nfdrs.nc";
constexpr const char *OUTPUT_DFM_FILE = "../data/input_dfm.nc";
constexpr const char *OUTPUT_FILE = "../data/output_nfdrs.nc";
constexpr const char *CUSTOM_FM_FILE = "../data/custom_fuel_models.csv";

class GridNFDRSData
{
    size_t T, N, M; // T timesteps, NxM Grid
    vector<int> year, month, day, hour;
    vector<double> lat, annAvgPrec;
    vector<int> fModels, slopeClass;
    vector<double> temp, rh, ppt;
    vector<bool> snowDay;
    vector<double> windSpeed;
    vector<bool> isBurnable;

public:
    explicit GridNFDRSData(size_t T, size_t N, size_t M)
        : T(T), N(N), M(M),
          year(T), month(T), day(T), hour(T),
          lat(N * M), fModels(N * M), slopeClass(N * M),
          annAvgPrec(N * M), isBurnable(N * M),
          temp(T * N * M), rh(T * N * M), ppt(T * N * M),
          snowDay(T * N * M), windSpeed(T * N * M)
    {
    }
};

// Read NetCDF File
GridNFDRSData ReadNetCDF(const string &filename = INPUT_FILE)
{
    try
    {
        // Open the NetCDF file
        netCDF::NcFile ncFile(filename, netCDF::NcFile::read);

        // Get the dimensions
        netCDF::NcDim timeDim = ncFile.getDim("time");
        netCDF::NcDim southNorthDim = ncFile.getDim("south_north");
        netCDF::NcDim westEastDim = ncFile.getDim("west_east");

        GridNFDRSData data(timeDim.getSize(), southNorthDim.getSize(), westEastDim.getSize());

        // Get the variables
        netCDF::NcVar yearVar = ncFile.getVar("Year");
        netCDF::NcVar monthVar = ncFile.getVar("Month");
        netCDF::NcVar dayVar = ncFile.getVar("Day");
        netCDF::NcVar hourVar = ncFile.getVar("Hour");
        netCDF::NcVar tVar = ncFile.getVar("Temp");
        netCDF::NcVar rhVar = ncFile.getVar("RH");
        netCDF::NcVar srVar = ncFile.getVar("SR");
        netCDF::NcVar pptVar = ncFile.getVar("PPT");

        // Read the data from the variables into the vectors
        yearVar.getVar(&data.year[0]);
        monthVar.getVar(&data.month[0]);
        dayVar.getVar(&data.day[0]);
        hourVar.getVar(&data.hour[0]);
        tVar.getVar(&data.temp[0]);
        rhVar.getVar(&data.rh[0]);
        srVar.getVar(&data.sr[0]);
        pptVar.getVar(&data.ppt[0]);

        // Close the NetCDF file
        ncFile.close();

        return data;
    }
    catch (netCDF::exceptions::NcException &e)
    {
        cerr << "NetCDF Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

int main()
{
    // TODO: read CSV file CUSTOM_FM_FILE
    // TODO: create CFuelModelParams with entries in the CSV file
    // TODO: use NFDRS4::AddCustomFuel to add the custom fuel

    cout << "Hello, World!" << endl;
    auto data = ReadNetCDF();
    return 0;
}