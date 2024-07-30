/*****************************************************************
|
|   Platinum - Micro Media Controller
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

#ifndef _MICRO_MEDIA_CONTROLLER_H_
#define _MICRO_MEDIA_CONTROLLER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Platinum.h"
#include "Neptune.h"
#include "PltMediaServer.h"
#include "PltSyncMediaBrowser.h"
#include "PltMediaController.h"


/*----------------------------------------------------------------------
|   definitions
+---------------------------------------------------------------------*/
typedef NPT_Map<NPT_String, NPT_String>              PLT_StringMap;
typedef NPT_Lock<PLT_StringMap>                      PLT_LockStringMap;
typedef NPT_Map<NPT_String, NPT_String>::Entry       PLT_StringMapEntry;

/*----------------------------------------------------------------------
|   PLT_MediaItemIDFinder
+---------------------------------------------------------------------*/
class PLT_MediaItemIDFinder
{
public:
    // methods
    PLT_MediaItemIDFinder(const char* object_id) : m_ObjectID(object_id) {}

    bool operator()(const PLT_MediaObject* const & item) const {
        return item->m_ObjectID.Compare(m_ObjectID, true) ? false : true;
    }

private:
    // members
    NPT_String m_ObjectID;
};

/*----------------------------------------------------------------------
|   PLT_MicroMediaController
+---------------------------------------------------------------------*/
class PLT_MicroMediaController :/* public PLT_SyncMediaBrowser,*/
                                 public PLT_MediaController,
                                 public PLT_MediaControllerDelegate
{
public:
    PLT_MicroMediaController(PLT_CtrlPointReference& ctrlPoint);
    virtual ~PLT_MicroMediaController();

    void ProcessCommandLoop();

    // PLT_MediaControllerDelegate methods
    bool OnMRAdded(PLT_DeviceDataReference& device);
    void OnMRRemoved(PLT_DeviceDataReference& device);
    void OnMRStateVariablesChanged(PLT_Service* /* service */, 
                                   NPT_List<PLT_StateVariable*>* /* vars */);
    void OnGetPositionInfoResult(NPT_Result /* res */,
                                 PLT_DeviceDataReference& /* device */,
                                 PLT_PositionInfo* /* info */,
                                 void* /* userdata */);
    virtual void OnGetTransportInfoResult(NPT_Result /* res */,
                                          PLT_DeviceDataReference& /* device */,
                                          PLT_TransportInfo* /* info */,
                                          void* /* userdata */);
    virtual void OnGetVolumeResult(NPT_Result /* res */,
                                   PLT_DeviceDataReference& /* device */,
                                   const char* /* channel */,
                                   NPT_UInt32 /* volume */,
                                   void* /* userdata */);
    virtual void OnGetTransportSettingsResult(
        NPT_Result /* res */,
        PLT_DeviceDataReference& /* device */,
        PLT_TransportSettings*  settings ,
        void* /* userdata */);
    
    // PLT_HttpClientTask method
    NPT_Result ProcessResponse(NPT_Result                    res,
                               const NPT_HttpRequest&        request,
                               const NPT_HttpRequestContext& context,
                               NPT_HttpResponse*             response);
    
private:
    const char* ChooseIDFromTable(PLT_StringMap& table);
  
    void        GetCurMediaRenderer(PLT_DeviceDataReference& renderer);

    PLT_DeviceDataReference ChooseDevice(const NPT_Lock<PLT_DeviceMap>& deviceList);

    // Command Handlers
    void    HandleCmd_help();
    void    HandleCmd_getmr();
    void    HandleCmd_setmr();
    void    HandleCmd_open(const char* command);
    void    HandleCmd_play();
    void    HandleCmd_pause();

    void    HandleCmd_seek(const char* command);
    void    HandleCmd_stop();
    void    HandleCmd_mute();
    void    HandleCmd_unmute();
    void HandleCmd_get_setting();

    void HandleCmd_get_position_info();
    void HandleCmd_get_transport_state();
    void HandleCmd_set_volume();
    void HandleCmd_get_volume();


private:
    /* Tables of known devices on the network.  These are updated via the
     * OnMSAddedRemoved and OnMRAddedRemoved callbacks.  Note that you should first lock
     * before accessing them using the NPT_Map::Lock function.
     */
    NPT_Lock<PLT_DeviceMap> m_MediaRenderers;

 

    /* Currently selected media renderer as well as 
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the 
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
    PLT_DeviceDataReference m_CurMediaRenderer;
    NPT_Mutex               m_CurMediaRendererLock;

    
};

#endif /* _MICRO_MEDIA_CONTROLLER_H_ */

