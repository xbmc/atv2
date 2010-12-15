#ifdef __APPLE__
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
//#if !defined(__arm__)

#include "IOSCoreAudio.h"
#include <PlatformDefs.h>
#include <Log.h>
#include <math.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

char* IOSUInt32ToFourCC(UInt32* pVal) // NOT NULL TERMINATED! Modifies input value.
{
  UInt32 inVal = *pVal;
  char* pIn = (char*)&inVal;
  char* fourCC = (char*)pVal;
  fourCC[3] = pIn[0];
  fourCC[2] = pIn[1];
  fourCC[1] = pIn[2];
  fourCC[0] = pIn[3];
  return fourCC;
}

const char* IOSStreamDescriptionToString(AudioStreamBasicDescription desc, CStdString& str)
{
  UInt32 formatId = desc.mFormatID;
  char* fourCC = IOSUInt32ToFourCC(&formatId);
  
  switch (desc.mFormatID)
  {
    case kAudioFormatLinearPCM:
      str.Format("[%4.4s] %s%u Channel %u-bit %s (%uHz)", 
                 fourCC,
                 (desc.mFormatFlags & kAudioFormatFlagIsNonMixable) ? "" : "Mixable ",
                 desc.mChannelsPerFrame,
                 desc.mBitsPerChannel,
                 (desc.mFormatFlags & kAudioFormatFlagIsFloat) ? "Floating Point" : "Signed Integer",
                 (UInt32)desc.mSampleRate);
      break;
    case kAudioFormatAC3:
      str.Format("[%4.4s] AC-3/DTS (%uHz)", fourCC, (UInt32)desc.mSampleRate);
      break;
    case kAudioFormat60958AC3:
      str.Format("[%4.4s] AC-3/DTS for S/PDIF (%uHz)", fourCC, (UInt32)desc.mSampleRate);
      break;
    default:
      str.Format("[%4.4s]", fourCC);
      break;
  }
  return str.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIOSCoreAudioHardware
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AudioDeviceID CIOSCoreAudioHardware::FindAudioDevice(CStdString searchName)
{
  if (!searchName.length())
    return 0;
  
  UInt32 size = 0;
  AudioDeviceID deviceId = 0;
  OSStatus ret;
 
  if (searchName.Equals("Default Output Device"))
  {
    AudioDeviceID defaultDevice = GetDefaultOutputDevice();
    CLog::Log(LOGDEBUG, "CIOSCoreAudioHardware::FindAudioDevice: Returning default device [0x%04x].", (uint32_t)defaultDevice);
    return defaultDevice;  
  }
  CLog::Log(LOGDEBUG, "CIOSCoreAudioHardware::FindAudioDevice: Searching for device - %s.", searchName.c_str());
  
  // Obtain a list of all available audio devices
  AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, NULL);
  UInt32 deviceCount = size / sizeof(AudioDeviceID);
  AudioDeviceID* pDevices = new AudioDeviceID[deviceCount];
  ret = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, pDevices);
  if (ret)
  { 
    CLog::Log(LOGERROR, "CIOSCoreAudioHardware::FindAudioDevice: Unable to retrieve the list of available devices. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    delete[] pDevices;
    return 0; 
  }
  
  // Attempt to locate the requested device
  CStdString deviceName;
  for (UInt32 dev = 0; dev < deviceCount; dev++)
  {
    CIOSCoreAudioDevice device;
    device.Open((pDevices[dev]));
    device.GetName(deviceName);
    UInt32 totalChannels = device.GetTotalOutputChannels();
    CLog::Log(LOGDEBUG, "CIOSCoreAudioHardware::FindAudioDevice:   Device[0x%04x] - Name: '%s', Total Ouput Channels: %u. ", pDevices[dev], deviceName.c_str(), totalChannels);
    if (searchName.Equals(deviceName))
      deviceId = pDevices[dev];
    if (deviceId)
      break;
  }
  delete[] pDevices;  
  
  return deviceId;
}

AudioDeviceID CIOSCoreAudioHardware::GetDefaultOutputDevice()
{
  UInt32 size = sizeof(AudioDeviceID);
  AudioDeviceID deviceId = 0;
  
  OSStatus ret;
  AudioComponentInstance audioUnit;
  
  // Describe audio component
  AudioComponentDescription desc;
  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_RemoteIO;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  
  // Get component
  AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);
  
  // Get audio units
  ret = AudioComponentInstanceNew(inputComponent, &audioUnit);
  
  if (ret) {
    CLog::Log(LOGERROR, "CIOSCoreAudioHardware::GetDefaultOutputDevice: Unable to identify default output device. Error = 0x%08x (%4.4s).", ret, CONVERT_OSSTATUS(ret));
    return 0;
  }
  
  return (AudioDeviceID)audioUnit;
  //return (AudioDeviceID)inputComponent;
  
}

