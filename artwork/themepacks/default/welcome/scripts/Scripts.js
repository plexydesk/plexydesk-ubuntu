/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@kde.org>
*  Authored By  : Varuna Lekamwasam <vrlekamwasam@gmail.com>
*                 PhobosK <phobosk@kbfx.net>
*
*  PlexyDesk is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  PlexyDesk is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with PlexyDesk. If not, see <http://www.gnu.org/licenses/lgpl.html>
*******************************************************************************/

.pragma library


var infoDesktop = "<p>PlexyDesk is an extension which lets you customize your desktop workspace.</p><p>With PlexyDesk you can add your own graphics, QML Widgets and even write extensions to change the way your desktop works.</p>";
var infoAddingWidget = "<p>Just \"Drag'n'Drop\" any QML Widget from your File Manager or the Web and watch the widget come alive!</p>";
var addWidgetCommand = "";
var infoDeleteWidget = "<p>Not supported Currently.</p>";
var infoChangeWallpaper = "<p>With PlexyDesk just \"Drag'n'Drop\" any image to change your wallpaper.</p>";
var infoDevelopment = "<p>If you are good or even interested in: VOIP Technologies, OpenGL, Mathematics and Physics, Have experience with libjingle (or you are willing to code using it), 2D Graphics and Artwork, GTK and Qt 4.x, Know how to write HOWTOS, Report Bugs and Beta testing experienced or Just good at gossiping?</p><p>We would like to hear from you. The PlexyDesk Team and the rest of the community will indeed appreciate your valuable contribution.</p>";
var infoWiki = "<br><p>Visit our Wiki at: <a href='http://redmine.plexyplanet.org/projects/plexydesk/wiki'>click here</a></p><p>Visit our Homepage at: <a href='http://www.plexyplanet.org/'>click here</a></p>";

function changeState(base,stt) {
    base.state = stt;
}

function showInfo(clicked,topic,content)
{
    if (clicked === "Desktop") {
        topic.text = "Desktop";
        content.text = infoDesktop;
        //code.text = "";
        }
    else if (clicked === "Adding Widgets") {
        topic.text = "Adding Widgets";
        content.text = infoAddingWidget;
        //code.text = addWidgetCommand;
        }
    else if (clicked === "Removing Widgets") {
        topic.text = "Removing Widgets";
        content.text = infoDeleteWidget;
        //code.text ="";
        }
    else if (clicked === "Changing Wallpaper") {
        topic.text = "Changing Wallpaper";
        content.text = infoChangeWallpaper;
        //code.text = "";
        }
    else if (clicked === "Development") {
        topic.text = "Development";
        content.text = infoDevelopment;
        //code.text = "";
        }
    else if (clicked === "Wiki") {
        topic.text = "Wiki";
        content.text = infoWiki;
        //code.text = "";
        }
}

function buttonClicked(base,toState,clickedButton,topic,content)
{
    changeState(base,toState);
    showInfo(clickedButton,topic,content);
    changeState(base,toState);
}

function onEnteredDo(back)
{
    back.color ="#b1b0b0";
    back.opacity = 1;
}

function onExitedDo(back)
{
    back.color ="#5c5c5c";
    back.opacity = 0.8;
}
