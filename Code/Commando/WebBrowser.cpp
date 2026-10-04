/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/******************************************************************************
*
* NAME
*     $Archive: /Commando/Code/Commando/WebBrowser.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 24 $
*     $Modtime: 1/18/02 10:31p $
*
******************************************************************************/

#include "Platform/Windows/Files.h"
#include "always.h"

#include "WebBrowser.h"
#include <WWLib\WWCOMUtil.h>
#include <WW3D2\WW3D.h>
#include <WWOnline\WOLLoginInfo.h>
#include <WWDebug\WWDebug.h>
#include "win.h"
#include "_globals.h"
#include "Settings.h"

WebBrowser* WebBrowser::_mInstance = NULL;

/******************************************************************************
*
* NAME
*     WebBrowser::InstallPrerequisites
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful.
*
******************************************************************************/

#ifdef _DEBUG
bool WebBrowser::InstallPrerequisites(void)
{
    SettingsClass settings(APPLICATION_SETTINGS_SECTION_URL);
		struct URLEntry
			{
			const char* Name;
			const char* Data;
			};

		static URLEntry urls[] =
			{
			{"BattleClans", "http://renchat2.westwood.com/cgi-bin/cgiclient?ren_clan_manager&request=expand_template&Template=index.html&SKU=3072&LANGCODE=0&embedded=1"},
			{"BattleClansX", "http://renchat2.westwood.com/cgi-bin/cgiclient?ren_clan_manager&request=expand_template&Template=index.html&SKU=3072&LANGCODE=0"},
			{"Ladder", "http://renchat2.westwood.com/renegade_embedded/index.html"},
			{"LadderX", "http://renchat2.westwood.com/renegade/index.html"},
			{"NetStatus", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_netstatus&LANGCODE=0&SKU=3072&embedded=1"},
			{"NetStatusX", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_netstatus&LANGCODE=0&SKU=3072"},
			{"News", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_news&LANGCODE=0&SKU=3072&embedded=1"},
			{"NewsX", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_news&LANGCODE=0&SKU=3072"},
			{"Signup", "http://games2.westwood.com/cgi-bin/cgiclient?ren_reg2&request=expand_template&Template=newreg_menu.html&LANGCODE=0&embedded=1&SKU=3072"},
			{"SignupX", "http://games2.westwood.com/cgi-bin/cgiclient?ren_reg2&request=expand_template&Template=newreg_menu.html&LANGCODE=0"},
			{NULL, NULL}
			};

    for (int index = 0; urls[index].Name; ++index) {
        StringClass value;
        settings.Get_String(urls[index].Name, value, urls[index].Data);
        settings.Set_String(urls[index].Name, value);
    }
    return true;
}
#endif // _DEBUG


/******************************************************************************
*
* NAME
*     WebBrowser::IsWebPageDisplayed
*
* DESCRIPTION
*     Test if a web page is currently being displayed.
*
* INPUTS
*     NONE
*
* RESULT
*     True if page is displayed; otherwise false.
*
******************************************************************************/

bool WebBrowser::IsWebPageDisplayed(void)
	{
	return (_mInstance && _mInstance->UsingEmbeddedBrowser() && _mInstance->IsVisible());
	}


/******************************************************************************
*
* NAME
*     WebBrowser::CreateInstance
*
* DESCRIPTION
*     Create an instance of the embedded browser manager.
*
* INPUTS
*     NONE
*
* RESULT
*     Browser - Pointer to instance of browser.
*
******************************************************************************/

WebBrowser* WebBrowser::CreateInstance(HWND window)
	{
	if (_mInstance)
		{
		return NULL;
		}

	WebBrowser* object = new WebBrowser;

	if (object)
		{
		bool success = object->FinalizeCreate(window);

		if (!success)
			{
			WWDEBUG_SAY(("WebBrowser: Failed to initialize!\n"));
			object->Release();
			object = NULL;
			}
		}

	return object;
	}


/******************************************************************************
*
* NAME
*     WebBrowser::WebBrowser
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WebBrowser::WebBrowser() :
		mRefCount(1),
		mVisible(false),
		mSwitchedMode(false)
	{
	WWDEBUG_SAY(("WebBrowser: Instantiating\n"));
	memset(&mProcessInfo, 0, sizeof(mProcessInfo));
	_mInstance = this;
	}


/******************************************************************************
*
* NAME
*     WebBrowser::~WebBrowser
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WebBrowser::~WebBrowser()
	{
	WWDEBUG_SAY(("WebBrowser: Destructing\n"));

	_mInstance = NULL;

	if (mWOLBrowser)
		{
		WWDEBUG_SAY(("WebBrowser: Releasing WOLBrowser\n"));
		mWOLBrowser->RemoveObserver(static_cast<IWOLBrowserEvent*>(this));
		mWOLBrowser->Shutdown();
		}

	if (mSwitchedMode)
		{
		WW3D::Set_Device_Resolution(mRestoreWidth, mRestoreHeight, mRestoreBits, -1, true);
		}
	}


/******************************************************************************
*
* NAME
*     WebBrowser::FinalizeCreate
*
* DESCRIPTION
*     Perform post creation initialization.
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if finalization was successful; otherwise false.
*
******************************************************************************/

bool WebBrowser::FinalizeCreate(HWND window)
	{
	if (window)
		{
		RECT windowRect;
		GetClientRect(window, &windowRect);

		LONG windowWidth = (windowRect.right - windowRect.left);
		LONG windowHeight = (windowRect.bottom - windowRect.top);

		// If the display is smaller than 800 x 600 use the external browser.
		// Therefore we do NOT need to create the embedded browser component.
		if ((windowWidth >= 800) && (windowHeight >= 600))
			{
			// Create the embedded browser component.
			WWDEBUG_SAY(("WebBrowser: Creating WOLBrowser component\n"));

			HRESULT hr = CreateCOMObjectFromLibrary("OnlineBrowser.dll", CLSID_WOLBrowser,
					IID_IWOLBrowser, (void**)&mWOLBrowser);

			if (FAILED(hr))
				{
				WWDEBUG_SAY(("WebBrowser: Failed to create WOLBrowser component\n"));
				return false;
				}

			// If the display resolution is greater than 800 x 600 then set the
			// display mode to 800 x 600 if we are not in windowed mode.
			if ((windowWidth > 800) && (windowHeight > 600))
				{
				bool windowed = false;
				WW3D::Get_Device_Resolution(mRestoreWidth, mRestoreHeight, mRestoreBits, windowed);

				if (!windowed)
					{
					mSwitchedMode = true;
					WW3D::Set_Device_Resolution(800, 600, -1, -1, true);
					}
				}

			// Center webpage within window
			GetClientRect(window, &windowRect);
			windowWidth = (windowRect.right - windowRect.left);
			windowHeight = (windowRect.bottom - windowRect.top);

			RECT webRect;
			webRect.left = windowRect.left + ((windowWidth - 800) / 2);
			webRect.right = (webRect.left + 800);
			webRect.top = windowRect.top + ((windowHeight - 600) / 2);
			webRect.bottom = (webRect.top + 600);

			WWDEBUG_SAY(("WebBrowser: Starting up WOLBrowser\n"));
			mWOLBrowser->AddObserver(static_cast<IWOLBrowserEvent*>(this));

			// Initialize browser to cover dialog browser area
			mWOLBrowser->Startup(window, &webRect);
			mWOLBrowser->AllowPageCertification(TRUE);
			mWOLBrowser->SetUIFlags(UIFLAG_SCROLLBARS);
			}
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     WebBrowser::ShowWebPage
*
* DESCRIPTION
*     Display the specified web page.
*
* INPUTS
*     Page - Web page to display.
*
* RESULT
*     NONE
*
******************************************************************************/

bool WebBrowser::ShowWebPage(const char* page)
	{
	// Retrieve URL from page identifier
	char url[512];

	if (RetrievePageURL(page, url, sizeof(url)))
		{
		WWDEBUG_SAY(("WebBrowser: Showing webpage '%s' - %s\n", page, url));

		if (mWOLBrowser)
			{
			// Convert string to Unicode
			mbstowcs(mPendingURL, url, 512);

			char path[MAX_PATH];

			if (RetrieveHTMLPath(path, MAX_PATH))
				{
				wchar_t framePage[MAX_PATH];
				mbstowcs(framePage, path, MAX_PATH);
				wcscat(framePage, L"frame.htm");

				HRESULT hr = mWOLBrowser->Navigate(framePage, 0 , NULL);

				if (SUCCEEDED(hr))
					{
					Show();
					return true;
					}
				}
			}
		else
			{
			return LaunchExternal(url);
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     WebBrowser::Show
*
* DESCRIPTION
*     Show the web browser
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WebBrowser::Show(void)
	{
	if (mWOLBrowser)
		{
		mWOLBrowser->Show();
		mVisible = true;
		}
	}


/******************************************************************************
*
* NAME
*     WebBrowser::Hide
*
* DESCRIPTION
*     Hide the web browser
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WebBrowser::Hide(void)
	{
	if (mWOLBrowser)
		{
		mWOLBrowser->Hide();
		mVisible = false;
		}
	}


/******************************************************************************
*
* NAME
*     WebBrowser::RetrievePageURL
*
* DESCRIPTION
*     Retrieve the URL for the specified page type.
*
* INPUTS
*     Page - Page name identifier
*     URL  - Buffer to receive URL.
*     Size - Maximum size of buffer.
*
* RESULT
*     Success - True if sucessful.
*
******************************************************************************/

bool WebBrowser::RetrievePageURL(const char* page, char* url, int size)
	{
    SettingsClass settings(APPLICATION_SETTINGS_SECTION_URL);
    StringClass valueName(page);
    if (mWOLBrowser == NULL) valueName += "X";
    settings.Get_String(valueName, url, size);
    return url[0] != 0;
	}


/******************************************************************************
*
* NAME
*     WebBrowser::RetrieveHTMLPath
*
* DESCRIPTION
*     Retrieve the path string to the local HTML data.
*
* INPUTS
*     Path - Pointer to buffer to receive path.
*     Size - Size of buffer in characters.
*
* RESULT
*     Success - True if successful.
*
******************************************************************************/

bool WebBrowser::RetrieveHTMLPath(char* path, int size)
	{
	// Retrieve the full pathname for the current process.
	DWORD length = GetCurrentDirectory(size, path);

	if (length == 0)
		{
		Print_Win32Error(GetLastError());
		return false;
		}

	int dirSize = (length + strlen("\\HTML\\"));

	if (dirSize > size)
		{
		return false;
		}

	strcat(path, "\\HTML\\");

	return true;
	}


/******************************************************************************
*
* NAME
*     IUnknown::QueryInterface
*
* DESCRIPTION
*
* INPUTS
*     IID - Interface ID
*
* RESULT
*
******************************************************************************/

STDMETHODIMP WebBrowser::QueryInterface(REFIID iid, void** ppv)
	{
	*ppv = NULL;

	if ((iid == IID_IUnknown) || (iid == IID_IWOLBrowserEvent))
		{
		*ppv = static_cast<IWOLBrowserEvent*>(this);
		}
	else
		{
		return E_NOINTERFACE;
		}

	static_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IUnknown::AddRef
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

ULONG STDMETHODCALLTYPE WebBrowser::AddRef(void)
	{
	return ++mRefCount;
	}


/******************************************************************************
*
* NAME
*     IUnknown::Release
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

ULONG STDMETHODCALLTYPE WebBrowser::Release(void)
	{
	WWASSERT(mRefCount > 0 && "WebBrowser: Negative reference count");
	--mRefCount;

	if (mRefCount == 0)
		{
		delete this;
		return 0;
		}

	return mRefCount;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnScriptQuit
*
* DESCRIPTION
*     Handle quit notification from BrowserHost.
*
* INPUTS
*     Cancel - Cancel flag
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnScriptQuit(void)
	{
	AddRef();

	WebEvent event(WebEvent::Quit, static_cast<WebBrowser*>(this));
	NotifyObservers(event);

	Release();

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnBeforeNavigate
*
* DESCRIPTION
*     Invoked before navigating to a url.
*
* INPUTS
*     URL         - URL navigating to.
*     TargetFrame - Name of frame that navigation is happening in.
*     Cancel      - On return; Cancel flag
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnBeforeNavigate(const wchar_t* /* url */,
		const wchar_t* /* targetFrame */)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnDocumentComplete
*
* DESCRIPTION
*     Invoked when the web page is loaded, once for each frame.
*
* INPUTS
*     URL      - URL navigated to.
*     TopFrame - Flag indicating if this is the top most frame.
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnDocumentComplete(const wchar_t* url, BOOL isTopFrame)
	{
	WWDEBUG_SAY(("WebBrowser: OnDocumentComplete: %S\n", url));

	if (isTopFrame)
		{
		WWDEBUG_SAY(("WebBrowser: pending url: %S\n", mPendingURL));
		mWOLBrowser->Navigate(mPendingURL, 0 , L"Main");
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnDownloadBegin
*
* DESCRIPTION
*     Invoked at the beginning of a download (Image, Sound, etc...)
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnDownloadBegin(void)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnProgressChange
*
* DESCRIPTION
*     Invoked as download progress changes.
*
* INPUTS
*     Progress    - Current progress.
*     MaxProgress - Maximum progress.
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnProgressChange(long /* progress */, long /* progressMax */)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnDownloadComplete
*
* DESCRIPTION
*     Invoked when download is completed.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnDownloadComplete(void)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnNavigateComplete
*
* DESCRIPTION
*     Invoked when navigation is complete.
*
* INPUTS
*     URL
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnNavigateComplete(const wchar_t* /* url */)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnStatusTextChange
*
* DESCRIPTION
*     Invoked when the status text changes.
*
* INPUTS
*     Status - Status text string.
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnStatusTextChange(const wchar_t* /* statusText */)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnTitleChange
*
* DESCRIPTION
*     Invoked when the document title changes.
*
* INPUTS
*     Title - Title string.
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnTitleChange(const wchar_t* /* title */)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnNewWindow
*
* DESCRIPTION
*     Invoked when requesting to open a new window for web content.
*
* INPUTS
*     Allow - On return; Flag to allow or disallow opening of new window.
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnNewWindow(void)
	{
	return E_FAIL;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnShowMessage
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnShowMessage(const wchar_t* /* text */, const wchar_t* /* caption */,
		unsigned long /* type */, long* /* result */)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnFailedPageCertification
*
* DESCRIPTION
*     Invoked when the web page fails certification.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnFailedPageCertification(void)
	{
	WWDEBUG_SAY(("WebBrowser: !!!!! PAGE FAILED CERTIFICATION !!!!!\n"));

	AddRef();

	WebEvent event(WebEvent::CertificationFailed, static_cast<WebBrowser*>(this));
	NotifyObservers(event);

	Release();

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnErrorMsg
*
* DESCRIPTION
*     Error message has changed.
*
* INPUTS
*     Error - Error message string.
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnErrorMsg(const wchar_t* /* error */)
	{
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     IWOLBrowserEvent::OnRegisterLogin
*
* DESCRIPTION
*     Westwood online signup account registration request.
*     This message is invoked via script in the account signup page.
*
* INPUTS
*     Nick     - User nickname.
*     Password - User password.
*
* RESULT
*     NONE
*
******************************************************************************/

STDMETHODIMP WebBrowser::OnRegisterLogin(const wchar_t* nick , const wchar_t* pass )
	{
	WWDEBUG_SAY(("WebBrowser: Register WWOnline login: '%S' - '%S'\n", nick, pass));

	RefPtr<WWOnline::LoginInfo> login = WWOnline::LoginInfo::Create(nick, pass, false);

	if (login.IsValid())
		{
		login->Remember(true);
		}
	else
		{
		WWASSERT((!"WebBrowser: Failed to create new login."));
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     WebBrowser::LaunchExternal
*
* DESCRIPTION
*     Launch the external browser.
*
* INPUTS
*     URL - URL to navigate to.
*
* RESULT
*     Success - True if successful
*
******************************************************************************/

bool WebBrowser::LaunchExternal(const char* url)
{
    return url && *url && reinterpret_cast<INT_PTR>(ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL)) > 32;
}

/******************************************************************************
*
* NAME
*     WebBrowser::IsExternalBrowserRunning
*
* DESCRIPTION
*     Test if the external browser is running;
*
* INPUTS
*     NONE
*
* RESULT
*     Running - True if the external browser is running.
*
******************************************************************************/

bool WebBrowser::IsExternalBrowserRunning(void) const
	{
  DWORD wait = WaitForInputIdle(mProcessInfo.hProcess, 50);

	if (WAIT_TIMEOUT == wait)
		{
		return true;
		}

	DWORD active = 0;
	BOOL success = GetExitCodeProcess(mProcessInfo.hProcess, &active);
	WWASSERT_PRINT(success, "GetExitCodeProcess() Failed");

	if (success == FALSE)
		{
		Print_Win32Error(GetLastError());
		}

	return (STILL_ACTIVE == active);
	}