UInt32 CIOSCoreAudioHardware::GetOutputDevices(IOSCoreAudioDeviceList* pList)
{
  if (!pList)
    return 0;
  
  // Obtain a list of all available audio devices
  UInt32 found = 0;
  UInt32 size = 0;
  OSStatus ret;
  
  //kAudioObjectPropertyScopeGlobal

  /*
  AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, NULL);
  UInt32 deviceCount = size / sizeof(AudioDeviceID);
  AudioDeviceID* pDevices = new AudioDeviceID[deviceCount];
  ret = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, pDevices);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioHardware::GetOutputDevices: Unable to retrieve the list of available devices. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
  else
  {
    for (UInt32 dev = 0; dev < deviceCount; dev++)
    {
      CIOSCoreAudioDevice device(pDevices[dev]);
      if (device.GetTotalOutputChannels() == 0)
        continue;
      found++;
      pList->push_back(pDevices[dev]);
    }
  }
  delete[] pDevices;
  return found;
  */
  return 0;
}

bool CIOSCoreAudioHardware::GetAutoHogMode()
{
  UInt32 val = 0;
  UInt32 size = sizeof(val);
  OSStatus ret = AudioHardwareGetProperty(kAudioHardwarePropertyHogModeIsAllowed, &size, &val);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioHardware::GetAutoHogMode: Unable to get auto 'hog' mode. Error = 0x%08x (%4.4s).", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return (val == 1);
}

void CIOSCoreAudioHardware::SetAutoHogMode(bool enable)
{
  UInt32 val = enable ? 1 : 0;
  OSStatus ret = AudioHardwareSetProperty(kAudioHardwarePropertyHogModeIsAllowed, sizeof(val), &val);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioHardware::SetAutoHogMode: Unable to set auto 'hog' mode. Error = 0x%08x (%4.4s).", ret, CONVERT_OSSTATUS(ret));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIOSCoreAudioDevice
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIOSCoreAudioDevice::CIOSCoreAudioDevice()  : 
  m_DeviceId(0),
  m_Started(false),
  m_Hog(-1),
  m_MixerRestore(-1),
  m_IoProc(NULL),
  m_SampleRateRestore(0.0f)
{
  
}

CIOSCoreAudioDevice::CIOSCoreAudioDevice(AudioDeviceID deviceId) : 
  m_DeviceId(deviceId),
  m_Started(false),
  m_Hog(-1),
  m_MixerRestore(-1),
  m_IoProc(NULL),
  m_SampleRateRestore(0.0f)
{
  
}

CIOSCoreAudioDevice::~CIOSCoreAudioDevice()
{
  Close();
}

bool CIOSCoreAudioDevice::Open(AudioDeviceID deviceId)
{
  m_DeviceId = deviceId;
  CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::Open: Opened device 0x%04x", (uint32_t)m_DeviceId);
  return true;
}

void CIOSCoreAudioDevice::Close()
{
  if (!m_DeviceId)
    return;
  
  Stop(); // Stop the device if it was started
  
  RemoveIOProc(); // Unregister the IOProc if we have one
  
  SetHogStatus(false);
  if (m_MixerRestore > -1) // We changed the mixer status
    SetMixingSupport((m_MixerRestore ? true : false));
  m_MixerRestore = -1;
  
  if (m_SampleRateRestore != 0.0f)
  {
    CLog::Log(LOGDEBUG,  "CIOSCoreAudioUnit::Close: Restoring original nominal samplerate.");    
    SetNominalSampleRate(m_SampleRateRestore);
  }
  
  CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::Close: Closed device 0x%04x", m_DeviceId);
  m_DeviceId = 0;
  m_IoProc = NULL;
  
}

void CIOSCoreAudioDevice::Start()
{
  if (!m_DeviceId || m_Started) 
    return;
  
  /*
  OSStatus ret = AudioDeviceStart(m_DeviceId, m_IoProc);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::Start: Unable to start device. Error = 0x%08x (%4.4s).", ret, CONVERT_OSSTATUS(ret));
  else*/
    m_Started = true;
}

void CIOSCoreAudioDevice::Stop()
{
  if (!m_DeviceId || !m_Started)
    return;
  
  /*
  OSStatus ret = AudioDeviceStop(m_DeviceId, m_IoProc);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::Stop: Unable to stop device. Error = 0x%08x (%4.4s).", ret, CONVERT_OSSTATUS(ret));
  */
  
  m_Started = false;
}

bool CIOSCoreAudioDevice::AddIOProc(AudioDeviceIOProc ioProc, void* pCallbackData)
{
  if (!m_DeviceId || m_IoProc) // Only one IOProc at a time
    return false;
  
  /*
  OSStatus ret = AudioDeviceAddIOProc(m_DeviceId, ioProc, pCallbackData);  
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::Stop: Unable to add IOProc. Error = 0x%08x (%4.4s).", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  m_IoProc = ioProc;
  CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::AddIOProc: IOProc set for device 0x%04x", m_DeviceId);
  */
  
  return true;
}

void CIOSCoreAudioDevice::RemoveIOProc()
{
  if (!m_DeviceId || !m_IoProc)
    return;
  
  Stop();

  /*
  OSStatus ret = AudioDeviceRemoveIOProc(m_DeviceId, m_IoProc);  
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::RemoveIOProc: Unable to remove IOProc. Error = 0x%08x (%4.4s).", ret, CONVERT_OSSTATUS(ret));
  else
    CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::AddIOProc: IOProc removed for device 0x%04x", m_DeviceId);
  */
  m_IoProc = NULL; // Clear the reference no matter what
}

const char* CIOSCoreAudioDevice::GetName(CStdString& name)
{
  if (!m_DeviceId)
    return NULL;

  UInt32 size = 0;
  AudioDeviceGetPropertyInfo(m_DeviceId,0, false, kAudioDevicePropertyDeviceName, &size, NULL); // TODO: Change to kAudioObjectPropertyObjectName
  OSStatus ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyDeviceName, &size, name.GetBufferSetLength(size));  
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::GetName: Unable to get device name - id: 0x%04x Error = 0x%08x (%4.4s)", m_DeviceId, ret, CONVERT_OSSTATUS(ret));
    return NULL;
  }
  return name.c_str();
}

