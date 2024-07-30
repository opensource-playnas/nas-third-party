/*****************************************************************
|
|   Platinum - Miccro Media Controller
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
| 
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltMicroMediaController.h"
#include "PltLeaks.h"
#include "PltDownloader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

//NPT_SET_LOCAL_LOGGER("platinum.tests.micromediacontroller")

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaController::PLT_MicroMediaController(PLT_CtrlPointReference& ctrlPoint) :
  /*  PLT_SyncMediaBrowser(ctrlPoint),*/
    PLT_MediaController(ctrlPoint)
{
    PLT_MediaController::SetDelegate(this);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaController::~PLT_MicroMediaController()
{
}

/*
*  Remove trailing white space from a string
*/
static void strchomp(char* str)
{
    if (!str) return;
    char* e = str+NPT_StringLength(str)-1;

    while (e >= str && *e) {
        if ((*e != ' ')  &&
            (*e != '\t') &&
            (*e != '\r') &&
            (*e != '\n'))
        {
            *(e+1) = '\0';
            break;
        }
        --e;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ChooseIDFromTable
+---------------------------------------------------------------------*/
/* 
 * Presents a list to the user, allows the user to choose one item.
 *
 * Parameters:
 *      PLT_StringMap: A map that contains the set of items from
 *                        which the user should choose.  The key should be a unique ID,
 *                       and the value should be a string describing the item. 
 *       returns a NPT_String with the unique ID. 
 */
const char*
PLT_MicroMediaController::ChooseIDFromTable(PLT_StringMap& table)
{
    printf("Select one of the following:\n");

    NPT_List<PLT_StringMapEntry*> entries = table.GetEntries();
    if (entries.GetItemCount() == 0) {
        printf("None available\n"); 
    } else {
        // display the list of entries
        NPT_List<PLT_StringMapEntry*>::Iterator entry = entries.GetFirstItem();
        int count = 0;
        while (entry) {
            printf("%d)\t%s (%s)\n", ++count, (const char*)(*entry)->GetValue(), (const char*)(*entry)->GetKey());
            ++entry;
        }

        int index, watchdog = 3;
        char buffer[1024];

        // wait for input
        while (watchdog > 0) {
            fgets(buffer, 1024, stdin);
            strchomp(buffer);

            if (1 != sscanf(buffer, "%d", &index)) {
                printf("Please enter a number\n");
            } else if (index < 0 || index > count)  {
                printf("Please choose one of the above, or 0 for none\n");
                watchdog--;
                index = 0;
            } else {    
                watchdog = 0;
            }
        }

        // find the entry back
        if (index != 0) {
            entry = entries.GetFirstItem();
            while (entry && --index) {
                ++entry;
            }
            if (entry) {
                return (*entry)->GetKey();
            }
        }
    }

    return NULL;
}
/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRAdded
+---------------------------------------------------------------------*/
bool
PLT_MicroMediaController::OnMRAdded(PLT_DeviceDataReference& device)
{
    NPT_String uuid = device->GetUUID();

    // test if it's a media renderer
    PLT_Service* service;
    if (NPT_SUCCEEDED(device->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:*", service))) {
        NPT_AutoLock lock(m_MediaRenderers);
        m_MediaRenderers.Put(uuid, device);
    }
    
    return true;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRRemoved
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::OnMRRemoved(PLT_DeviceDataReference& device)
{
    NPT_String uuid = device->GetUUID();

    {
        NPT_AutoLock lock(m_MediaRenderers);
        m_MediaRenderers.Erase(uuid);
    }

    {
        NPT_AutoLock lock(m_CurMediaRendererLock);

        // if it's the currently selected one, we have to get rid of it
        if (!m_CurMediaRenderer.IsNull() && m_CurMediaRenderer == device) {
            m_CurMediaRenderer = NULL;
        }
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRStateVariablesChanged
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::OnMRStateVariablesChanged(PLT_Service*                  service,
                                                    NPT_List<PLT_StateVariable*>* vars)
{
    NPT_String uuid = service->GetDevice()->GetUUID();
    NPT_List<PLT_StateVariable*>::Iterator var = vars->GetFirstItem();
    while (var) {
        printf("Received state var \"%s:%s:%s\" changes: \"%s\"\n",
               (const char*)uuid,
               (const char*)service->GetServiceID(),
               (const char*)(*var)->GetName(),
               (const char*)(*var)->GetValue());
        ++var;
    }
}


std::string formatTime(double seconds) {
    int hours = static_cast<int>(seconds) / 3600;
    int minutes = (static_cast<int>(seconds) % 3600) / 60;
    int remaining_seconds = static_cast<int>(seconds) % 60;

    char buffer[9];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes,
                  remaining_seconds);
    return std::string(buffer);
}

void PLT_MicroMediaController::OnGetPositionInfoResult(
    NPT_Result  res ,
    PLT_DeviceDataReference&  device ,
    PLT_PositionInfo*  info ,
    void*  userdata ) {


  if (res != NPT_SUCCESS) {

  }

  printf("OnGetPositionInfoResult, prograss: %s / %s\n",
           formatTime(info->rel_time.ToSeconds()).c_str(),
           formatTime(info->track_duration.ToSeconds()).c_str());

}

void PLT_MicroMediaController::OnGetTransportInfoResult(
    NPT_Result  res ,
    PLT_DeviceDataReference&  device ,
    PLT_TransportInfo*  info ,
    void*  userdata ) {

  printf("OnGetTransportInfoResult, TransportState: %s ,TransportStatus: %s\n",
         info->cur_transport_state.GetChars(),
        info->cur_transport_status.GetChars());
}

void PLT_MicroMediaController::OnGetVolumeResult(
    NPT_Result  res ,
    PLT_DeviceDataReference&  device ,
    const char*  channel ,
    NPT_UInt32  volume ,
    void*  userdata ) {

  printf("OnGetVolumeResult, volume: %d\n", volume);
}

void PLT_MicroMediaController::OnGetTransportSettingsResult(
    NPT_Result /* res */,
    PLT_DeviceDataReference& /* device */,
    PLT_TransportSettings* settings,
    void* /* userdata */) {

   printf("OnGetTransportSettingsResult, play_mode: %s ,rec_quality_mode: %s\n",
      settings->play_mode.GetChars(), settings->rec_quality_mode.GetChars());
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetCurMediaRenderer
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetCurMediaRenderer(PLT_DeviceDataReference& renderer)
{
    NPT_AutoLock lock(m_CurMediaRendererLock);

    if (m_CurMediaRenderer.IsNull()) {
        printf("No renderer selected, select one with setmr\n");
    } else {
        renderer = m_CurMediaRenderer;
    }
}


/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_getmr
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_getmr()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        printf("Current media renderer: %s\n", (const char*)device->GetFriendlyName());
    } else {
        // this output is taken care of by the GetCurMediaRenderer call
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ChooseDevice
+---------------------------------------------------------------------*/
PLT_DeviceDataReference
PLT_MicroMediaController::ChooseDevice(const NPT_Lock<PLT_DeviceMap>& deviceList)
{
    PLT_StringMap            namesTable;
    PLT_DeviceDataReference* result = NULL;
    NPT_String               chosenUUID;

    // create a map with the device UDN -> device Name 
    const NPT_List<PLT_DeviceMapEntry*>& entries = deviceList.GetEntries();
    NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();
    while (entry) {
        PLT_DeviceDataReference device = (*entry)->GetValue();
        NPT_String              name   = device->GetFriendlyName();
        namesTable.Put((*entry)->GetKey(), name);

        ++entry;
    }

    // ask user to choose
    chosenUUID = ChooseIDFromTable(namesTable);
    if (chosenUUID.GetLength()) {
        deviceList.Get(chosenUUID, result);
    }

    return result?*result:PLT_DeviceDataReference(); // return empty reference if not device was selected
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_setmr
+---------------------------------------------------------------------*/
void 
PLT_MicroMediaController::HandleCmd_setmr()
{
    NPT_AutoLock lock(m_CurMediaRendererLock);

    m_CurMediaRenderer = ChooseDevice(m_MediaRenderers);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_open
+---------------------------------------------------------------------*/
void PLT_MicroMediaController::HandleCmd_open(const char* command) {
  NPT_String object_id;
    PLT_StringMap           tracks;
    PLT_DeviceDataReference device;

    GetCurMediaRenderer(device);
    if (device.IsNull()) {
        printf("not device");
        return;      
    }
    NPT_String target = command;
    NPT_List<NPT_String> args = target.Split(" ");
    if (args.GetItemCount() < 2)
        return;

    args.Erase(args.GetFirstItem());
    NPT_String play_url;
    args.Get(0, play_url);

    SetAVTransportURI(device, 0, play_url, nullptr, NULL);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_play
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_play()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        Play(device, 0, "1", NULL);
    }
}

void PLT_MicroMediaController::HandleCmd_pause() {
  PLT_DeviceDataReference device;
  GetCurMediaRenderer(device);
  if (!device.IsNull()) {
    Pause(device, 0,NULL);
  }

}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_seek
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_seek(const char* command)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // remove first part of command ("seek")
        NPT_String target = command;
        NPT_List<NPT_String> args = target.Split(" ");
        if (args.GetItemCount() < 2) return;

        args.Erase(args.GetFirstItem());
        target = NPT_String::Join(args, " ");
        
        Seek(device, 0, (target.Find(":")!=-1)?"REL_TIME":"X_DLNA_REL_BYTE", target, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_stop
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_stop()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        Stop(device, 0, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_mute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_mute()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        SetMute(device, 0, "Master", true, NULL);
    }

}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_unmute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_unmute()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        SetMute(device, 0, "Master", false, NULL);
    }
}

void PLT_MicroMediaController::HandleCmd_get_setting() {
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        GetTransportSettings(device, 0, NULL);
    }

}

void PLT_MicroMediaController::HandleCmd_get_position_info() {
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        GetPositionInfo(device, 0, nullptr);
    }


}

void PLT_MicroMediaController::HandleCmd_get_transport_state() {

  PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        NPT_String state;
        GetTransportInfo(device, 0, nullptr);
    }
}

void PLT_MicroMediaController::HandleCmd_set_volume() {
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        NPT_String state;
        SetVolume(device, 0, "Master", 20, nullptr);
    }
  

}

void PLT_MicroMediaController::HandleCmd_get_volume() {
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        NPT_String state;
        GetVolume(device, 0, "Master",nullptr);
    }

}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_help
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_help()
{
    printf("\n\nNone of the commands take arguments.  The commands with a * \n");
    printf("signify ones that will prompt the user for more information once\n");
    printf("the command is called\n\n");
    printf("The available commands are:\n\n");
    printf(" quit    -   shutdown the Control Point\n");
    printf(" exit    -   same as quit\n");

    
    printf(" setmr   - * select a media renderer to become the active media renderer\n");
    printf(" getmr   -   print the friendly name of the active media renderer\n");
    printf(" open    -   set the uri on the active media renderer\n");
    printf(" play    -   play the active uri on the active media renderer\n");
    printf(" stop    -   stop the active uri on the active media renderer\n");
    printf(" seek    -   issue a seek command\n");
    printf(" mute    -   mute the active media renderer\n");
    printf(" unmute  -   unmute the active media renderer\n");

    printf(" help    -   print this help message\n\n");
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ProcessCommandLoop
+---------------------------------------------------------------------*/
void 
PLT_MicroMediaController::ProcessCommandLoop()
{
    char command[2048];
    bool abort = false;

    command[0] = '\0';
    while (!abort) {
        printf("command> ");
        fflush(stdout);
        fgets(command, 2048, stdin);
        strchomp(command);

        if (0 == strcmp(command, "quit") || 0 == strcmp(command, "exit")) {
            abort = true;
        }  else if (0 == strcmp(command, "setmr")) {
            HandleCmd_setmr();
        } else if (0 == strcmp(command, "getmr")) {
            HandleCmd_getmr();
        } else if (0 == strncmp(command, "open", 4)) {
            HandleCmd_open(command);
        } else if (0 == strcmp(command, "play")) {
            HandleCmd_play();
        } else if (0 == strcmp(command, "pause")) {
            HandleCmd_pause();
        } else if (0 == strcmp(command, "stop")) {
            HandleCmd_stop();
        } else if (0 == strncmp(command, "seek", 4)) {
            HandleCmd_seek(command);
        } else if (0 == strcmp(command, "mute")) {
            HandleCmd_mute();
        } else if (0 == strcmp(command, "unmute")) {
            HandleCmd_unmute();
        } else if (0 == strcmp(command, "get_info")) {
            HandleCmd_get_position_info();
        } else if (0 == strcmp(command, "get_state")) {
            HandleCmd_get_transport_state();
        } else if (0 == strcmp(command, "set_vol")) {
            HandleCmd_set_volume();
        } else if (0 == strcmp(command, "get_vol")) {
            HandleCmd_get_volume();
        } else if (0 == strcmp(command, "get_setting")) {
            HandleCmd_get_setting();
        } else if (0 == strcmp(command, "help")) {
            HandleCmd_help();
        } else if (0 == strcmp(command, "")) {
            // just prompt again
        } else {
            printf("Unrecognized command: %s\n", command);
            HandleCmd_help();
        }
    }
}

