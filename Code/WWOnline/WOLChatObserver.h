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
* FILE
*     $Archive: /Commando/Code/WWOnline/WOLChatObserver.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 10/14/02 12:38p $
*
******************************************************************************/

#ifndef __WOLCHATOBSERVER_H__
#define __WOLCHATOBSERVER_H__

#include "Platform/Online/WOL.h"
#include "Platform/Online/Types.h"
#include <atomic>
#include "RefPtr.h"
#include "WOLUser.h"



namespace WWOnline {

class Session;
class SquadData;

class ChatObserver :
		public WOL::IChatEvent
	{
	public:
		ChatObserver();

		void Init(Session& outer);

		//---------------------------------------------------------------------------
		// IUnknown methods
		//---------------------------------------------------------------------------
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
		virtual ULONG STDMETHODCALLTYPE AddRef(void);
		virtual ULONG STDMETHODCALLTYPE Release(void);

		//---------------------------------------------------------------------------
		// IChatEvent Methods
		//---------------------------------------------------------------------------
		virtual HRESULT STDMETHODCALLTYPE OnServerList(HRESULT hr, WOL::Server* servers);
        
		virtual HRESULT STDMETHODCALLTYPE OnUpdateList(HRESULT hr, WOL::Update* updates);
    
		virtual HRESULT STDMETHODCALLTYPE OnServerError(HRESULT hr, LPCSTR ircmsg);
    
		virtual HRESULT STDMETHODCALLTYPE OnConnection(HRESULT hr, LPCSTR motd);
    
		virtual HRESULT STDMETHODCALLTYPE OnMessageOfTheDay(HRESULT hr, LPCSTR motd);
    
		virtual HRESULT STDMETHODCALLTYPE OnChannelList(HRESULT hr, WOL::Channel* channels);
    
		virtual HRESULT STDMETHODCALLTYPE OnChannelCreate(HRESULT hr, WOL::Channel* channel);
    
		virtual HRESULT STDMETHODCALLTYPE OnChannelJoin(HRESULT hr, WOL::Channel* channel, WOL::User* user);
    
		virtual HRESULT STDMETHODCALLTYPE OnChannelLeave(HRESULT hr, WOL::Channel* channel, WOL::User* user);
    
		virtual HRESULT STDMETHODCALLTYPE OnChannelTopic(HRESULT hr, WOL::Channel* channel, LPCSTR topic);
    
		virtual HRESULT STDMETHODCALLTYPE OnPrivateAction(HRESULT hr, WOL::User* user, LPCSTR action);
    
		virtual HRESULT STDMETHODCALLTYPE OnPublicAction(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR action);
    
		virtual HRESULT STDMETHODCALLTYPE OnUserList(HRESULT hr, WOL::Channel* channel, WOL::User* users);
    
		virtual HRESULT STDMETHODCALLTYPE OnPublicMessage(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR message);
    
		virtual HRESULT STDMETHODCALLTYPE OnPrivateMessage(HRESULT hr, WOL::User* user, LPCSTR message);
    
		virtual HRESULT STDMETHODCALLTYPE OnSystemMessage(HRESULT hr, LPCSTR message);
    
		virtual HRESULT STDMETHODCALLTYPE OnNetStatus(HRESULT hr);
    
		virtual HRESULT STDMETHODCALLTYPE OnLogout(HRESULT status, WOL::User* user);
    
		virtual HRESULT STDMETHODCALLTYPE OnPrivateGameOptions(HRESULT hr, WOL::User* user, LPCSTR options);
    
		virtual HRESULT STDMETHODCALLTYPE OnPublicGameOptions(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR options);
    
		virtual HRESULT STDMETHODCALLTYPE OnGameStart(HRESULT hr, WOL::Channel* channel, WOL::User* users, int gameid);
    
		virtual HRESULT STDMETHODCALLTYPE OnUserKick(HRESULT hr, WOL::Channel* channel, WOL::User* kicked, WOL::User* kicker);
    
		virtual HRESULT STDMETHODCALLTYPE OnUserIP(HRESULT hr, WOL::User* user);
    
		virtual HRESULT STDMETHODCALLTYPE OnFind(HRESULT hr, WOL::Channel* chan);
    
		virtual HRESULT STDMETHODCALLTYPE OnPageSend(HRESULT hr);
    
		virtual HRESULT STDMETHODCALLTYPE OnPaged(HRESULT hr, WOL::User* user, LPCSTR message);
    
		virtual HRESULT STDMETHODCALLTYPE OnServerBannedYou(HRESULT hr, WOL::time_t bannedTill);
    
		virtual HRESULT STDMETHODCALLTYPE OnUserFlags(HRESULT hr, LPCSTR name, unsigned int flags, unsigned int mask);
    
		virtual HRESULT STDMETHODCALLTYPE OnChannelBan(HRESULT hr, LPCSTR name, int banned);
    
		virtual HRESULT STDMETHODCALLTYPE OnSquadInfo(HRESULT hr, unsigned long id, WOL::Squad* squad);
    
		virtual HRESULT STDMETHODCALLTYPE OnUserLocale(HRESULT hr, WOL::User* users);
    
		virtual HRESULT STDMETHODCALLTYPE OnUserTeam(HRESULT hr, WOL::User* users);
    
		virtual HRESULT STDMETHODCALLTYPE OnSetLocale(HRESULT hr, WOL::Locale newlocale);
    
		virtual HRESULT STDMETHODCALLTYPE OnSetTeam(HRESULT hr, int newteam);

		virtual HRESULT STDMETHODCALLTYPE OnBuddyList(HRESULT hr, WOL::User* buddyList);
        
		virtual HRESULT STDMETHODCALLTYPE OnBuddyAdd(HRESULT hr, WOL::User* buddyAdded);
        
		virtual HRESULT STDMETHODCALLTYPE OnBuddyDelete(HRESULT hr, WOL::User* buddyDeleted);

		virtual HRESULT STDMETHODCALLTYPE OnPublicUnicodeMessage(HRESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* message);
        
		virtual HRESULT STDMETHODCALLTYPE OnPrivateUnicodeMessage(HRESULT hr, WOL::User* user, const unsigned short* message);
        
		virtual HRESULT STDMETHODCALLTYPE OnPrivateUnicodeAction(HRESULT hr, WOL::User* user, const unsigned short* action);
        
		virtual HRESULT STDMETHODCALLTYPE OnPublicUnicodeAction(HRESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* action);
        
		virtual HRESULT STDMETHODCALLTYPE OnPagedUnicode(HRESULT hr, WOL::User* user, const unsigned short* message);
        
		virtual HRESULT STDMETHODCALLTYPE OnServerTime(HRESULT hr, WOL::time_t stime);
        
		virtual HRESULT STDMETHODCALLTYPE OnInsiderStatus(HRESULT hr, WOL::User* users);
        
		virtual HRESULT STDMETHODCALLTYPE OnSetLocalIP(HRESULT hr, LPCSTR message);

		virtual HRESULT STDMETHODCALLTYPE OnChannelListBegin(HRESULT hr);
        
		virtual HRESULT STDMETHODCALLTYPE OnChannelListEntry(HRESULT hr, WOL::Channel* channel);
        
		virtual HRESULT STDMETHODCALLTYPE OnChannelListEnd(HRESULT hr);

	protected:
		virtual ~ChatObserver();

		// prevent copy and assignment
		ChatObserver(ChatObserver const &);
		ChatObserver const & operator =(ChatObserver const &);

		void AssignSquadToUsers(const UserList& users, const RefPtr<SquadData>& squad);
		void ProcessSquadRequest(const RefPtr<SquadData>& squad);
		void Kick_Spammer(WOL::User *wol_user);


	private:
		std::atomic<ULONG> mRefCount;
		Session* mOuter;
	};

}

#endif // __WOLCHATOBSERVER_H__