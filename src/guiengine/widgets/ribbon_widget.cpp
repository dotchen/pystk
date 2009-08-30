//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "guiengine/widgets/ribbon_widget.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/my_button.hpp"
#include "io/file_manager.hpp"
using namespace GUIEngine;

#ifndef round
#  define round(x)  (floor(x+0.5f))
#endif

// -----------------------------------------------------------------------------
RibbonWidget::RibbonWidget(const RibbonType type)
{
    for (int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        m_selection[n] = -1;
    }
    m_selection[0] = 0; // only player 0 has a selection by default
    
    m_ribbon_type = type;
    m_focus = NULL;
    updateSelection();
    m_type = WTYPE_RIBBON;
}
// -----------------------------------------------------------------------------
void RibbonWidget::add()
{
    m_labels.clearWithoutDeleting();
    
    
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    
    int id = (m_reserved_id == -1 ? getNewID() : m_reserved_id);
    
    IGUIButton * btn = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, id, L"");
    m_element = btn;
    
    const int subbuttons_amount = m_children.size();
    
    // ---- check how much space each child button will take and fit them within available space
    int total_needed_space = 0;
    for (int i=0; i<subbuttons_amount; i++)
    {
        m_children[i].readCoords(this);
        
        if (m_children[i].m_type != WTYPE_ICON_BUTTON && m_children[i].m_type != WTYPE_BUTTON)
        {
            std::cerr << "/!\\ Warning /!\\ : ribbon widgets can only have (icon)button widgets as children " << std::endl;
            continue;
        }
        
        total_needed_space += m_children[i].w;
    }
    
    int free_h_space = w - total_needed_space;
    
    int biggest_y = 0;
    const int button_y = 10;
    float global_zoom = 1;
    
    const int min_free_space = 50;
    global_zoom = (float)w / (float)( w - free_h_space + min_free_space );
    free_h_space = (int)(w - total_needed_space*global_zoom);
    
    const int one_button_space = (int)round((float)w / (float)subbuttons_amount);
    
    // ---- add children
    for(int i=0; i<subbuttons_amount; i++)
    {
        
        const int widget_x = one_button_space*(i+1) - one_button_space/2;
        
        IGUIButton * subbtn;
        
        if(/*m_children[i].m_type == WTYPE_BUTTON*/ getRibbonType() == RIBBON_TABS)
        {
            rect<s32> subsize = rect<s32>(widget_x - one_button_space/2+2,  0,
                                          widget_x + one_button_space/2-2,  h);
            
            stringw& message = m_children[i].m_text;
            
            if(m_children[i].m_type == WTYPE_BUTTON)
            {
                subbtn = GUIEngine::getGUIEnv()->addButton(subsize, btn, getNewNoFocusID(), message.c_str(), L"");
                subbtn->setTabStop(false);
                subbtn->setTabGroup(false);
            }
            else if(m_children[i].m_type == WTYPE_ICON_BUTTON)
            {
                rect<s32> icon_part = rect<s32>(15,
                                                0,
                                                subsize.getHeight()+15,
                                                subsize.getHeight());
                rect<s32> label_part = rect<s32>(subsize.getHeight()+15,
                                                 0,
                                                 subsize.getWidth()-15,
                                                 subsize.getHeight());
                
                // use the same ID for all subcomponents; since event handling is done per-ID, no matter
                // which one your hover, this widget will get it
                int same_id = getNewNoFocusID();
                subbtn = GUIEngine::getGUIEnv()->addButton(subsize, btn, same_id, L"", L"");
                
                MyGUIButton* icon = new MyGUIButton(GUIEngine::getGUIEnv(), subbtn, same_id, icon_part, true);
                icon->setImage( GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" + m_children[i].m_properties[PROP_ICON]).c_str()) );
                icon->setUseAlphaChannel(true);
                icon->setDrawBorder(false);
                icon->setTabStop(false);
                
                IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), label_part,
                                                                              false /* border */,
                                                                              true /* word wrap */,
                                                                              subbtn, same_id);
                label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
                label->setTabStop(false);
                label->setNotClipped(true);
                m_labels.push_back(label);
                
                subbtn->setTabStop(false);
                subbtn->setTabGroup(false);
                
            }
            
            m_children[i].m_element = subbtn;
        }
        else if(m_children[i].m_type == WTYPE_ICON_BUTTON)
        {
            const bool has_label = m_children[i].m_text.size() > 0;
            
            // how much space to keep for the label under the button
            const int needed_space_under_button = has_label ? 30 : 10; // quite arbitrary for now
            // if button too high to fit, scale down
            float zoom = global_zoom;
            while(button_y + m_children[i].h*zoom + needed_space_under_button > h) zoom -= 0.01f;
            
            // ---- add bitmap button part
            const float image_w = m_children[i].w*zoom;
            rect<s32> subsize = rect<s32>(widget_x - (int)(image_w/2.0f), button_y,
                                          widget_x + (int)(image_w/2.0f), button_y + (int)(m_children[i].h*zoom));
            
            subbtn = new MyGUIButton(GUIEngine::getGUIEnv(), btn, getNewNoFocusID(), subsize, true);
            
            m_children[i].m_element = subbtn;
            subbtn->setUseAlphaChannel(true);
            subbtn->setImage( GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" + m_children[i].m_properties[PROP_ICON]).c_str()) );
            
            // ---- label part
            if(has_label)
            {
                subsize = rect<s32>(widget_x - one_button_space/2,
                                    (int)((button_y + m_children[i].h)*zoom) + 5 /* leave 5 pixels between button and label */,
                                    widget_x + (int)(one_button_space/2.0f), h);
                
                stringw& message = m_children[i].m_text;
                IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), subsize, false, true, btn);
                label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
                label->setTabStop(false);
                label->setNotClipped(true);
                
                m_labels.push_back(label);
                
                const int final_y = subsize.getHeight() + label->getTextHeight();
                if(final_y > biggest_y) biggest_y = final_y;
            }
            
            subbtn->setTabStop(false);
            subbtn->setTabGroup(false);
        }
        else
        {
            std::cerr << "/!\\ Warning /!\\ : Invalid contents type in ribbon" << std::endl;
        }
        
        
        m_children[i].id = subbtn->getID();
        m_children[i].m_event_handler = this;
    }// next sub-button
    
    id = m_element->getID();
    m_element->setTabOrder(id);
    m_element->setTabGroup(false);
    updateSelection();
}

