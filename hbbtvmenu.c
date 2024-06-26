
/*
 * hbbtvmenu.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */
#include "hbbtvmenu.h"
#include <vdr/menuitems.h>
#include <vdr/tools.h>
#include <vdr/device.h>
#include <vdr/plugin.h>
#include <string>
#include <cstring>

#ifdef DEBUG
#       define DSYSLOG(x...)    dsyslog(x);
#else
#       define DSYSLOG(x...)
#endif
using namespace std;

extern char *BROWSER;

#ifdef DEBUG
static const char *CtrlCodes[8] =
{  "Autostart",
   "Present",
   "Destroy",
   "Kill",
   "Prefetch",
   "Remote",
   "Disabled",
   "Playback_autostart"
};
#endif

cHbbtvMenu::cHbbtvMenu(const char *title, int c0, int c1, int c2, int c3, int c4)
:cOsdMenu(title, 4, 3, 2, 13, c4)
{
   hbbtvURLs = (cHbbtvURLs *)cHbbtvURLs::HbbtvURLs();
   SetHelp("Refresh");
}

cHbbtvMenu::~cHbbtvMenu()
{
}

void cHbbtvMenu::Display(void)
{
   int current;

   current = Current();
   Clear();

   SetTitle(*cString::sprintf("HbbTV URLs - Anzahl: %d", hbbtvURLs->Count()));
   DSYSLOG("[hbbtv] URL List %d entries --------------------------------------------", hbbtvURLs->Count());
   Add (new cOsdItem(*cString::sprintf("CtlC\tAID\tPri\tName\tURL")));
   //hbbtvURLs->Sort();
   for (cHbbtvURL *url = hbbtvURLs->First(); url; url = hbbtvURLs->Next(url)) {
      Add(new cOsdItem(*cString::sprintf("%s\t%d\t%d\t%s \t%s%s",  
          url->ControlCode() == 1 ? "Auto" : url->ControlCode() == 2 ? "Pres" : "unknown", url->ApplicationId(), url->Priority(),
          *url->Name(),  *url->UrlBase(),  *url->UrlLoc())));
      DSYSLOG("[hbbtv] CtrlCode=%-9s AppID=%2d Prio=%2d Name=%s URL=%s%s", CtrlCodes[url->ControlCode()-1], url->ApplicationId(), 
              url->Priority(), *url->Name(), *url->UrlBase(),  *url->UrlLoc());
   }
   SetCurrent(Get(current));
   cOsdMenu::Display();
}


eOSState cHbbtvMenu::ProcessKey(eKeys Key)
{
   eOSState state = cOsdMenu::ProcessKey(Key);

   if (state == osUnknown) {
      switch (Key) {
       case kOk:     {
                        cHbbtvURL *url = hbbtvURLs->Get(Current()-1);
                        if (url) 
                        {
                           struct sPlayerArgs {
                               string mMenuEntry;
                               string mPlayerCommand;
                               ePlayMode mPlayMode;
                               bool mSlaveMode;
                               bool mDeactivateRemotes;
                               bool mBlockMenu;
                           };

                           sPlayerArgs pa;
                           string disp = getenv("DISPLAY");

                           DSYSLOG("Menuitem: %d %s", Current(), *cString::sprintf("DISPLAY=%s %s %s%s", disp.c_str(), BROWSER, *url->UrlBase(),  *url->UrlLoc()));

                           pa.mMenuEntry = "hbbtv browser";
                           pa.mPlayerCommand = *cString::sprintf("DISPLAY=%s %s %s%s", disp.c_str(), BROWSER, *url->UrlBase(),  *url->UrlLoc());
                           pa.mPlayMode = pmExtern_THIS_SHOULD_BE_AVOIDED;
                           pa.mSlaveMode = false;
                           pa.mDeactivateRemotes = true;
                           pa.mBlockMenu = false;

                           if (!cPluginManager::CallFirstService("Run External", &pa)) {
                               SystemExec(*cString::sprintf("DISPLAY=%s %s %s%s", disp.c_str(), BROWSER, *url->UrlBase(),  *url->UrlLoc()), true);
                           }
                        }
                        return osEnd;
                     }
       case kRed:    Display();
                     break; 
       case kGreen:  break;
       case kYellow: break;
       case kBlue:   break;
       case kUp:     cDevice::SwitchChannel(1);
                     break;
       case kDown:   cDevice::SwitchChannel(-1);
                     break;
       default: break;
      }
      Display();
   }

   return state;
}
