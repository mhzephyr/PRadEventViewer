#ifndef PRAD_DATA_HANDLER_H
#define PRAD_DATA_HANDLER_H

// a hash map reduce nearly half of the time to process events data, 
// unfortunately it is only supported by standard library in C++11
// comment hash map definition if the compiler does not support C++11
#define  USE_HASH_MAP

// Multi threads to deal with several banks in an event simultaneously.
// Depends on the running evironment, you may have greatly improved
// performance to read large evio file.
//#define MULTI_THREAD

#ifdef USE_HASH_MAP
#include <unordered_map>
#else
#include <map>
#endif

#ifdef MULTI_THREAD
#include <thread>
#include <mutex>
#endif

#include <deque>
#include <vector>
#include "datastruct.h"

using namespace std;

class PRadEvioParser;
class HyCalModule;
class TH1D;


#ifdef USE_HASH_MAP
// a simple hash function for DAQ configuration
namespace std {
    template <>
    struct hash<CrateConfig>
    {
        size_t operator()(const CrateConfig& cfg) const
        {
            // crate id is 1-6, slot is 1-26, channel is 0-63
            // thus they can be filled in a 16 bit word
            // [ 0 0 0 | 0 0 0 0 0 | 0 0 0 0 0 0 0 0 ]
            // [ crate |    slot   |     channel     ]
            // this simple hash ensures no collision at current setup
            // which means fast access
            return (cfg.crate << 13 | cfg.slot << 8 | cfg.channel);
        }
   };
}

typedef unordered_map< CrateConfig, HyCalModule* >::iterator daq_iter;
typedef unordered_map< string, HyCalModule* >::iterator name_iter;
typedef unordered_map< int, vector< HyCalModule* > >::iterator tdc_iter;
#else
typedef map< CrateConfig, HyCalModule* >::iterator daq_iter;
typedef map< string, HyCalModule* >::iterator name_iter;
typedef map< int, vector< HyCalModule* > >::iterator tdc_iter;
#endif

class PRadDataHandler
{
public:
    typedef struct {
        unsigned short id;
        unsigned short adcValue;
    } ModuleEnergyData;

    PRadDataHandler();
    virtual ~PRadDataHandler();
    void RegisterModule(HyCalModule *hyCalModule);
    void FeedData(ADC1881MData &adcData);
    void FeedData(GEMAPVData &gemData);
    void FeedData(CAENHVData &hvData);
    void ShowEvent(int idx = 0);
    int GetEventCount() {return energyData.size();};
    TH1D *GetEnergyHist() {return energyHist;};
    void Clear();
    void EndofThisEvent();
    void OnlineMode() {onlineMode = true;};
    void OfflineMode() {onlineMode = false;};
    void BuildModuleMap();
    HyCalModule *FindModule(CrateConfig &daqInfo);
    HyCalModule *FindModule(const unsigned short &id);
    vector< HyCalModule* > GetTDCGroup(int &id);
    vector< int > GetTDCGroupIDList();
    const vector< HyCalModule* > &GetModuleList() {return moduleList;};

private:
    double totalE;
    bool onlineMode;
#ifdef MULTI_THREAD
    mutex myLock;
#endif

#ifdef USE_HASH_MAP
    unordered_map< CrateConfig, HyCalModule* > map_daq;
    unordered_map< string, HyCalModule* > map_name;
    unordered_map< int, vector< HyCalModule* > > map_tdc;
#else
    map< CrateConfig, HyCalModule* > map_daq;
    map< string, HyCalModule* > map_name;
    map< int, vector< HyCalModule* > > map_tdc;
#endif
    vector< HyCalModule* > moduleList;
    deque< vector< ModuleEnergyData > > energyData;
    vector < ModuleEnergyData > newEvent, lastEvent;
    TH1D *energyHist;
};

#endif