UInt32 CIOSCoreAudioDevice::GetTotalOutputChannels()
{
  if (!m_DeviceId)
    return 0;
  
  UInt32 channels = 0;
	UInt32 size = 0;
  AudioDeviceGetPropertyInfo(m_DeviceId, 0, false, kAudioDevicePropertyStreamConfiguration, &size, NULL);
  AudioBufferList* pList = (AudioBufferList*)malloc(size);
  OSStatus ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyStreamConfiguration, &size, pList); 
  if (!ret)
    for(UInt32 buffer = 0; buffer < pList->mNumberBuffers; ++buffer)
      channels += pList->mBuffers[buffer].mNumberChannels;
  else
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::GetTotalOutputChannels: Unable to get total device output channels - id: 0x%04x Error = 0x%08x (%4.4s)", m_DeviceId, ret, CONVERT_OSSTATUS(ret));
  CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::GetTotalOutputChannels: Found %u channels in %u buffers", channels, pList->mNumberBuffers);
  free(pList);	
  return channels;
}

bool CIOSCoreAudioDevice::GetStreams(AudioStreamIdList* pList)
{
  if (!pList || !m_DeviceId)
    return false;
  
  UInt32 propertySize = 0;
  Boolean writable = false;
  OSStatus ret = AudioDeviceGetPropertyInfo(m_DeviceId, 0, false, kAudioDevicePropertyStreams, &propertySize, &writable);
  if (ret)
    return false;
  UInt32 streamCount = propertySize / sizeof(AudioStreamID);
  AudioStreamID* pStreamList = new AudioStreamID[streamCount];
  ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyStreams, &propertySize, pStreamList);
  if (!ret)
  {
    for (UInt32 stream = 0; stream < streamCount; stream++)
      pList->push_back(pStreamList[stream]);
  }
  delete[] pStreamList;
  return (ret == noErr);  
}


bool CIOSCoreAudioDevice::IsRunning()
{
  UInt32 isRunning = false;
  UInt32 size = sizeof(isRunning);
  OSStatus ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyDeviceIsRunning, &size, &isRunning);
  if (ret)
    return false;
  return (isRunning != 0);
}

bool CIOSCoreAudioDevice::SetHogStatus(bool hog)
{
  // According to Jeff Moore (Core Audio, Apple), Setting kAudioDevicePropertyHogMode
  // is a toggle and the only way to tell if you do get hog mode is to compare
  // the returned pid against getpid, if the match, you have hog mode, if not you don't.
  if (!m_DeviceId)
    return false;
  
  /*
  if (hog)
  {
    if (m_Hog == -1) // Not already set
    {
      CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::SetHogStatus: Setting 'hog' status on device 0x%04x", m_DeviceId);
      OSStatus ret = AudioDeviceSetProperty(m_DeviceId, NULL, 0, false, kAudioDevicePropertyHogMode, sizeof(m_Hog), &m_Hog);
      if (ret || m_Hog != getpid())
      {
        CLog::Log(LOGERROR, "CIOSCoreAudioDevice::SetHogStatus: Unable to set 'hog' status. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
        return false;
      }
      CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::SetHogStatus: Successfully set 'hog' status on device 0x%04x", m_DeviceId);
    }
  }
  else
  {
    if (m_Hog > -1) // Currently Set
    {
      CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::SetHogStatus: Releasing 'hog' status on device 0x%04x", m_DeviceId);
      pid_t hogPid = -1;
      OSStatus ret = AudioDeviceSetProperty(m_DeviceId, NULL, 0, false, kAudioDevicePropertyHogMode, sizeof(hogPid), &hogPid);
      if (ret || hogPid == getpid())
      {
        CLog::Log(LOGERROR, "CIOSCoreAudioDevice::SetHogStatus: Unable to release 'hog' status. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
        return false;
      }
      m_Hog = hogPid; // Reset internal state
    }
  }
  return true;
  */
  
  return false;
}

