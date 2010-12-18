#if !defined(__AudioHardware_h__)
#define __AudioHardware_h__

#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CoreFoundation.h>

#if defined(__cplusplus)
extern "C"
{
#endif

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

typedef UInt32      AudioObjectID;
typedef AudioObjectID   AudioDeviceID;
typedef short SInt16;
typedef float Float32;

typedef UInt32 AudioHardwarePropertyID;
typedef UInt32 AudioDevicePropertyID;

extern OSStatus
AudioHardwareGetPropertyInfo(   AudioHardwarePropertyID inPropertyID,
                                UInt32*                 outSize,
                                Boolean*                outWritable);

extern OSStatus
AudioHardwareGetProperty(   AudioHardwarePropertyID inPropertyID,
                            UInt32*                 ioPropertyDataSize,
                            void*                   outPropertyData);

extern OSStatus
AudioDeviceGetPropertyInfo( AudioDeviceID           inDevice,
                            UInt32                  inChannel,
                            Boolean                 isInput,
                            AudioDevicePropertyID   inPropertyID,
                            UInt32*                 outSize,
                            Boolean*                outWritable);

extern OSStatus
AudioDeviceGetProperty( AudioDeviceID           inDevice,
                        UInt32                  inChannel,
                        Boolean                 isInput,
                        AudioDevicePropertyID   inPropertyID,
                        UInt32*                 ioPropertyDataSize,
                        void*                   outPropertyData);

extern OSStatus
AudioDeviceSetProperty( AudioDeviceID           inDevice,
                        const AudioTimeStamp*   inWhen,
                        UInt32                  inChannel,
                        Boolean                 isInput,
                        AudioDevicePropertyID   inPropertyID,
                        UInt32                  inPropertyDataSize,
                        const void*             inPropertyData);

typedef OSStatus
(*AudioDeviceIOProc)(   AudioDeviceID           inDevice,
                        const AudioTimeStamp*   inNow,
                        const AudioBufferList*  inInputData,
                        const AudioTimeStamp*   inInputTime,
                        AudioBufferList*        outOutputData,
                        const AudioTimeStamp*   inOutputTime,
                        void*                   inClientData);

#if defined(__cplusplus)
}
#endif

#endif __AudioHardware_h__
