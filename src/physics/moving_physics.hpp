//  $Id: moving_physics.hpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_MOVING_PHYSICS_HPP
#define HEADER_MOVING_PHYSICS_HPP
#include <string>
#ifdef HAVE_IRRLICHT
#include "irrlicht.h"
using namespace irr;
#endif
#define _WINSOCKAPI_
#include <plib/ssg.h>
#include "btBulletDynamicsCommon.h"
#include "callback.hpp"
#include "user_pointer.hpp"

class Vec3;
#ifdef HAVE_IRRLICHT
class scene::IAnimatedMesh;
class XMLNode;
#endif

#ifdef HAVE_IRRLICHT
class MovingPhysics : public Callback
#else
class MovingPhysics : public ssgTransform, public Callback
#endif
{
public:
    enum bodyTypes {MP_NONE, MP_CONE, MP_BOX, MP_SPHERE};

protected:
    bodyTypes             m_body_type;
    btCollisionShape     *m_shape;
    btRigidBody          *m_body;
    btDefaultMotionState *m_motion_state;
    float                 m_half_height;
    float                 m_mass;
    UserPointer           m_user_pointer;
    btTransform           m_init_pos;
#ifdef HAVE_IRRLICHT
    scene::IMesh         *m_mesh;
    scene::ISceneNode    *m_node;
#endif
public:
#ifdef HAVE_IRRLICHT
    MovingPhysics           (const XMLNode *node);
#else
    MovingPhysics           (const std::string data);
#endif
    ~MovingPhysics          (); 
    void update             (float dt);
    void         init       ();
    virtual void reset      ();
    btRigidBody *getBody    ()          { return m_body;           }
    const char  *getTypeName()          { return "moving physics"; }
    virtual void handleExplosion(const Vec3& pos, bool directHit);
};  // MovingPhysics

#endif
/* EOF */