pid_t CIOSCoreAudioDevice::GetHogStatus()
{
  /*
  if (!m_DeviceId)
    return false;
  
  pid_t hogPid = -1;
  UInt32 size = sizeof(hogPid);
  AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyHogMode, &size, &hogPid);

  return hogPid;
  */
  return false;
}

bool CIOSCoreAudioDevice::SetMixingSupport(bool mix)
{
  return false;
  /*
  if (!m_DeviceId)
    return false;
  int restore = -1;
  if (m_MixerRestore == -1) // This is our first change to this setting. Store the original setting for restore
    restore = (GetMixingSupport() ? 1 : 0);
  UInt32 mixEnable = mix ? 1 : 0;
  CLog::Log(LOGDEBUG, "CIOSCoreAudioDevice::SetMixingSupport: %sabling mixing for device 0x%04x",mix ? "En" : "Dis",  m_DeviceId);
  OSStatus ret = AudioDeviceSetProperty(m_DeviceId, NULL, 0, false, kAudioDevicePropertySupportsMixing, sizeof(mixEnable), &mixEnable);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::SetMixingSupport: Unable to set MixingSupport to %s. Error = 0x%08x (%4.4s)", mix ? "'On'" : "'Off'", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  if (m_MixerRestore == -1) 
    m_MixerRestore = restore;
  return true;
  */
}

bool CIOSCoreAudioDevice::GetMixingSupport()
{
  return false;
  /*
  if (!m_DeviceId)
    return false;
  UInt32 val = 0;
  UInt32 size = sizeof(val);
  OSStatus ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertySupportsMixing, &size, &val);
  if (ret)
    return false;
  return (val > 0);
  */
}

bool CIOSCoreAudioDevice::GetPreferredChannelLayout(CoreAudioChannelList* pChannelMap)
{
  if (!pChannelMap || !m_DeviceId)
    return false;

  /*
  UInt32 propertySize = 0;
  Boolean writable = false;

  // BAD
  OSStatus ret = AudioDeviceGetPropertyInfo(m_DeviceId, 0, false, kAudioDevicePropertyPreferredChannelLayout, &propertySize, &writable);
  if (ret)
    return false;
  
  // kAudioChannelLabel_Unknown = -1 (0xffffffff)
  // kAudioChannelLabel_Unused = 0
  // kAudioChannelLabel_Left = 1
  // kAudioChannelLabel_Right = 2
  // ...
  
  void* pBuf = malloc(propertySize);
  AudioChannelLayout* pLayout = (AudioChannelLayout*)pBuf;
  ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyPreferredChannelLayout, &propertySize, pBuf);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetPreferredChannelLayout: Unable to retrieve preferred channel layout. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
  else
  {
    if(pLayout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions)
    {
      for (UInt32 i = 0; i < pLayout->mNumberChannelDescriptions; i++)
      {
        if (pLayout->mChannelDescriptions[i].mChannelLabel == kAudioChannelLabel_Unknown)
          pChannelMap->push_back(i + 1); // TODO: This is not the best way to handle unknown/unconfigured speaker layouts
        else
          pChannelMap->push_back(pLayout->mChannelDescriptions[i].mChannelLabel); // Will be one of kAudioChannelLabel_xxx
      }
    }
    else
    {
      // TODO: Determine if a method that uses a channel bitmap is also necessary
      free(pLayout);
      return false;
    }
  } 

  free(pLayout);
  return (ret == noErr);  
  */
  
  return false;
  
}

bool CIOSCoreAudioDevice::GetDataSources(CoreAudioDataSourceList* pList)
{
  /*
  if (!pList || !m_DeviceId)
    return false;
  
  UInt32 propertySize = 0;
  Boolean writable = false;
  OSStatus ret = AudioDeviceGetPropertyInfo(m_DeviceId, 0, false, kAudioDevicePropertyDataSources, &propertySize, &writable);
  if (ret)
    return false;
  UInt32 sources = propertySize / sizeof(UInt32);
  UInt32* pSources = new UInt32[sources];
  ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyDataSources, &propertySize, pSources);
  if (!ret)
    for (UInt32 i = 0; i < sources; i++)
      pList->push_back(pSources[i]);;
  delete[] pSources;
  return (!ret);
  */
  return false;
}

