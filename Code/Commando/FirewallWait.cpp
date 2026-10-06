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
*     $Archive: /Commando/Code/Commando/FirewallWait.cpp $
*
* DESCRIPTION
*     Firewall negotiation wait condition.
*
* PROGRAMMER
*     $Author: Tom_s $
*
* VERSION INFO
*     $Revision: 16 $
*     $Modtime: 3/04/02 11:45a $
*
******************************************************************************/

#include "always.h"
#include "FirewallWait.h"
#include "Platform/Platform.h"
#include <iterator>
#include "nat.h"
#include	"string_ids.h"
#include "translatedb.h"
#include "natter.h"
#include <WWOnline/WOLSession.h>
#include <wwdebug/wwdebug.h>

#ifdef _MSC_VER
#pragma warning (push,3)
#endif

#include "systimer.h"

#ifdef _MSC_VER
#pragma warning (pop)
#endif


/*
** Wait code for firewall/NAT detection.
**
**
**
*/

RefPtr<FirewallDetectWait> FirewallDetectWait::Create(void)
	{
	return new FirewallDetectWait();
	}


FirewallDetectWait::FirewallDetectWait(void) :
		SingleWait(TRANSLATION(IDS_FIREWALL_NEGOTIATING_FIREWALL), 60000),
		mEvent(),
		mPingsRemaining(UINT_MAX)
	{
	mWOLSession = WWOnline::Session::GetInstance(false);
	assert(mWOLSession.IsValid());
	}


FirewallDetectWait::~FirewallDetectWait()
	{
	WWDEBUG_SAY(("FirewallDetectWait: End - %S\n", mEndText.Peek_Buffer()));

	mWOLSession->EnablePinging(true);


	}


void FirewallDetectWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("FirewallDetectWait: Beginning\n"));

	mEvent = Platform::MakeEvent();

	if (!mEvent)
		{
		WWDEBUG_SAY(("FirewallDetectWait: Can't create event\n"));
		EndWait(Error, TRANSLATION(IDS_FIREWALL_CREATE_EVENT_FAILED));
		}
	else
		{
		mWOLSession->EnablePinging(false);
		mTimeout = 15000;
		}
	}


WaitCondition::WaitResult FirewallDetectWait::GetResult(void)
	{
	if (mEndResult == Waiting)
		{
		// Wait for pending pings to finish first
		unsigned int pingsWaiting = mWOLSession->GetPendingPingCount();

		if (mPingsRemaining != pingsWaiting)
			{
			mPingsRemaining = pingsWaiting;
			mTimeout = 60000;
			FirewallHelper.Detect_Firewall(mEvent);
			}

		if (mPingsRemaining == 0)
			{
			if (mEvent && mEvent->IsSignaled())
				{
				WWDEBUG_SAY(("FirewallDetectWait: ConditionMet\n"));
				WOLNATInterface.Save_Firewall_Info_To_Settings();
				EndWait(ConditionMet, TRANSLATION(IDS_FIREWALL_NEGOTIATION_COMPLETE));
				}

			}
		}

	if (mEndResult != Waiting)
		{
		mWOLSession->EnablePinging(true);
		}

	return mEndResult;
	}


/*
** Wait code for clients when trying to open up a firewall for a server connection.
**
*/
RefPtr<FirewallConnectWait> FirewallConnectWait::Create(void)
	{
	return new FirewallConnectWait;
	}


FirewallConnectWait::FirewallConnectWait(void) :
		SingleWait(TRANSLATION(IDS_FIREWALL_NEGOTIATING_WITH_SERVER)),
		mEvent(),
		mCancelEvent(),
		mSuccessFlag(FirewallHelperClass::FW_RESULT_UNKNOWN),
		mQueueCount(0),
		mLastQueueCount(0),
		mPingsRemaining(UINT_MAX)
	{
	mWOLSession = WWOnline::Session::GetInstance(false);
	assert(mWOLSession.IsValid());
	}


