/*
 *      Copyright (C) 2005-2009 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef __COREAUDIO_H__
#define __COREAUDIO_H__

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioServices.h>
#include "AudioHardware.h">
#include <StdString.h>
#include <list>
#include <vector>

#if defined(__arm__)
/*
enum
{
    kAudioHardwarePropertyProcessIsMaster                   = 'mast',
    kAudioHardwarePropertyIsInitingOrExiting                = 'inot',
    kAudioHardwarePropertyDevices                           = 'dev#',
    kAudioHardwarePropertyDefaultInputDevice                = 'dIn ',
    kAudioHardwarePropertyDefaultOutputDevice               = 'dOut',
    kAudioHardwarePropertyDefaultSystemOutputDevice         = 'sOut',
    kAudioHardwarePropertyDeviceForUID                      = 'duid',
    kAudioHardwarePropertySleepingIsAllowed                 = 'slep',
    kAudioHardwarePropertyUnloadingIsAllowed                = 'unld',
    kAudioHardwarePropertyHogModeIsAllowed                  = 'hogr',
    kAudioHardwarePropertyRunLoop                           = 'rnlp',
    kAudioHardwarePropertyPlugInForBundleID                 = 'pibi'
};
enum
{
    kAudioObjectPropertyClass               = 'clas',
    kAudioObjectPropertyOwner               = 'stdv',
    kAudioObjectPropertyCreator             = 'oplg',
    kAudioObjectPropertyName                = 'lnam',
    kAudioObjectPropertyManufacturer        = 'lmak',
    kAudioObjectPropertyElementName         = 'lchn',
    kAudioObjectPropertyElementCategoryName = 'lccn',
    kAudioObjectPropertyElementNumberName   = 'lcnn',
    kAudioObjectPropertyOwnedObjects        = 'ownd',
    kAudioObjectPropertyListenerAdded       = 'lisa',
    kAudioObjectPropertyListenerRemoved     = 'lisr'
};


enum
{
    kAudioDevicePropertyDeviceName                          = 'name',
    kAudioDevicePropertyDeviceNameCFString                  = kAudioObjectPropertyName,
    kAudioDevicePropertyDeviceManufacturer                  = 'makr',
    kAudioDevicePropertyDeviceManufacturerCFString          = kAudioObjectPropertyManufacturer,
    kAudioDevicePropertyRegisterBufferList                  = 'rbuf',
    kAudioDevicePropertyBufferSize                          = 'bsiz',
    kAudioDevicePropertyBufferSizeRange                     = 'bsz#',
    kAudioDevicePropertyChannelName                         = 'chnm',
    kAudioDevicePropertyChannelNameCFString                 = kAudioObjectPropertyElementName,
    kAudioDevicePropertyChannelCategoryName                 = 'ccnm',
    kAudioDevicePropertyChannelCategoryNameCFString         = kAudioObjectPropertyElementCategoryName,
    kAudioDevicePropertyChannelNumberName                   = 'cnnm',
    kAudioDevicePropertyChannelNumberNameCFString           = kAudioObjectPropertyElementNumberName,
    kAudioDevicePropertySupportsMixing                      = 'mix?',
    kAudioDevicePropertyStreamFormat                        = 'sfmt',
    kAudioDevicePropertyStreamFormats                       = 'sfm#',
    kAudioDevicePropertyStreamFormatSupported               = 'sfm?',
    kAudioDevicePropertyStreamFormatMatch                   = 'sfmm',
    kAudioDevicePropertyDataSourceNameForID                 = 'sscn',
    kAudioDevicePropertyClockSourceNameForID                = 'cscn',
    kAudioDevicePropertyPlayThruDestinationNameForID        = 'mddn',
    kAudioDevicePropertyChannelNominalLineLevelNameForID    = 'cnlv'
};





enum
{
    kAudioDevicePropertyPlugIn                          = 'plug',
    kAudioDevicePropertyConfigurationApplication        = 'capp',
    kAudioDevicePropertyDeviceUID                       = 'uid ',
    kAudioDevicePropertyModelUID                        = 'muid',
    kAudioDevicePropertyTransportType                   = 'tran',
    kAudioDevicePropertyRelatedDevices                  = 'akin',
    kAudioDevicePropertyClockDomain                     = 'clkd',
    kAudioDevicePropertyDeviceIsAlive                   = 'livn',
    kAudioDevicePropertyDeviceHasChanged                = 'diff',
    kAudioDevicePropertyDeviceIsRunning                 = 'goin',
    kAudioDevicePropertyDeviceIsRunningSomewhere        = 'gone',
    kAudioDevicePropertyDeviceCanBeDefaultDevice        = 'dflt',
    kAudioDevicePropertyDeviceCanBeDefaultSystemDevice  = 'sflt',
    kAudioDeviceProcessorOverload                       = 'over',
    kAudioDevicePropertyHogMode                         = 'oink',
    kAudioDevicePropertyLatency                         = 'ltnc',
    kAudioDevicePropertyBufferFrameSize                 = 'fsiz',
    kAudioDevicePropertyBufferFrameSizeRange            = 'fsz#',
    kAudioDevicePropertyUsesVariableBufferFrameSizes    = 'vfsz',
    kAudioDevicePropertyStreams                         = 'stm#',
    kAudioDevicePropertySafetyOffset                    = 'saft',
    kAudioDevicePropertyIOCycleUsage                    = 'ncyc',
    kAudioDevicePropertyStreamConfiguration             = 'slay',
    kAudioDevicePropertyIOProcStreamUsage               = 'suse',
    kAudioDevicePropertyPreferredChannelsForStereo      = 'dch2',
    kAudioDevicePropertyPreferredChannelLayout          = 'srnd',
    kAudioDevicePropertyNominalSampleRate               = 'nsrt',
    kAudioDevicePropertyAvailableNominalSampleRates     = 'nsr#',
    kAudioDevicePropertyActualSampleRate                = 'asrt'
};

typedef short SInt16;
typedef float Float32;
*/
#endif