Float64 CIOSCoreAudioDevice::GetNominalSampleRate()
{
  if (!m_DeviceId)
    return 0.0f;
  
  Float64 sampleRate = 0.0f;
  UInt32 size = sizeof(Float64);
  OSStatus ret = AudioDeviceGetProperty(m_DeviceId, 0, false, kAudioDevicePropertyNominalSampleRate, &size, &sampleRate);
  if (ret)
  { 
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetNominalSampleRate: Unable to retrieve current device sample rate. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return 0.0f;
  }
  return sampleRate;
}

bool CIOSCoreAudioDevice::SetNominalSampleRate(Float64 sampleRate)
{
  if (!m_DeviceId || sampleRate == 0.0f)
    return false;
  
  Float64 currentRate = GetNominalSampleRate();
  if (currentRate == sampleRate)
    return true; //No need to change
    
  UInt32 size = sizeof(Float64);
  OSStatus ret = AudioDeviceSetProperty(m_DeviceId, NULL, 0, false, kAudioDevicePropertyNominalSampleRate, size, &sampleRate);
  if (ret)
  { 
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetNominalSampleRate: Unable to set current device sample rate to %0.0f. Error = 0x%08x (%4.4s)", (float)sampleRate, ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  CLog::Log(LOGDEBUG,  "CIOSCoreAudioUnit::SetNominalSampleRate: Changed device sample rate from %0.0f to %0.0f.", (float)currentRate, (float)sampleRate);
  if (m_SampleRateRestore == 0.0f)
    m_SampleRateRestore = currentRate;
  
  return true;
}

UInt32 CIOSCoreAudioDevice::GetNumLatencyFrames()
{
  //return 0;

  UInt32 i_param, i_param_size, num_latency_frames = 0;
  if (!m_DeviceId)
    return 0;  

  i_param_size = sizeof(uint32_t);

  // number of frames of latency in the AudioDevice
  // BAD
  /*
  if (noErr == AudioDeviceGetProperty(m_DeviceId, 0, false, 
    kAudioDevicePropertyLatency, &i_param_size, &i_param))
  {
    num_latency_frames += i_param;
  }
  */
  // number of frames in the IO buffers
  if (noErr == AudioDeviceGetProperty(m_DeviceId, 0, false,
    kAudioDevicePropertyBufferFrameSize, &i_param_size, &i_param))
  {
    num_latency_frames += i_param;
  }
 
  // number for frames in ahead the current hardware position that is safe to do IO
  if (noErr == AudioDeviceGetProperty(m_DeviceId, 0, false, 
    kAudioDevicePropertySafetyOffset, &i_param_size, &i_param))
 	{
    num_latency_frames += i_param;
  }
  
  return(num_latency_frames);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIOSCoreAudioStream
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIOSCoreAudioStream::CIOSCoreAudioStream() :
m_StreamId(0)
{
  m_OriginalVirtualFormat.mFormatID = 0;
  m_OriginalPhysicalFormat.mFormatID = 0;
}

CIOSCoreAudioStream::~CIOSCoreAudioStream()
{
  Close();
}

bool CIOSCoreAudioStream::Open(AudioStreamID streamId)
{
  m_StreamId = streamId;
  CLog::Log(LOGDEBUG, "CIOSCoreAudioStream::Open: Opened stream 0x%04x.", m_StreamId);
  return true;
}

// TODO: Should it even be possible to change both the physical and virtual formats, since the devices do it themselves?
void CIOSCoreAudioStream::Close()
{
  if (!m_StreamId)
    return;
  
  // Revert any format changes we made
  if (m_OriginalVirtualFormat.mFormatID && m_StreamId)
  {
    CLog::Log(LOGDEBUG, "CIOSCoreAudioStream::Close: Restoring original virtual format for stream 0x%04x.", m_StreamId);
    SetVirtualFormat(&m_OriginalVirtualFormat);
  }
  if (m_OriginalPhysicalFormat.mFormatID && m_StreamId)
  {
    CLog::Log(LOGDEBUG, "CIOSCoreAudioStream::Close: Restoring original physical format for stream 0x%04x.", m_StreamId);
    SetPhysicalFormat(&m_OriginalPhysicalFormat);
  }
  
  m_OriginalPhysicalFormat.mFormatID = 0;
  m_OriginalVirtualFormat.mFormatID = 0;
  CLog::Log(LOGDEBUG, "CIOSCoreAudioStream::Close: Closed stream 0x%04x.", m_StreamId);
  m_StreamId = 0;
}

UInt32 CIOSCoreAudioStream::GetDirection()
{
  if (!m_StreamId)
    return 0;
  UInt32 size = sizeof(UInt32);
  UInt32 val = 0;
  OSStatus ret = AudioStreamGetProperty(m_StreamId, 0, kAudioStreamPropertyDirection, &size, &val);
  if (ret)
    return 0;
  return val;
}

UInt32 CIOSCoreAudioStream::GetTerminalType()
{
  if (!m_StreamId)
    return 0;
  UInt32 size = sizeof(UInt32);
  UInt32 val = 0;
  OSStatus ret = AudioStreamGetProperty(m_StreamId, 0, kAudioStreamPropertyTerminalType, &size, &val);
  if (ret)
    return 0;
  return val;  
}

UInt32 CIOSCoreAudioStream::GetNumLatencyFrames()
{
  UInt32 i_param, i_param_size, num_latency_frames = 0;
  if (!m_StreamId)
    return 0;

  i_param_size = sizeof(uint32_t);

  // number of frames of latency in the AudioStream
  if (noErr == AudioStreamGetProperty(m_StreamId, 0, 
    kAudioStreamPropertyLatency, &i_param_size, &i_param))
  {
    num_latency_frames += i_param;
  }

  return(num_latency_frames);
}

bool CIOSCoreAudioStream::GetVirtualFormat(AudioStreamBasicDescription* pDesc)
{
  if (!pDesc || !m_StreamId)
    return false;
  UInt32 size = sizeof(AudioStreamBasicDescription);
  OSStatus ret = AudioStreamGetProperty(m_StreamId, 0, kAudioStreamPropertyVirtualFormat, &size, pDesc);
  if (ret)
    return false;
  return true;
}

bool CIOSCoreAudioStream::SetVirtualFormat(AudioStreamBasicDescription* pDesc)
{
  if (!pDesc || !m_StreamId)
    return false;
  if (!m_OriginalVirtualFormat.mFormatID)
  {
    if (!GetVirtualFormat(&m_OriginalVirtualFormat)) // Store the original format (as we found it) so that it can be restored later
    {
      CLog::Log(LOGERROR, "CIOSCoreAudioStream::SetVirtualFormat: Unable to retrieve current virtual format for stream 0x%04x.", m_StreamId);
      return false;
    }
  }
  OSStatus ret = AudioStreamSetProperty(m_StreamId, NULL, 0, kAudioStreamPropertyVirtualFormat, sizeof(AudioStreamBasicDescription), pDesc);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioStream::SetVirtualFormat: Unable to set virtual format for stream 0x%04x. Error = 0x%08x (%4.4s)", m_StreamId, ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;   
}

bool CIOSCoreAudioStream::GetPhysicalFormat(AudioStreamBasicDescription* pDesc)
{
  if (!pDesc || !m_StreamId)
    return false;
  UInt32 size = sizeof(AudioStreamBasicDescription);
  OSStatus ret = AudioStreamGetProperty(m_StreamId, 0, kAudioStreamPropertyPhysicalFormat, &size, pDesc);
  if (ret)
    return false;
  return true;   
}

bool CIOSCoreAudioStream::SetPhysicalFormat(AudioStreamBasicDescription* pDesc)
{
  if (!pDesc || !m_StreamId)
    return false;
  if (!m_OriginalPhysicalFormat.mFormatID)
  {
    if (!GetPhysicalFormat(&m_OriginalPhysicalFormat)) // Store the original format (as we found it) so that it can be restored later
    {
      CLog::Log(LOGERROR, "CIOSCoreAudioStream::SetPhysicalFormat: Unable to retrieve current physical format for stream 0x%04x.", m_StreamId);
      return false;
    }
  }  
  OSStatus ret = AudioStreamSetProperty(m_StreamId, NULL, 0, kAudioStreamPropertyPhysicalFormat, sizeof(AudioStreamBasicDescription), pDesc);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioStream::SetVirtualFormat: Unable to set physical format for stream 0x%04x. Error = 0x%08x (%4.4s)", m_StreamId, ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;   
}

bool CIOSCoreAudioStream::GetAvailableVirtualFormats(StreamFormatList* pList)
{
  if (!pList || !m_StreamId)
    return false;
  
  UInt32 propertySize = 0;
  Boolean writable = false;
  OSStatus ret = AudioStreamGetPropertyInfo(m_StreamId, 0, kAudioStreamPropertyAvailableVirtualFormats, &propertySize, &writable);
  if (ret)
    return false;
  UInt32 formatCount = propertySize / sizeof(AudioStreamRangedDescription);
  AudioStreamRangedDescription* pFormatList = new AudioStreamRangedDescription[formatCount];
  ret = AudioStreamGetProperty(m_StreamId, 0, kAudioStreamPropertyAvailableVirtualFormats, &propertySize, pFormatList);
  if (!ret)
  {
    for (UInt32 format = 0; format < formatCount; format++)
      pList->push_back(pFormatList[format]);
  }
  delete[] pFormatList;
  return (ret == noErr);
}

bool CIOSCoreAudioStream::GetAvailablePhysicalFormats(StreamFormatList* pList)
{
  if (!pList || !m_StreamId)
    return false;
  
  UInt32 propertySize = 0;
  Boolean writable = false;
  OSStatus ret = AudioStreamGetPropertyInfo(m_StreamId, 0, kAudioStreamPropertyAvailablePhysicalFormats, &propertySize, &writable);
  if (ret)
    return false;
  UInt32 formatCount = propertySize / sizeof(AudioStreamRangedDescription);
  AudioStreamRangedDescription* pFormatList = new AudioStreamRangedDescription[formatCount];
  ret = AudioStreamGetProperty(m_StreamId, 0, kAudioStreamPropertyAvailablePhysicalFormats, &propertySize, pFormatList);
  if (!ret)
  {
    for (UInt32 format = 0; format < formatCount; format++)
      pList->push_back(pFormatList[format]);
  }
  delete[] pFormatList;
  return (ret == noErr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIOSCoreAudioUnit
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIOSCoreAudioUnit::CIOSCoreAudioUnit() :
  m_Initialized(false),
  m_AudioUnit(NULL)
{
  
}

CIOSCoreAudioUnit::~CIOSCoreAudioUnit() 
{
  Close();
}

bool CIOSCoreAudioUnit::Open(AudioComponentDescription desc)
{
  if (m_AudioUnit)
    Close();

  OSStatus ret;
  
  // Get component
  AudioComponent outputComponent = AudioComponentFindNext(NULL, &desc);
  
  // Get audio units
  ret = AudioComponentInstanceNew(outputComponent, &m_AudioUnit);
  
  if (ret) {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::Open: Unable to open AudioUnit Component. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  	
  return true;
}

bool CIOSCoreAudioUnit::Open(OSType type, OSType subType, OSType manufacturer)
{
  AudioComponentDescription desc;
  desc.componentType = type;
  desc.componentSubType = subType;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;
  desc.componentManufacturer = manufacturer;

  return Open(desc);
}


void CIOSCoreAudioUnit::Close()
{
  if (m_Initialized)
    AudioUnitUninitialize(m_AudioUnit);
  /*
  if (m_AudioUnit)
    CloseComponent(m_AudioUnit);
  */
  m_Initialized = false;
  m_AudioUnit = 0;
}

bool CIOSCoreAudioUnit::Initialize()
{
  if (!m_AudioUnit)
    return false;
  
  OSStatus ret = AudioUnitInitialize(m_AudioUnit);
  if (ret)
  { 
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::Initialize: Unable to Initialize AudioUnit. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false; 
  } 
  m_Initialized = true;
  return true;
}


bool CIOSCoreAudioUnit::GetInputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  UInt32 size = sizeof(AudioStreamBasicDescription);
  OSStatus ret = AudioUnitGetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Input, 0, pDesc, &size);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetInputFormat: Unable to get AudioUnit input format. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;
}

bool CIOSCoreAudioUnit::GetOutputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  UInt32 size = sizeof(AudioStreamBasicDescription);
  OSStatus ret = AudioUnitGetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Output, 0, pDesc, &size);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetInputFormat: Unable to get AudioUnit output format. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;
}

bool CIOSCoreAudioUnit::SetInputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Input, 0, pDesc, sizeof(AudioStreamBasicDescription));
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetInputFormat: Unable to set AudioUnit input format. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;  
}