// -----------------------------------------------------------------------------
void RibbonWidget::select(std::string item, const int playerID)
{
    const int subbuttons_amount = m_children.size();
    
    for (int i=0; i<subbuttons_amount; i++)
    {
        if (m_children[i].m_properties[PROP_ID] == item)
        {
            m_selection[playerID] = i;
            updateSelection();
            return;
        }
    }
    
}
// -----------------------------------------------------------------------------
bool RibbonWidget::rightPressed(const int playerID)
{
    m_selection[playerID]++;
    if (m_selection[playerID] >= m_children.size())
    {
        if (m_event_handler != NULL)
        {
            ((DynamicRibbonWidget*)m_event_handler)->scroll(1); // FIXME? - find cleaner way to propagate event to parent
            m_selection[playerID] = m_children.size()-1;
        }
        else m_selection[playerID] = 0;
    }
    updateSelection();
    if (playerID == 0) m_focus = m_children.get(m_selection[playerID]);
    
    return m_ribbon_type != RIBBON_TOOLBAR;
}
// -----------------------------------------------------------------------------
bool RibbonWidget::leftPressed(const int playerID)
{
    m_selection[playerID]--;
    if (m_selection[playerID] < 0)
    {
        if (m_event_handler != NULL)
        {
            ((DynamicRibbonWidget*)m_event_handler)->scroll(-1); // FIXME? - find cleaner way to propagate event to parent
            m_selection[playerID] = 0;
        }
        else m_selection[playerID] = m_children.size()-1;
    }
    updateSelection();
    
    if (playerID == 0) m_focus = m_children.get(m_selection[playerID]);
    
    return m_ribbon_type != RIBBON_TOOLBAR;
}
// -----------------------------------------------------------------------------
void RibbonWidget::focused(const int playerID)
{    
    Widget::focused(playerID);
    
    if (m_children.size() < 1) return; // empty ribbon
    
    if (m_focus == NULL) m_focus = m_children.get(m_selection[playerID]);
    
    if (m_event_handler != NULL)
    {
        if (playerID == 0) GUIEngine::getGUIEnv()->setFocus(m_focus->m_element);
                
        // FIXME : unclean, children ribbons shouldn't need to know about their parent
        ((DynamicRibbonWidget*)m_event_handler)->onRowChange( this, playerID );
    }
}
// -----------------------------------------------------------------------------
bool RibbonWidget::mouseHovered(Widget* child)
{
    const int subbuttons_amount = m_children.size();
    
    m_focus = child;
    
    for (int i=0; i<subbuttons_amount; i++)
    {
        if (m_children.get(i) == child)
        {
            // FIXME: don't hardcode player 0 there?
            if (m_selection[0] == i) return false; // was already selected, don't send another event
            if (m_ribbon_type == RIBBON_TOOLBAR) m_selection[0] = i; // don't change selection on hover for others
            break;
        }
    }
    updateSelection();
    return false;
}

// -----------------------------------------------------------------------------
const std::string& RibbonWidget::getSelectionIDString(const int playerID)
{
    static std::string empty;
    if (m_selection[playerID] == -1) return empty;
    return m_children[m_selection[playerID]].m_properties[PROP_ID];
}

// -----------------------------------------------------------------------------
void RibbonWidget::updateSelection()
{
    const int subbuttons_amount = m_children.size();
    
    for (int i=0; i<subbuttons_amount; i++)
    {
        // Player 0 selection
        m_children[i].m_selected = (i == m_selection[0]);
    }
    
    // FIXME: don't hardcode player 0
    if (subbuttons_amount > 0 && m_ribbon_type == RIBBON_TOOLBAR) m_focus = m_children.get(m_selection[0]);
}
// -----------------------------------------------------------------------------
bool RibbonWidget::transmitEvent(Widget* w, std::string& originator, const int playerID)
{
    const int subbuttons_amount = m_children.size();
    
    for (int i=0; i<subbuttons_amount; i++)
    {
        if (m_children[i].m_properties[PROP_ID] == originator)
        {
            m_selection[playerID] = i;
            break;
        }
    }
    
    updateSelection();
    
    // bring focus back to enclosing ribbon widget
    GUIEngine::getGUIEnv()->setFocus(m_element);
    
    return true;
}
// -----------------------------------------------------------------------------
void RibbonWidget::setLabel(const int id, irr::core::stringw new_name)
{
    if (m_labels.size() == 0) return; // ignore this call for ribbons without labels
    
    assert(id >= 0);
    assert(id < m_labels.size());
    m_labels[id].setText( new_name.c_str() );
    m_text = new_name;
}
// -----------------------------------------------------------------------------
int RibbonWidget::findItemNamed(const char* internalName)
{    
    const int size = m_children.size();
    for (int n=0; n<size; n++)
    {
        if (m_children[n].m_properties[PROP_ID] == internalName) return n;
    }
    return -1;
}
