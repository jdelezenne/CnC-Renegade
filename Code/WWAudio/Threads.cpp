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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/Threads.cpp                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/17/99 3:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "Threads.h"
#include "refcount.h"
#include "Utils.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_error.h>
#include <stdexcept>
#include "wwdebug.h"
#include "systimer.h"


///////////////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::DELAYED_RELEASE_INFO *	WWAudioThreadsClass::m_ReleaseListHead	= NULL;
CriticalSectionClass		WWAudioThreadsClass::m_ListMutex;
SDL_Thread*						WWAudioThreadsClass::m_hDelayedReleaseThread	= NULL;
SDL_Semaphore*				WWAudioThreadsClass::m_hDelayedReleaseEvent	= NULL;
CriticalSectionClass		WWAudioThreadsClass::m_CriticalSection;
std::atomic<bool>			WWAudioThreadsClass::m_IsFlushing				= false;

///////////////////////////////////////////////////////////////////////////////////////////
//
//	WWAudioThreadsClass
//
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::WWAudioThreadsClass (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	~WWAudioThreadsClass
//
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::~WWAudioThreadsClass (void)
{
	return ;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Delayed_Release_Thread
//
///////////////////////////////////////////////////////////////////////////////////////////
SDL_Thread*
WWAudioThreadsClass::Create_Delayed_Release_Thread (void* param)
{
	//
	//	If the thread isn't already running, then
	//
	CriticalSectionClass::LockClass lock(m_CriticalSection);
	if (m_hDelayedReleaseThread == NULL) {
		m_hDelayedReleaseEvent = SDL_CreateSemaphore(0);
		if (!m_hDelayedReleaseEvent) throw std::runtime_error(SDL_GetError());
		m_IsFlushing = false;
		m_hDelayedReleaseThread = SDL_CreateThread(Delayed_Release_Thread_Proc, "Audio release", param);
		if (!m_hDelayedReleaseThread) {
			SDL_DestroySemaphore(m_hDelayedReleaseEvent);
			m_hDelayedReleaseEvent = NULL;
			throw std::runtime_error(SDL_GetError());
		}
	}

	return m_hDelayedReleaseThread;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	End_Delayed_Release_Thread
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::End_Delayed_Release_Thread (std::uint32_t timeout)
{
	//
	//	If the thread is running, then wait for it to finish
	//
	SDL_Thread* thread = NULL;
	{
		CriticalSectionClass::LockClass lock(m_CriticalSection);
		m_IsFlushing = true;
		thread = m_hDelayedReleaseThread;
		if (thread) SDL_SignalSemaphore(m_hDelayedReleaseEvent);
	}
	if (thread) {
		const auto start = Platform::Ticks();
		while (SDL_GetThreadState(thread) != SDL_THREAD_COMPLETE && Platform::Ticks() - start < timeout)
			Platform::Sleep(1);
		if (SDL_GetThreadState(thread) != SDL_THREAD_COMPLETE)
			SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "Waiting for audio release thread after %u ms", timeout);
		SDL_WaitThread(thread, NULL);
		CriticalSectionClass::LockClass lock(m_CriticalSection);
		SDL_DestroySemaphore(m_hDelayedReleaseEvent);
		m_hDelayedReleaseEvent = NULL;
		m_hDelayedReleaseThread = NULL;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Delayed_Release_Object
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::Add_Delayed_Release_Object
(
	RefCountClass *	object,
	std::uint32_t					delay
)
{
	bool release_now = false;
	{
		CriticalSectionClass::LockClass thread_lock(m_CriticalSection);
		if (!m_IsFlushing) Create_Delayed_Release_Thread();
		CriticalSectionClass::LockClass list_lock(m_ListMutex);
		release_now = m_IsFlushing;
		if (!release_now) {
			DELAYED_RELEASE_INFO* info = new DELAYED_RELEASE_INFO;
			info->object = object;
			info->time = TIMEGETTIME() + delay;
			info->next = m_ReleaseListHead;
			info->prev = NULL;
			if (info->next) info->next->prev = info;
			m_ReleaseListHead = info;
		}
	}
	if (release_now) REF_PTR_RELEASE(object);

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Delayed_Release_Objects
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::Flush_Delayed_Release_Objects (void)
{
	DELAYED_RELEASE_INFO *release_list = NULL;
	{
		CriticalSectionClass::LockClass lock(m_ListMutex);
		m_IsFlushing = true;
		release_list = m_ReleaseListHead;
		m_ReleaseListHead = NULL;
	}

	//
	//	Loop through all the objects in our delay list, and
	// free them now.
	//
	DELAYED_RELEASE_INFO *info = NULL;
	DELAYED_RELEASE_INFO *next = NULL;
	for (info = release_list; info != NULL; info = next) {
		next = info->next;

		//
		//	Free the object
		//
		REF_PTR_RELEASE (info->object);
		SAFE_DELETE (info);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Delayed_Release_Thread_Proc
//
///////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioThreadsClass::Delayed_Release_Thread_Proc (void* /*param*/)
{
	const std::uint32_t base_timeout = 2000;
	std::uint32_t timeout = base_timeout + rand () % 1000;

	//
	//	Keep looping forever until we are singalled to quit (or an error occurs)
	//
	while (!SDL_WaitSemaphoreTimeout(m_hDelayedReleaseEvent, timeout)) {

		DELAYED_RELEASE_INFO *release_list = NULL;
		DELAYED_RELEASE_INFO **release_tail = &release_list;
		{
			CriticalSectionClass::LockClass lock(m_ListMutex);

			//
			//	Loop through all the objects in our delay list, and
			// free any that have expired.
			//
			std::uint32_t current_time			= TIMEGETTIME ();
			DELAYED_RELEASE_INFO *curr = NULL;
			DELAYED_RELEASE_INFO *prev	= NULL;
			DELAYED_RELEASE_INFO *next	= NULL;
			for (curr = m_ReleaseListHead; curr != NULL; curr = next) {
				next = curr->next;
				prev = curr->prev;

				//
				//	If the time has expired, free the object
				//
				if (current_time >= curr->time) {

					//
					//	Unlink the object
					//
					if (curr == m_ReleaseListHead) {
						m_ReleaseListHead = next;
					}

					if (prev != NULL) {
						prev->next = next;
					}

					if (next != NULL) {
						next->prev = prev;
					}

					//
					//	Free the object
					//
					curr->next = NULL;
					*release_tail = curr;
					release_tail = &curr->next;
				}
			}
		}

		while (release_list != NULL) {
			DELAYED_RELEASE_INFO *info = release_list;
			release_list = info->next;
			REF_PTR_RELEASE (info->object);
			SAFE_DELETE (info);
		}

		//
		//	To avoid 'periodic' releases, randomize our timeout
		//
		timeout = base_timeout + rand () % 1000;
	}

	Flush_Delayed_Release_Objects ();
	return 0;
}

/*
///////////////////////////////////////////////////////////////////////////////////////////
//
//	Begin_Modify_List
//
///////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioThreadsClass::Begin_Modify_List (void)
{
	bool retval = false;

	//
	//	Wait for up to one second to modify the list object
	//
	if (m_ListMutex != NULL) {
		retval = (::WaitForSingleObject (m_ListMutex, 1000) == WAIT_OBJECT_0);
		WWASSERT (retval);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	End_Modify_List
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::End_Modify_List (void)
{
	//
	//	Release this thread's hold on the mutex object.
	//
	if (m_ListMutex != NULL) {
		::ReleaseMutex (m_ListMutex);
	}

	return ;
}
*/