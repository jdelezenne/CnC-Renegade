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

#include "DlgWebPage.h"
#include "WebBrowser.h"
#include "DlgMessageBox.h"
#include "resource.h"
#include <Combat/string_ids.h>
#include "wwdebug.h"

void DlgWebPage::DoDialog(const char* page)
{
    WWASSERT_PRINT(page && *page, "Invalid parameter.\n");
    if (!page || !*page) return;
    auto* dialog = new DlgWebPage;
    dialog->mPage = page;
    if (!DlgMsgBox::DoDialog(0, IDS_WEB_LAUNCHBROWSER, DlgMsgBox::YesNo,
        static_cast<Observer<DlgMsgBoxEvent>*>(dialog))) dialog->Release_Ref();
}

DlgWebPage::DlgWebPage() : DialogBaseClass(IDD_WEBPAGE)
{
    WWDEBUG_SAY(("Instantiating DlgWebPage\n"));
}

DlgWebPage::~DlgWebPage()
{
    WWDEBUG_SAY(("Destroying DlgWebPage\n"));
}

void DlgWebPage::HandleNotification(DlgMsgBoxEvent& event)
{
    if (event.Event() == DlgMsgBoxEvent::Yes) {
        if (!WebBrowser::ShowWebPage(mPage)) DlgMsgBox::DoDialog(IDS_WEB_ERROR, IDS_WEB_PAGEFAILED);
        Release_Ref();
    } else if (event.Event() == DlgMsgBoxEvent::No) {
        Release_Ref();
    }
}
