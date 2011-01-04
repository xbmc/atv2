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

#if defined(__APPLE__)
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioServices.h>
#include "AudioHardware.h">
#include <StdString.h>
#include <list>
#include <vector>

//typedef short SInt16;
//typedef float Float32;

#include <StdString.h>
#include <list>
#include <vector>

// Forward declarations
class CIOSCoreAudioHardware;
class CIOSCoreAudioDevice;
class CIOSCoreAudioUnit;

typedef std::list<AudioComponentInstance> IOSCoreAudioDeviceList;

// There is only one AudioSystemObject instance system-side.
// Therefore, all CIOSCoreAudioHardware methods are static
class CIOSCoreAudioHardware
{
public:
  static AudioComponentInstance FindAudioDevice(CStdString deviceName);
  static AudioComponentInstance GetDefaultOutputDevice();
  static UInt32 GetOutputDevices(IOSCoreAudioDeviceList* pList);
};

class CIOSCoreAudioDevice
{
public:
  CIOSCoreAudioDevice();
  CIOSCoreAudioDevice(AudioComponentInstance deviceId);
  virtual ~CIOSCoreAudioDevice();
  
  AudioComponentInstance GetId() {return m_AudioUnit;}
  const char* GetName(CStdString& name);
  UInt32 GetTotalOutputChannels();

  void Attach(AudioUnit audioUnit) {m_AudioUnit = audioUnit;}
  AudioComponentInstance GetComponent(){return m_AudioUnit;}
  
  void SetDevice(AudioComponentInstance deviceId);
  bool Open();  
  void Close();
  void Start();
  void Stop();

  bool EnableInput();
  bool EnableOutput();
  bool SetRenderProc(AURenderCallback callback, void* pClientData);
  bool GetInputFormat(AudioStreamBasicDescription* pDesc);
  bool GetOutputFormat(AudioStreamBasicDescription* pDesc);    
  bool SetInputFormat(AudioStreamBasicDescription* pDesc);
  bool SetOutputFormat(AudioStreamBasicDescription* pDesc);
  bool SetOutputSampleRate(Float64 sampleRate);
  int  FramesPerSlice();
protected:
  AudioComponentInstance m_AudioUnit;
};

// Helper Functions
char* IOSUInt32ToFourCC(UInt32* val);
const char* IOSStreamDescriptionToString(AudioStreamBasicDescription desc, CStdString& str);

#define CONVERT_OSSTATUS(x) IOSUInt32ToFourCC((UInt32*)&ret)

#endif
#endif // __COREAUDIO_H__