//#if !defined(__arm__)
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioServices.h>
#include <StdString.h>
#include <list>
#include <vector>

enum {
  kAudioUnitSubType_HALOutput           = 'ahal',
  kAudioUnitSubType_DefaultOutput       = 'def ',
  kAudioUnitSubType_SystemOutput        = 'sys ',
};
enum {
  kAudioUnitSubType_StereoMixer            = 'smxr',
  kAudioUnitSubType_3DMixer                = '3dmx',
  kAudioUnitSubType_MatrixMixer            = 'mxmx',
};

typedef UInt32      AudioObjectID;
typedef AudioObjectID   AudioDeviceID;
typedef AudioObjectID   AudioStreamID;
/*
struct  AudioStreamRangedDescription
{
    AudioStreamBasicDescription     mFormat;
    AudioValueRange                 mSampleRateRange;
};
typedef struct AudioStreamRangedDescription AudioStreamRangedDescription;
*/
typedef OSStatus
(*AudioDeviceIOProc)(   AudioDeviceID           inDevice,
                        const AudioTimeStamp*   inNow,
                        const AudioBufferList*  inInputData,
                        const AudioTimeStamp*   inInputTime,
                        AudioBufferList*        outOutputData,
                        const AudioTimeStamp*   inOutputTime,
                        void*                   inClientData);
struct ComponentDescription {
  OSType              componentType;          /* A unique 4-byte code indentifying the command set */
  OSType              componentSubType;       /* Particular flavor of this instance */
  OSType              componentManufacturer;  /* Vendor indentification */
  unsigned long       componentFlags;         /* 8 each for Component,Type,SubType,Manuf/revision */
  unsigned long       componentFlagsMask;     /* Mask for specifying which flags to consider in search, zero during registration */
};
typedef struct ComponentDescription     ComponentDescription;

// Forward declarations
class CIOSCoreAudioHardware;
class CIOSCoreAudioDevice;
class CIOSCoreAudioStream;
class CIOSCoreAudioUnit;

typedef std::list<AudioDeviceID> IOSCoreAudioDeviceList;

// Not yet implemented
// kAudioHardwarePropertyDevices                     
// kAudioHardwarePropertyDefaultInputDevice              
// kAudioHardwarePropertyDefaultSystemOutputDevice   
// kAudioHardwarePropertyDeviceForUID                          

// There is only one AudioSystemObject instance system-side.
// Therefore, all CIOSCoreAudioHardware methods are static
class CIOSCoreAudioHardware
{
public:
  static AudioDeviceID FindAudioDevice(CStdString deviceName);
  static AudioDeviceID GetDefaultOutputDevice();
  static UInt32 GetOutputDevices(IOSCoreAudioDeviceList* pList);
  static bool GetAutoHogMode();
  static void SetAutoHogMode(bool enable);
};

// Not yet implemented
// kAudioDevicePropertyDeviceIsRunning, kAudioDevicePropertyDeviceIsRunningSomewhere, kAudioDevicePropertyLatency, 
// kAudioDevicePropertyBufferFrameSize, kAudioDevicePropertyBufferFrameSizeRange, kAudioDevicePropertyUsesVariableBufferFrameSizes,
// kAudioDevicePropertySafetyOffset, kAudioDevicePropertyIOCycleUsage, kAudioDevicePropertyStreamConfiguration
// kAudioDevicePropertyIOProcStreamUsage, kAudioDevicePropertyPreferredChannelsForStereo, kAudioDevicePropertyPreferredChannelLayout,
// kAudioDevicePropertyAvailableNominalSampleRates, kAudioDevicePropertyActualSampleRate,
// kAudioDevicePropertyTransportType

