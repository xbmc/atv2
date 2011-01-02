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

AudioComponentInstance CIOSCoreAudioHardware::FindAudioDevice(CStdString searchName)
{
  if (!searchName.length())
    return 0;
  
  //UInt32 size = 0;
  //AudioComponentInstance deviceId = 0;
  //OSStatus ret;
 
  // For now we alway return the default audio device
  //if (searchName.Equals("Default Output Device"))
  //{
    AudioComponentInstance defaultDevice = GetDefaultOutputDevice();
    CLog::Log(LOGDEBUG, "CIOSCoreAudioHardware::FindAudioDevice: Returning default device [0x%04x].", (uint32_t)defaultDevice);
    return defaultDevice;  
  //}
  /*
  CLog::Log(LOGDEBUG, "CIOSCoreAudioHardware::FindAudioDevice: Searching for device - %s.", searchName.c_str());
  
  // Obtain a list of all available audio devices
  AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, NULL);
  UInt32 deviceCount = size / sizeof(AudioComponentInstance);
  AudioComponentInstance* pDevices = new AudioDeviceID[deviceCount];
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
    CLog::Log(LOGDEBUG, "CIOSCoreAudioHardware::FindAudioDevice:   Device[0x%08x] - Name: '%s' ", (unsigned int)pDevices[dev], deviceName.c_str());
    if (searchName.Equals(deviceName))
      deviceId = pDevices[dev];
    if (deviceId)
      break;
  }
  delete[] pDevices;  
  
  return deviceId;
  */
}

AudioComponentInstance CIOSCoreAudioHardware::GetDefaultOutputDevice()
{
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
    CLog::Log(LOGERROR, "CIOSCoreAudioHardware::GetDefaultOutputDevice: Unable to identify default output device. Error = 0x%08x (%4.4s).", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return 0;
  }
  
  return audioUnit;
}

UInt32 CIOSCoreAudioHardware::GetOutputDevices(IOSCoreAudioDeviceList* pList)
{
  if (!pList)
    return 0;
  
  // Obtain a list of all available audio devices
  /*
  OSStatus ret;
  
  AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, NULL);
  UInt32 deviceCount = size / sizeof(AudioComponentInstance);
  AudioComponentInstance* pDevices = new AudioDeviceID[deviceCount];
  ret = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, pDevices);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioHardware::GetOutputDevices: Unable to retrieve the list of available devices. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIOSCoreAudioDevice
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIOSCoreAudioDevice::CIOSCoreAudioDevice()  : 
  m_AudioUnit(0)
{
  
}

CIOSCoreAudioDevice::CIOSCoreAudioDevice(AudioComponentInstance deviceId) : 
  m_AudioUnit(deviceId)
{
  
}

CIOSCoreAudioDevice::~CIOSCoreAudioDevice()
{
  Stop();
}

void CIOSCoreAudioDevice::SetDevice(AudioComponentInstance deviceId)
{
  if(m_AudioUnit) 
    Stop();

  m_AudioUnit = deviceId;
}

bool CIOSCoreAudioDevice::Open()
{
  if(!m_AudioUnit)
    return false;
  
  OSStatus ret = AudioUnitInitialize(m_AudioUnit);
  if (ret)
  { 
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::Initialize: Unable to Initialize AudioUnit. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return false; 
  } 

  return true;
}

void CIOSCoreAudioDevice::Close()
{
  if (!m_AudioUnit)
    return;
 
  Stop();
  
  OSStatus ret = AudioUnitUninitialize(m_AudioUnit);
  
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::Close: Unable to close device. Error = 0x%08x (%4.4s).", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    
  m_AudioUnit = 0;
  
}

void CIOSCoreAudioDevice::Start()
{
  if (!m_AudioUnit) 
    return;
  
  OSStatus ret = AudioOutputUnitStart(m_AudioUnit);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::Start: Unable to start device. Error = 0x%08x (%4.4s).", (uint32_t)ret, CONVERT_OSSTATUS(ret));
  
}

void CIOSCoreAudioDevice::Stop()
{
  if (!m_AudioUnit)
    return;
  
  OSStatus ret = AudioOutputUnitStop(m_AudioUnit);
  if (ret)
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::Stop: Unable to stop device. Error = 0x%08x (%4.4s).", (uint32_t)ret, CONVERT_OSSTATUS(ret));
  
}

const char* CIOSCoreAudioDevice::GetName(CStdString& name)
{
  if (!m_AudioUnit)
    return NULL;

  /*
  UInt32 size = 0;
  AudioDeviceGetPropertyInfo(m_AudioUnit,0, false, kAudioDevicePropertyDeviceName, &size, NULL); // TODO: Change to kAudioObjectPropertyObjectName
  OSStatus ret = AudioDeviceGetProperty(m_AudioUnit, 0, false, kAudioDevicePropertyDeviceName, &size, name.GetBufferSetLength(size));  
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioDevice::GetName: Unable to get device name - id: 0x%08x Error = 0x%08x (%4.4s)", (uint32_t)m_AudioUnit, (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return NULL;
  }
  */
  return name.c_str();
}

bool CIOSCoreAudioDevice::GetInputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  UInt32 size = sizeof(AudioStreamBasicDescription);
  OSStatus ret = AudioUnitGetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Input, 0, pDesc, &size);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetInputFormat: Unable to get AudioUnit input format. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;
}

bool CIOSCoreAudioDevice::GetOutputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  UInt32 size = sizeof(AudioStreamBasicDescription);
  OSStatus ret = AudioUnitGetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Output, 0, pDesc, &size);
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::GetInputFormat: Unable to get AudioUnit output format. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;
}

bool CIOSCoreAudioDevice::SetInputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Input, 0, pDesc, sizeof(AudioStreamBasicDescription));
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetInputFormat: Unable to set AudioUnit input format. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;  
}

bool CIOSCoreAudioDevice::SetOutputFormat(AudioStreamBasicDescription* pDesc)
{
  if (!m_AudioUnit || !pDesc)
    return false;
  
  OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Output, 0, pDesc, sizeof(AudioStreamBasicDescription));
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetOutputFormat: Unable to set AudioUnit output format. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  return true;  
}

bool CIOSCoreAudioDevice::SetRenderProc(AURenderCallback callback, void* pClientData)
{
  if (!m_AudioUnit)
    return false;
  
  AURenderCallbackStruct callbackInfo;
	callbackInfo.inputProc = callback; // Function to be called each time the AudioUnit needs data
	callbackInfo.inputProcRefCon = pClientData; // Pointer to be returned in the callback proc
	OSStatus ret = AudioUnitSetProperty(m_AudioUnit, kAudioUnitProperty_SetRenderCallback, 
                                      kAudioUnitScope_Global, 0, &callbackInfo, sizeof(AURenderCallbackStruct));
  if (ret)
  {
    CLog::Log(LOGERROR, "CIOSCoreAudioUnit::SetRenderProc: Unable to set AudioUnit render callback. Error = 0x%08x (%4.4s)", (uint32_t)ret, CONVERT_OSSTATUS(ret));
    return false;
  }
  
  return true;
}

//#endif
#endif