bool CIOSCoreAudioUnit::SetOutputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Output, 0, pDesc, sizeof(AudioStreamBasicDescription));
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetInputFormat: Unable to set AudioUnit output format. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;  
}

bool CIOSCoreAudioUnit::SetRenderProc(AURenderCallback callback, void* pClientData)
{
  if (!m_AudioUnit)
    return false;
  
  UInt32 flag = 0;
  AURenderCallbackStruct callbackInfo;
	callbackInfo.inputProc = callback; // Function to be called each time the AudioUnit needs data
	callbackInfo.inputProcRefCon = pClientData; // Pointer to be returned in the callback proc
	OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioUnitProperty_SetRenderCallback, 
                                      kAudioUnitScope_Global, 0, &callbackInfo, sizeof(AURenderCallbackStruct));
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetRenderProc: Unable to set AudioUnit render callback. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  
  /*
  ret = AudioUnitSetProperty(m_AudioUnit,  kAudioUnitProperty_ShouldAllocateBuffer,
                            kAudioUnitScope_Global, 0, &flag, sizeof(flag));  
  
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetRenderProc: Unable to set AudioUnitSetProperty Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  */
  
  return true;
}


bool CIOSCoreAudioUnit::SetMaxFramesPerSlice(UInt32 maxFrames)
{
  if (!m_AudioUnit)
    return false;
  
	OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioUnitProperty_MaximumFramesPerSlice, 
                                      kAudioUnitScope_Global, 0, &maxFrames, sizeof(UInt32));
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetMaxFramesPerSlice: Unable to set AudioUnit max frames per slice. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  
  return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIOSAUOutputDevice
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: The channel map setter/getter are inefficient

