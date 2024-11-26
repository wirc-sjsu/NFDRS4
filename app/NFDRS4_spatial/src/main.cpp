#include <iostream>
#include <netcdf>

using namespace std;

constexpr const int NO_DATA = -9999;
constexpr const char *INPUT_FILE = "../data/input_nfdrs.nc";
constexpr const char *OUTPUT_DFM_FILE = "../data/output_dfm.nc";
constexpr const char *OUTPUT_FILE = "../data/output_nfdrs.nc";
constexpr const char *CUSTOM_FM_FILE = "../data/custom_fuel_models.csv";

class GridNFDRSData
{
    size_t T, N, M; // T timesteps, NxM Grid
    vector<int> year, month, day, hour;
    vector<double> lat;
    vector<int> fModels, slopeClass;
    vector<double> temp, rh, ppt;
    vector<bool> snowDay;
    vector<double> windSpeed;
    vector<bool> isBurnable;

public:
    explicit GridNFDRSData(size_t T, size_t N, size_t M)
        : T(T), N(N), M(M),
          year(T), month(T), day(T), hour(T),
          lat(N), fModels(N), slopeClass(N),
          temp(T * N * M), rh(T * N * M), ppt(T * N * M),
          snowDay(T * N * M), windSpeed(T * N * M),
          isBurnable(N * M)
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

        GridDFMData data(timeDim.getSize(), southNorthDim.getSize(), westEastDim.getSize());

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
        yearVar.getVar(&data.yearData[0]);
        monthVar.getVar(&data.monthData[0]);
        dayVar.getVar(&data.dayData[0]);
        hourVar.getVar(&data.hourData[0]);
        tVar.getVar(&data.tempData[0]);
        rhVar.getVar(&data.rhData[0]);
        srVar.getVar(&data.srData[0]);
        pptVar.getVar(&data.pptData[0]);

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