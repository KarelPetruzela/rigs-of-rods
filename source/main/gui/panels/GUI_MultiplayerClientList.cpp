/*
    This source file is part of Rigs of Rods
    Copyright 2005-2012 Pierre-Michel Ricordel
    Copyright 2007-2012 Thomas Fischer

    For more information, see http://www.rigsofrods.org/

    Rigs of Rods is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3, as
    published by the Free Software Foundation.

    Rigs of Rods is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/

/// @file
/// @author Thomas Fischer (thomas{AT}thomasfischer{DOT}biz)
/// @date   7th of September 2009

/// @author Remake to DearIMGUI: Petr Ohlidal, 11/2019


#include "GUI_MultiplayerClientList.h"

#include "Application.h"
#include "BeamFactory.h"
#include "GUIManager.h"
#include "Language.h"
#include "Network.h"
#include "RoRFrameListener.h"

#include <vector>

using namespace RoR;
using namespace GUI;
using namespace Ogre;

void MpClientList::Draw()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;
    const float content_width = 200.f;
    ImGui::SetNextWindowContentWidth(content_width);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - (content_width + (2*ImGui::GetStyle().WindowPadding.x) + 15.f), 15.f));
    const ImVec4  PANEL_BG_COLOR = ImVec4(0.1f, 0.1f, 0.1f, 0.8f); // FIXME: copypaste from TopMenubar
    ImGui::PushStyleColor(ImGuiCol_WindowBg, PANEL_BG_COLOR);
    ImGui::Begin("Peers", nullptr, flags);

    std::vector<RoRnet::UserInfo> users = RoR::Networking::GetUserInfos();
    users.insert(users.begin(), RoR::Networking::GetLocalUserData());
    for (RoRnet::UserInfo const& user: users)
    {
        bool hovered = false;
        Ogre::TexturePtr flag_tex;
        Ogre::TexturePtr auth_tex;
        Ogre::TexturePtr down_tex;
        Ogre::TexturePtr up_tex;

        // Icon sizes: flag(16x11), auth(16x16), up(16x16), down(16x16)

        // Auth icon
        switch (user.authstatus)
        {
        case RoRnet::AUTH_ADMIN:  auth_tex = this->FetchIcon("flag_red.png"); break;
        case RoRnet::AUTH_MOD:    auth_tex = this->FetchIcon("flag_blue.png"); break;
        case RoRnet::AUTH_RANKED: auth_tex = this->FetchIcon("flag_green.png"); break;
        default:;
        }
        hovered |= this->DrawIcon(auth_tex, ImVec2(14.f, ImGui::GetTextLineHeight()));

        // Flag
        StringVector parts = StringUtil::split(user.language, "_");
        if (parts.size() == 2)
        {
            StringUtil::toLowerCase(parts[1]);
            flag_tex = this->FetchIcon((parts[1] + ".png").c_str());
            hovered |= this->DrawIcon(flag_tex, ImVec2(16.f, ImGui::GetTextLineHeight()));
        }

        // Stream state
        if (user.uniqueid != RoR::Networking::GetLocalUserData().uniqueid &&
            App::app_state.GetActive() != AppState::MAIN_MENU)
        {
            switch (App::GetSimController()->GetBeamFactory()->CheckNetworkStreamsOk(user.uniqueid))
            {
            case 0: down_tex = this->FetchIcon("arrow_down_red.png");  break;
            case 1: down_tex = this->FetchIcon("arrow_down.png");      break;
            case 2: down_tex = this->FetchIcon("arrow_down_grey.png"); break;
            default:;
            }
            

            switch (App::GetSimController()->GetBeamFactory()->CheckNetRemoteStreamsOk(user.uniqueid))
            {
            case 0: up_tex = this->FetchIcon("arrow_up_red.png");  break;
            case 1: up_tex = this->FetchIcon("arrow_up.png");      break;
            case 2: up_tex = this->FetchIcon("arrow_up_grey.png"); break;
            default:;
            }
        }
        // Always invoke to keep usernames aligned
        hovered |= this->DrawIcon(down_tex, ImVec2(8.f, ImGui::GetTextLineHeight()));
        hovered |= this->DrawIcon(up_tex, ImVec2(8.f, ImGui::GetTextLineHeight()));

        // Player name
        ColourValue col = Networking::GetPlayerColor(user.colournum);
        ImGui::TextColored(ImVec4(col.r, col.g, col.b, col.a), "%s", user.username);
        hovered |= ImGui::IsItemHovered();

        // Tooltip
        if (hovered)
        {
            ImGui::BeginTooltip();

            // TextDisabled() are captions, Text() are values

            ImGui::TextDisabled("%s: ",_L("user name"));
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(col.r, col.g, col.b, col.a), "%s", user.username);
            ImGui::Separator();

            ImGui::TextDisabled("%s", _L("user language: "));
            ImGui::SameLine();
            ImGui::Text("%s", parts[0].c_str());

            ImGui::TextDisabled("%s", _L("user country: "));
            ImGui::SameLine();
            ImGui::Text("%s", parts[1].c_str());
            if (flag_tex)
            {
                ImGui::SameLine();
                ImGui::Image(reinterpret_cast<ImTextureID>(flag_tex->getHandle()),
                    ImVec2(flag_tex->getWidth(), flag_tex->getHeight()));
            }

            ImGui::Separator();
            ImGui::TextDisabled("%s", _L("user authentication level"));
            if (auth_tex)
            {
                ImGui::Image(reinterpret_cast<ImTextureID>(auth_tex->getHandle()),
                    ImVec2(auth_tex->getWidth(), auth_tex->getHeight()));
                ImGui::SameLine();
            }
            switch (user.authstatus)
            {
            case RoRnet::AUTH_ADMIN:  ImGui::Text("%s", _L("Server Administrator")); break;
            case RoRnet::AUTH_MOD:    ImGui::Text("%s", _L("Server Moderator"));     break;
            case RoRnet::AUTH_RANKED: ImGui::Text("%s", _L("ranked user"));          break;
            case RoRnet::AUTH_NONE:   ImGui::Text("%s", _L("Guest"));                break;
            default:;
            }

            // Stream state
            if (user.uniqueid != RoR::Networking::GetLocalUserData().uniqueid &&
                App::app_state.GetActive() != AppState::MAIN_MENU)
            {
                ImGui::Separator();
                ImGui::TextDisabled("%s", _L("truck loading state"));
                if (down_tex)
                {
                    ImGui::Image(reinterpret_cast<ImTextureID>(down_tex->getHandle()),
                        ImVec2(down_tex->getWidth(), down_tex->getHeight()));
                    ImGui::SameLine();
                }
                switch (App::GetSimController()->GetBeamFactory()->CheckNetworkStreamsOk(user.uniqueid))
                {
                case 0: ImGui::Text("%s", _L("Truck loading errors")); break;
                case 1: ImGui::Text("%s", _L("Truck loaded correctly, no errors")); break;
                case 2: ImGui::Text("%s", _L("no truck loaded")); break;
                default:; // never happens
                }

                ImGui::TextDisabled("%s", _L("remote truck loading state"));
                if (up_tex)
                {
                    ImGui::Image(reinterpret_cast<ImTextureID>(up_tex->getHandle()),
                        ImVec2(up_tex->getWidth(), up_tex->getHeight()));
                    ImGui::SameLine();
                }
                switch (App::GetSimController()->GetBeamFactory()->CheckNetRemoteStreamsOk(user.uniqueid))
                {
                case 0: ImGui::Text("%s", _L("Remote Truck loading errors")); break;
                case 1: ImGui::Text("%s", _L("Remote Truck loaded correctly, no errors")); break;
                case 2: ImGui::Text("%s", _L("No Trucks loaded")); break;
                default:; // never happens
                }
            }

            ImGui::EndTooltip();
        }
    }

    if (RoR::Networking::GetNetQuality() != 0)
    {
        ImGui::Separator();
        ImGui::TextColored(App::GetGuiManager()->GetTheme().error_text_color, "<!> %s", _L("Slow  Network  Download"));
    }

    ImGui::End();
    ImGui::PopStyleColor(1); // WindowBg
}

Ogre::TexturePtr MpClientList::FetchIcon(const char* name)
{
    try
    {
        return Ogre::static_pointer_cast<Ogre::Texture>(
            Ogre::TextureManager::getSingleton().createOrRetrieve(name, "FlagsRG").first);
    }
    catch (...) {}

    return Ogre::TexturePtr(); // null
}

bool MpClientList::DrawIcon(Ogre::TexturePtr tex, ImVec2 reference_box)
{
    ImVec2 orig_pos = ImGui::GetCursorPos();
    bool hovered = false;
    if (tex)
    {
   // TODO: moving the cursor somehow deforms the image
   //     ImGui::SetCursorPosX(orig_pos.x + (reference_box.x - tex->getWidth()) / 2.f);
   //     ImGui::SetCursorPosY(orig_pos.y + (reference_box.y - tex->getHeight()) / 2.f);
        ImGui::Image(reinterpret_cast<ImTextureID>(tex->getHandle()), ImVec2(tex->getWidth(), tex->getHeight()));
        hovered = ImGui::IsItemHovered();
    }
    ImGui::SetCursorPosX(orig_pos.x + reference_box.x + ImGui::GetStyle().ItemSpacing.x);
    ImGui::SetCursorPosY(orig_pos.y);
    return hovered;
}
