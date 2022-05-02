/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#pragma once

// String table - nicer to have these in one place
namespace Strings
{
    constexpr auto ACTION_COPY_PATCH        = "Copy Patch";
    constexpr auto ACTION_INIT_PATCH        = "Init Patch";
    constexpr auto ACTION_PASTE_PATCH       = "Paste Patch";
    constexpr auto BUTTON_CLOSE             = "Close";
    constexpr auto BUTTON_NO                = "No";
    constexpr auto BUTTON_YES               = "Yes";
    constexpr auto COULD_NOT_LOAD_ICON_FONT = "Could not load fa-solid-900.ttf - Icons will not look as intended";
    constexpr auto COULD_NOT_LOAD_MAIN_FONT = "Could not load DroidSans.ttf - Application will not look as intended";
    constexpr auto NO_AVAILABLE_PORTS       = "No available ports";
    constexpr auto STATUSBAR_DX7_MODE_ON    = "DX7 mode is ON";
    constexpr auto TITLE_DX7_COMPAT_MODE    = "DX7 Compatibility Mode";
    constexpr auto TITLE_ERROR              = "Error";
    constexpr auto TITLE_OPTIONS            = "Options";
    constexpr auto TOOLTOP_DX7_COMPAT_MODE  = "The Volca repurposes the DX7 checksum data to send the operator on/off status.\nCheck this box to send the message in the intended DX7 format.\nMost patches will sound wrong on the Volca when this is enabled.";
}