CIOSAUOutputDevice::CIOSAUOutputDevice()
{
  
}

CIOSAUOutputDevice::~CIOSAUOutputDevice()
{
  
}

bool CIOSAUOutputDevice::SetCurrentDevice(AudioDeviceID deviceId)
{
  if (!m_AudioUnit)
    return false;
  
  /*
  OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &deviceId, sizeof(AudioDeviceID));
  if (ret)
  { 
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetCurrentDevice: Unable to set current device. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false; 
  }
  */
  return true;
}

bool CIOSAUOutputDevice::GetInputChannelMap(CoreAudioChannelList* pChannelMap)
{
  if (!m_AudioUnit)
    return false;
  
  return false;
  
  UInt32 size = 0;
  Boolean writable = false;
  AudioUnitGetPropertyInfo(m_AudioUnit, kAudioOutputUnitProperty_ChannelMap, kAudioUnitScope_Input, 0, &size, &writable);
  UInt32 channels = size/sizeof(SInt32);
  SInt32* pMap = new SInt32[channels];
  OSStatus ret = AudioUnitGetProperty(m_AudioUnit, kAudioOutputUnitProperty_ChannelMap, kAudioUnitScope_Input, 0, pMap, &size);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetInputChannelMap: Unable to retrieve AudioUnit input channel map. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
  else
    for (UInt32 i = 0; i < channels; i++)
      pChannelMap->push_back(pMap[i]);  
  delete[] pMap;
  return (!ret);
}

