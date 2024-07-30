/*****************************************************************
|
|   Platinum - main
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/command_line.h"


#include "PltMicroMediaController.h"
#include "PltVersion.h"

#include "DummyPltMediaRenderer.h"

#define BROADCAST_EXTRA 1


int main(int  argc , char** argv) {
  base::CommandLine::Init(argc, argv);
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  std::string uuid;
  if (!command_line || !command_line->HasSwitch("uuid")) {
    // return 1;
  }
  uuid = command_line->GetSwitchValueASCII("uuid");
  PLT_UPnP upnp;
  PLT_DeviceHostReference device(new SimulationMediaRenderer(uuid.c_str()));
  upnp.AddDevice(device);
  bool added = true;
  upnp.Start();
  char buf[256];
  while (true) {
    fgets(buf, 256, stdin);
    if (*buf == 'q')
      break;

    if (*buf == 's') {
      if (added) {
        upnp.RemoveDevice(device);
      } else {
        upnp.AddDevice(device);
      }
      added = !added;
    }
  }

  upnp.Stop();
  return 0;
}


/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
int main2(int argc, const char* argv[]) {
  system("chcp 65001");
  // setup Neptune logging
  NPT_LogManager::GetDefault().Configure(
      "plist:.level=INFO;.handlers=ConsoleHandler;.ConsoleHandler.colors=off");

  // Create upnp engine
  PLT_UPnP upnp;
  // Create control point
  PLT_CtrlPointReference ctrlPoint(new PLT_CtrlPoint());

  // Create controller
  PLT_MicroMediaController controller(ctrlPoint);
  // add control point to upnp engine and start it
  upnp.AddCtrlPoint(ctrlPoint);

  upnp.Start();

#ifdef BROADCAST_EXTRA
  // tell control point to perform extra broadcast discover every 6 secs
  // in case our device doesn't support multicast
  ctrlPoint->Discover(NPT_HttpUrl("255.255.255.255", 1900, "*"),
                      "upnp:rootdevice", 1, NPT_TimeStamp(6.));
  ctrlPoint->Discover(NPT_HttpUrl("239.255.255.250", 1900, "*"),
                      "upnp:rootdevice", 1, NPT_TimeStamp(6.));
#endif

  // start to process commands
  controller.ProcessCommandLoop();

  // stop everything
  upnp.Stop();

  return 0;
}
