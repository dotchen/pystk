//  $Id: xml_reader.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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
#ifdef HAVE_IRRLICHT
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"
#include "utils/vec3.hpp"

XMLNode::XMLNode(const std::string &name, io::IXMLReader *xml)
{
    m_name = name;
    for(unsigned int i=0; i<xml->getAttributeCount(); i++)
    {
        std::string   name  = core::stringc(xml->getAttributeName(i)).c_str();
        core::stringw value = xml->getAttributeValue(i);
        m_attributes[name] = value;
    }   // for i
}   // XMLNode

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, set 'value' to the value of the
*   attribute and return 1, otherwise return 0 and do not change value.
*  \param attribute Name of the attribute.
*  \param value Value of the attribute.
*/
int XMLNode::get(const std::string &attribute, std::string *value) const
{
    if(m_attributes.size()==0) return 0;
    std::map<std::string, core::stringw>::const_iterator o;
    o = m_attributes.find(attribute);
    if(o==m_attributes.end()) return 0;
    *value=core::stringc(o->second).c_str();
    return 1;
}   // get

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, core::vector3df *value) const
{
    std::string s = "";
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=3) return 0;
    value->X = (float)atof(v[0].c_str());
    value->Y = (float)atof(v[1].c_str());
    value->Z = (float)atof(v[2].c_str());
    return 1;
}   // get(vector3df)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, Vec3 *value) const
{
    std::string s = "";
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=3) return 0;
    value->setX((float)atof(v[0].c_str()));
    value->setY((float)atof(v[1].c_str()));
    value->setZ((float)atof(v[2].c_str()));
    return 1;
}   // get(Vec3)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, video::SColorf *color) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=4) return 0;
    color->set((float)atof(v[0].c_str()),
               (float)atof(v[1].c_str()),
               (float)atof(v[2].c_str()),
               (float)atof(v[3].c_str()));
    return 1;
}   // get(SColor)
// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, int *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;
    *value = atoi(s.c_str());
    return 1;
}   // get(int)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, float *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;
    *value = (float)atof(s.c_str());
    return 1;
}   // get(int)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, bool *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;
    *value = s==""     || s[0]=='T' || s[0]=='t' ||
             s=="true" || s=="TRUE" || s=="#t"   || s=="#T";
    return 1;
}   // get(bool)

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, split the value of the attribute by spaces,
 *  set value to this vector array and return the number of elements. Otherwise
 *  return 0 and do not change value.
 *  \param attribute Name of the attribute.
 *  \param value Value of the attribute.
 */
int XMLNode::get(const std::string &attribute, 
                 std::vector<std::string> *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    *value = StringUtils::split(s,' ');
    return value->size();
}   // get(vector<string>)

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, split the value of the attribute by spaces,
 *  set value to this vector array and return the number of elements. Otherwise
 *  return 0 and do not change value.
 *  \param attribute Name of the attribute.
 *  \param value Value of the attribute.
 */
int XMLNode::get(const std::string &attribute, 
                 std::vector<float> *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    value->clear();
    for(unsigned int i=0; i<v.size(); i++)
    {
        value->push_back((float)atof(v[i].c_str()));
    }
    return value->size();
}   // get(vector<float>)

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, split the value of the attribute by spaces,
 *  set value to this vector array and return the number of elements. Otherwise
 *  return 0 and do not change value.
 *  \param attribute Name of the attribute.
 *  \param value Value of the attribute.
 */
int XMLNode::get(const std::string &attribute, std::vector<int> *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    value->clear();
    for(unsigned int i=0; i<v.size(); i++)
    {
        value->push_back(atoi(v[i].c_str()));
    }
    return value->size();
}   // get(vector<int>)

// ----------------------------------------------------------------------------
/** Interprets the attributes 'x', 'y', 'z'  or 'h', 'p', 'r' as a 3d vector
 *  and set the corresponding elements of value. Not all values need to be
 *  defined as attributes (and the correspnding elements of the vector will
 *  not be changed). It returns a bit field for each defined value, i.e. if x 
 *  and y are defined, 3 is returned.
 *  \param value Vector to return the values in.
 */
int XMLNode::get(core::vector3df *value) const
{
    float f;
    int bits=0;
    core::vector3df result = *value;
    if(get("x", &f)) { value->X = f; bits |= 1; }
    if(get("h", &f)) { value->X = f; bits |= 1; }
    if(get("y", &f)) { value->Y = f; bits |= 2; }
    if(get("p", &f)) { value->Y = f; bits |= 2; }
    if(get("z", &f)) { value->Z = f; bits |= 4; }
    if(get("r", &f)) { value->Z = f; bits |= 4; }
    return bits;
}
// ----------------------------------------------------------------------------
#endif
