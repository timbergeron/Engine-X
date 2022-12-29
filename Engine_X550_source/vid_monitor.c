#include "quakedef.h" // Almost optional! 
#include <windows.h> 
#define MSGBOX(x) MessageBox(NULL, x, "Info", MB_OK);      // Developer message 

typedef struct 
{ 
   char DeviceName[32]; // wingdi.h --> CCHFORMNAME which is 32 
   int original_x; 
   int original_y; 
   int original_width; 
   int original_height; 
} monitor_def_t; 
#define MAX_NUM_MONITORS 2 

monitor_def_t monitor_info[MAX_NUM_MONITORS]; 
int original_primary = -1; 
int original_secondary = -1; 
int num_monitors; 
int monitors_swapped;    

DISPLAY_DEVICE DisplayDevice; 

char monitorstring[2048]; 

DEVMODE deviceMode[MAX_NUM_MONITORS]; 

void CollectMonitorInformation (void) 
{ 
   int i; 
   { 
      // Reset monitor info 
      num_monitors = 0; // Just in case 
      original_primary = -1; // Just in case 
      ZeroMemory(&DisplayDevice, sizeof(DISPLAY_DEVICE)); 
      DisplayDevice.cb = sizeof(DisplayDevice); 
   } 

   for(i = 0; EnumDisplayDevices(NULL, i, &DisplayDevice, 0) && num_monitors < MAX_NUM_MONITORS; i++) 
   { 
      if(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER || !(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) 
         continue; // Non-monitor 

      // Ok we found a monitor 
          
      deviceMode[num_monitors].dmSize = sizeof(DEVMODE); 
      deviceMode[num_monitors].dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS; 
          
      EnumDisplaySettings(DisplayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode[num_monitors]); 
    
      // Populate fields 
      strcpy (monitor_info[num_monitors].DeviceName, DisplayDevice.DeviceName);    
      monitor_info[num_monitors].original_x      =   deviceMode[num_monitors].dmPosition.x; 
      monitor_info[num_monitors].original_y      =   deviceMode[num_monitors].dmPosition.y; 
      monitor_info[num_monitors].original_width   =   deviceMode[num_monitors].dmPelsWidth; 
      monitor_info[num_monitors].original_height   =   deviceMode[num_monitors].dmPelsHeight; 

      if (deviceMode[num_monitors].dmPosition.x == 0 && deviceMode[num_monitors].dmPosition.y ==0) 
         original_primary = num_monitors;  // Primary monitor is one with 0,0 xy position 
      else 
         original_secondary = num_monitors; 

      strcat (monitorstring, va("Monitor #%i %s %s x y w h %i %i %i %i\n", 
      num_monitors+1, 
      monitor_info[num_monitors].DeviceName, 
      num_monitors == original_primary ? "(Primary)" : num_monitors == original_secondary ? "(Secondary)" : "(Neither)", 
      monitor_info[num_monitors].original_x, 
      monitor_info[num_monitors].original_y, 
      monitor_info[num_monitors].original_width, 
      monitor_info[num_monitors].original_height)); 

      num_monitors ++; 
       
   } 
   MSGBOX (va("Num monitors is %i", num_monitors)); 
   MSGBOX (va("Monitor infos \n\n%s", monitorstring)); 
   MSGBOX (va("Primary Monitor is %i and secondary is %i", original_primary, original_secondary)); 

} 

void SetPrimaryMonitorOnDeviceName (const char *NewPrimaryMonitor, const char *NewSecondaryMonitor, int SecondaryX, int SecondaryY) 
{ 
   DEVMODE deviceMode_newSecondary; 
   DEVMODE deviceMode_newPrimary; 

   if (num_monitors == 0) 
   { 
      MSGBOX ("No monitors ... need to call CollectMonitorInformation first") 
      return; 
   } 

   MSGBOX (va("Setting monitor %s to primary monitor", NewPrimaryMonitor)) 

   // Get primary infos 
   deviceMode_newPrimary.dmSize = sizeof(DEVMODE); 
   deviceMode_newPrimary.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS; 
   EnumDisplaySettings(NewPrimaryMonitor, ENUM_CURRENT_SETTINGS, &deviceMode_newPrimary); 

   // Get secondary infos 
   deviceMode_newSecondary.dmSize = sizeof(DEVMODE); 
   deviceMode_newSecondary.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS; 
   EnumDisplaySettings(NewSecondaryMonitor, ENUM_CURRENT_SETTINGS, &deviceMode_newSecondary); 

   // move old primary display to new position 
   deviceMode_newSecondary.dmFields = DM_POSITION; 
   deviceMode_newSecondary.dmPosition.x = deviceMode_newPrimary.dmPelsWidth; 
   deviceMode_newSecondary.dmPosition.y = 0; 

   ChangeDisplaySettingsEx(NewSecondaryMonitor, &deviceMode_newSecondary, NULL, CDS_UPDATEREGISTRY | CDS_NORESET, NULL); 

   // move old secondary display to 0,0 
   deviceMode_newPrimary.dmFields = DM_POSITION; 
   deviceMode_newPrimary.dmPosition.x = 0; 
   deviceMode_newPrimary.dmPosition.y = 0; 

   // CDS_UPDATEREGISTRY | CDS_NORESET = Have settings updated in registry, but CDS_NORESET means it won't take effect yet 
   ChangeDisplaySettingsEx(NewPrimaryMonitor, &deviceMode_newPrimary, NULL, CDS_SET_PRIMARY | CDS_UPDATEREGISTRY | CDS_NORESET , NULL); 
   ChangeDisplaySettingsEx (NULL, NULL, NULL, 0, NULL);  // Force it to take effect 

//   Sleep (500); // Let it think a minute 
} 

void SwapPrimaryMonitor (void)  // In a dual monitor situation, use 0 = first monitor or 1 = second monitor, etc. 
{ 
   // Set secondary monitor to primary 
   SetPrimaryMonitorOnDeviceName (monitor_info[original_secondary].DeviceName, monitor_info[original_primary].DeviceName, monitor_info[original_secondary].original_width, monitor_info[original_secondary].original_height); 
   monitors_swapped = 1; 
} 

void RestorePrimaryMonitor (void) 
{ 
   // Set original primary monitor back to primary 
   SetPrimaryMonitorOnDeviceName (monitor_info[original_primary].DeviceName, monitor_info[original_secondary].DeviceName, monitor_info[original_primary].original_width, monitor_info[original_primary].original_height); 
   monitors_swapped = 0; 
}