typedef std::list<AudioStreamID> AudioStreamIdList;
typedef std::vector<SInt32> CoreAudioChannelList;
typedef std::list<UInt32> CoreAudioDataSourceList;

class CIOSCoreAudioDevice
{
public:
  CIOSCoreAudioDevice();
  CIOSCoreAudioDevice(AudioDeviceID deviceId);
  virtual ~CIOSCoreAudioDevice();
  
  bool Open(AudioDeviceID deviceId);
  void Close();
  
  void Start();
  void Stop();
  bool AddIOProc(AudioDeviceIOProc ioProc, void* pCallbackData);
  void RemoveIOProc();
  
  AudioDeviceID GetId() {return m_DeviceId;}
  const char* GetName(CStdString& name);
  UInt32 GetTotalOutputChannels();
  bool GetStreams(AudioStreamIdList* pList);
  bool IsRunning();
  bool SetHogStatus(bool hog);
  pid_t GetHogStatus();
  bool SetMixingSupport(bool mix);
  bool GetMixingSupport();
  bool GetPreferredChannelLayout(CoreAudioChannelList* pChannelMap);
  bool GetDataSources(CoreAudioDataSourceList* pList);
  Float64 GetNominalSampleRate();
  bool SetNominalSampleRate(Float64 sampleRate);
  UInt32 GetNumLatencyFrames();
protected:
  AudioDeviceID m_DeviceId;
  bool m_Started;
  pid_t m_Hog;
  int m_MixerRestore;
  AudioDeviceIOProc m_IoProc;
  Float64 m_SampleRateRestore;
};

typedef std::list<AudioStreamRangedDescription> StreamFormatList;

class CIOSCoreAudioStream
{
public:
  CIOSCoreAudioStream();
  virtual ~CIOSCoreAudioStream();
  
  bool Open(AudioStreamID streamId);
  void Close();
  
  AudioStreamID GetId() {return m_StreamId;}
  UInt32 GetDirection();
  UInt32 GetTerminalType();
  UInt32 GetNumLatencyFrames();
  bool GetVirtualFormat(AudioStreamBasicDescription* pDesc);
  bool GetPhysicalFormat(AudioStreamBasicDescription* pDesc);
  bool SetVirtualFormat(AudioStreamBasicDescription* pDesc);
  bool SetPhysicalFormat(AudioStreamBasicDescription* pDesc);
  bool GetAvailableVirtualFormats(StreamFormatList* pList);
  bool GetAvailablePhysicalFormats(StreamFormatList* pList);

protected:
  AudioStreamID m_StreamId;
  AudioStreamBasicDescription m_OriginalVirtualFormat;  
  AudioStreamBasicDescription m_OriginalPhysicalFormat;  
};

class CIOSCoreAudioUnit
{
public:
  CIOSCoreAudioUnit();
  virtual ~CIOSCoreAudioUnit();
  
  bool Open(AudioComponentDescription desc);
  bool Open(OSType type, OSType subType, OSType manufacturer);
  void Attach(AudioUnit audioUnit) {m_AudioUnit = audioUnit;}
  AudioComponentInstance GetComponent(){return m_AudioUnit;}
  void Close();
  bool Initialize();
  bool IsInitialized() {return m_Initialized;}
  bool SetRenderProc(AURenderCallback callback, void* pClientData);
  bool GetInputFormat(AudioStreamBasicDescription* pDesc);
  bool GetOutputFormat(AudioStreamBasicDescription* pDesc);    
  bool SetInputFormat(AudioStreamBasicDescription* pDesc);
  bool SetOutputFormat(AudioStreamBasicDescription* pDesc);
  bool SetMaxFramesPerSlice(UInt32 maxFrames);
protected:
  AudioComponentInstance m_AudioUnit;
  bool m_Initialized;
};

class CIOSAUOutputDevice : public CIOSCoreAudioUnit
{
public:
  CIOSAUOutputDevice();
  virtual ~CIOSAUOutputDevice();
  bool SetCurrentDevice(AudioDeviceID deviceId);
  bool GetInputChannelMap(CoreAudioChannelList* pChannelMap);
  bool SetInputChannelMap(CoreAudioChannelList* pChannelMap);
  UInt32 GetBufferFrameSize();
  
  void Start();
  void Stop();
  bool IsRunning();

  Float32 GetCurrentVolume();
  bool SetCurrentVolume(Float32 vol);  
protected:
};

class CIOSAUMatrixMixer : public CIOSCoreAudioUnit
{
public:
  CIOSAUMatrixMixer();
  virtual ~CIOSAUMatrixMixer();
protected:
};

// Helper Functions
char* IOSUInt32ToFourCC(UInt32* val);
const char* IOSStreamDescriptionToString(AudioStreamBasicDescription desc, CStdString& str);

#define CONVERT_OSSTATUS(x) IOSUInt32ToFourCC((UInt32*)&ret)

#endif // __COREAUDIO_H__
