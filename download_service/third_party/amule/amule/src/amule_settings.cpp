#include "amule_settings.h"
#include "amule.h"
#include "Preferences.h"

AmuleSettings::AmuleSettings()
{

}

AmuleSettings::~AmuleSettings()
{

}


// **************************设置项****************************
void AmuleSettings::SetTcpListenPort(int port) {
  thePrefs::SetPort(port);
  // save the preferences on ok
  theApp->glob_prefs->Save();
}

void AmuleSettings::SetUdpListenPort(int port) {
  thePrefs::SetUDPPort(port);
  // save the preferences on ok
  theApp->glob_prefs->Save();
}

void AmuleSettings::SetMaxConnections(int connections) {
  thePrefs::SetMaxConnections(connections);
  // save the preferences on ok
  theApp->glob_prefs->Save();
}

void AmuleSettings::SetMaxUploadSpeed(int64_t speed) {
  thePrefs::SetMaxUpload(speed);
  // save the preferences on ok
  theApp->glob_prefs->Save();
}

void AmuleSettings::SetMaxDownloadSpeed(int64_t speed) {
  thePrefs::SetMaxDownload(speed);
  // save the preferences on ok
  theApp->glob_prefs->Save();
}