FirewallConnectWait::~FirewallConnectWait()
	{
	WWDEBUG_SAY(("FirewallConnectWait: End - %S\n", mEndText.Peek_Buffer()));

	mWOLSession->EnablePinging(true);



	FirewallHelper.Set_Client_Connect_Event({}, {}, nullptr, nullptr);
	}


void FirewallConnectWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("FirewallConnectWait: Beginning\n"));

	mEvent = Platform::MakeEvent();
	mCancelEvent = Platform::MakeEvent();

	if (!mEvent || !mCancelEvent)
		{
		WWDEBUG_SAY(("FirewallConnectWait: Can't create event\n"));
		EndWait(Error, TRANSLATION(IDS_FIREWALL_CREATE_EVENT_FAILED));
		}
	else
		{
		mWOLSession->EnablePinging(false);
		WOLNATInterface.Tell_Server_That_Client_Is_In_Channel();
		mTimeout = 15000;
		mStartTime = TIMEGETTIME();
		}
	}


WaitCondition::WaitResult FirewallConnectWait::GetResult(void)
	{
	if (mEndResult == Waiting)
		{
		// Wait for pending pings to finish first
		unsigned int pingsWaiting = mWOLSession->GetPendingPingCount();

		if (mPingsRemaining != pingsWaiting)
			{
			mPingsRemaining = pingsWaiting;

			if (mPingsRemaining == 0)
				{
				FirewallHelper.Set_Client_Connect_Event(mEvent, mCancelEvent, &mSuccessFlag, &mQueueCount);
				mTimeout = 32000;
				mStartTime = TIMEGETTIME();
				}
			}

		if (mPingsRemaining == 0)
			{
			if ((TIMEGETTIME() - mStartTime) > mTimeout)
				{
				EndWait(TimeOut, TRANSLATION(IDS_FIREWALL_PORT_NEGOTIATION_TIMEOUT));
				}
			else
				{
				// Maybe change the wait text if there are players queued in front of us.
				const auto queueCount = mQueueCount.load();
				if (queueCount != mLastQueueCount)
					{
					wchar_t temp[256];
					swprintf(temp, std::size(temp), TRANSLATION(IDS_FIREWALL_QUEUE_NOTIFICATION), queueCount);
					WideStringClass text(temp, true);
					SetWaitText(text);
					mLastQueueCount = queueCount;
					mTimeout = max((unsigned)32000, ((queueCount * 32000) + 32000));
					mStartTime = TIMEGETTIME();
					}

				if (mEvent && mEvent->IsSignaled())
					{
					if (mSuccessFlag == FirewallHelperClass::FW_RESULT_SUCCEEDED)
						{
						WWDEBUG_SAY(("FirewallConnectWait: ConditionMet\n"));
						EndWait(ConditionMet, TRANSLATION(IDS_FIREWALL_PORT_NEGOTIATION_COMPLETE));
						}
					else
						{
						assert(mSuccessFlag == FirewallHelperClass::FW_RESULT_FAILED);
						WWDEBUG_SAY(("FirewallConnectWait: ConditionMet\n"));
						EndWait(Error, TRANSLATION(IDS_FIREWALL_PORT_NEGOTIATION_FAILED));
						}
					}

				}
			}
		}

	if (mEndResult != Waiting)
		{
		mWOLSession->EnablePinging(true);
		}

	return mEndResult;
	}



//
// Override base class end wait to check for cancel being pressed.
//
void FirewallConnectWait::EndWait(WaitResult result, const wchar_t* endText)
	{
	WWDEBUG_SAY(("FirewallConnectWait: EndWait\n"));

	if (result == UserCancel || result == TimeOut)
		{
		// Tell the firewall negotiation code to give up.
		if (mCancelEvent) mCancelEvent->Signal();
		}

		// Give the firewall code a little time to respond then remove it's cancel event anyway. It'll figure it out.
		for (int i=0 ; i<100 ; i++)
			{
			if (mSuccessFlag == FirewallHelperClass::FW_RESULT_CANCELLED)
				{
				break;
				}

			Platform::Sleep(1);
			}

	FirewallHelper.Set_Client_Connect_Event({}, {}, nullptr, nullptr);

	SingleWait::EndWait(result, endText);
	}