bool CIOSAUOutputDevice::SetInputChannelMap(CoreAudioChannelList* pChannelMap)
{
	// The number of array elements must match the number of output channels provided by the device
  if (!m_AudioUnit || !pChannelMap)
    return false;
  
  return false;
  
  UInt32 channels = pChannelMap->size();
  UInt32 size = sizeof(SInt32) * channels;
  SInt32* pMap = new SInt32[channels];
  for (UInt32 i = 0; i < channels; i++)
    pMap[i] = (*pChannelMap)[i];
  OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioOutputUnitProperty_ChannelMap, kAudioUnitScope_Input, 0, pMap, size);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetInputChannelMap: Unable to get current device's buffer size. ErrCode = Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
  delete[] pMap;
  return (!ret);
}

void CIOSAUOutputDevice::Start()
{
  // TODO: Check component status
  if (m_AudioUnit && m_Initialized)
    AudioOutputUnitStart(m_AudioUnit);  
}

void CIOSAUOutputDevice::Stop()
{
  // TODO: Check component status
  if (m_AudioUnit && m_Initialized)
    AudioOutputUnitStop(m_AudioUnit);    
}

Float32 CIOSAUOutputDevice::GetCurrentVolume()
{
  if (!m_AudioUnit)
    return 0.0f;
  
  Float32 volPct = 0.0f;
  /*
  OSStatus ret = AudioUnitGetParameter(m_AudioUnit,  kHALOutputParam_Volume, kAudioUnitScope_Global, 0, &volPct);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetCurrentVolume: Unable to get AudioUnit volume. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return 0.0f;
  }
  */
  return volPct;
}

bool CIOSAUOutputDevice::SetCurrentVolume(Float32 vol)
{
  if (!m_AudioUnit)
    return false;
  
  /*
  OSStatus ret = AudioUnitSetParameter(m_AudioUnit, kHALOutputParam_Volume, kAudioUnitScope_Global, 0, vol, 0);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetCurrentVolume: Unable to set AudioUnit volume. Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  */
  return true;
}

bool CIOSAUOutputDevice::IsRunning()
{
  if (!m_AudioUnit)
    return false;
  
  UInt32 isRunning = 0;
  UInt32 size = sizeof(isRunning);
  AudioUnitGetProperty(m_AudioUnit, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &isRunning, &size);
  return (isRunning != 0);
}

UInt32 CIOSAUOutputDevice::GetBufferFrameSize()
{
  if (!m_AudioUnit)
    return 0;
  
  UInt32 size = sizeof(UInt32);
  UInt32 bufferSize = 0;
  OSStatus ret;
  
  /*
  AudioObjectPropertyAddress pa;
  pa.mElement = kAudioObjectPropertyElementMaster;
  pa.mScope = kAudioObjectPropertyScopeWildcard;
  pa.mSelector = kAudioDevicePropertyBufferFrameSize;
  
  ret = AudioObjectGetPropertyData (m_AudioUnit, &pa, 0, 0, &size, &bufferSize))
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetBufferFrameSize: Unable to get current device's buffer size. ErrCode = Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return 1024;
  }
  */
  
  ret = AudioUnitGetProperty(m_AudioUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Input, 0, &bufferSize, &size);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetBufferFrameSize: Unable to get current device's buffer size. ErrCode = Error = 0x%08x (%4.4s)", ret, CONVERT_OSSTATUS(ret));
    return 1024*1024;
  }
  return bufferSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIOSAUMatrixMixer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CIOSAUMatrixMixer::CIOSAUMatrixMixer()
{
  
}

CIOSAUMatrixMixer::~CIOSAUMatrixMixer()
{
  
}



//#endif
#endif
