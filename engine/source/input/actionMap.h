//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _ACTIONMAP_H_
#define _ACTIONMAP_H_


#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _VECTOR_H_
#include "collection/vector.h"
#endif
#ifndef _SIMBASE_H_
#include "sim/simBase.h"
#endif

struct InputEvent;

struct EventDescriptor
{
   U8  flags;      ///< Combination of any modifier flags.
   U8  eventType;  ///< SI_KEY, etc.
   U16 eventCode;  ///< From event.h
};

/// Map raw inputs to a variety of actions.  This is used for all keymapping
/// in the engine.
/// @see ActionMap::Node
class ActionMap : public SimObject
{
   typedef SimObject Parent;

  protected:
   bool onAdd();

   struct Node {
      U32 modifiers;
      U32 action;

      enum Flags {
         Ranged      = BIT(0),   ///< Ranged input.
         HasScale    = BIT(1),   ///< Scaled input.
         HasDeadZone = BIT(2),   ///< Dead zone is present.
         Inverted    = BIT(3),   ///< Input is inverted.
		 NonLinear   = BIT(4),
         BindCmd     = BIT(5)    ///< Bind a console command to this.
      };

      U32 flags;           /// @see Node::Flags
      F32 deadZoneBegin;
      F32 deadZoneEnd;
      F32 scaleFactor;

      SimObject* object;                ///< Object to call consoleFunction on.
      StringTableEntry consoleFunction; ///< Console function to call with new values.

      char *makeConsoleCommand;         ///< Console command to execute when we make this command.
      char *breakConsoleCommand;        ///< Console command to execute when we break this command.
   };

   /// Used to represent a devices.
   struct DeviceMap
   {
      U32 deviceType;
      U32 deviceInst;

      Vector<Node> nodeMap;
      DeviceMap() {
         VECTOR_SET_ASSOCIATION(nodeMap);
      }
      ~DeviceMap();
   };
   struct BreakEntry
   {
      U32 deviceType;
      U32 deviceInst;
      U32 objInst;
      SimObject* object;
      StringTableEntry consoleFunction;
      char *breakConsoleCommand;

      // It's possible that the node could be deleted (unlikely, but possible,
      //  so we replicate the node flags here...
      //
      U32 flags;
      F32 deadZoneBegin;
      F32 deadZoneEnd;
      F32 scaleFactor;

      // [neo, 5/7/2007 - #2975]
      // object above can be deleted in between a make/break and so object will point
      // to turfed memory and crash. To keep things simple we just store id as well so
      // we can look it up to validate object ref.
      S32 objectId;
   };


   Vector<DeviceMap*>        mDeviceMaps;
   static Vector<BreakEntry> smBreakTable;

   // Find: return NULL if not found in current map, Get: create if not
   //  found.
   const Node* findNode(const U32 inDeviceType, const U32 inDeviceInst,
                        const U32 inModifiers,  const U32 inAction);
   bool findBoundNode( const char* function, U32 &devMapIndex, U32 &nodeIndex );
   bool nextBoundNode( const char* function, U32 &devMapIndex, U32 &nodeIndex );
   Node* getNode(const U32 inDeviceType, const U32 inDeviceInst,
                 const U32 inModifiers,  const U32 inAction,
                 SimObject* object = NULL);

   void removeNode(const U32 inDeviceType, const U32 inDeviceInst,
                 const U32 inModifiers,  const U32 inAction,
                 SimObject* object = NULL);

   void enterBreakEvent(const InputEvent* pEvent, const Node* pNode);

   static const char* getModifierString(const U32 modifiers);

   /// Pass index to a break entry, and this function will fire it off.
   static void fireBreakEvent(U32 idx, F32 value = 0.f);

  public:
   ActionMap();
   ~ActionMap();

   void dumpActionMap(const char* fileName, const bool append) const;

   static bool createEventDescriptor(const char* pEventString, EventDescriptor* pDescriptor);
   static const char* swapCtrlForCmd(const char* input); ///< A simple helper function that takes Ctrl and turns it to Cmd on Mac

   bool processBind(const U32 argc, const char** argv, SimObject* object = NULL);
   bool processBindCmd(const char *device, const char *action, const char *makeCmd, const char *breakCmd);
   bool processUnbind(const char *device, const char *action, SimObject* object = NULL);

   /// @name Console Interface Functions
   /// @{
   const char* getBinding( const char* command );                    ///< Find what the given command is bound to.
   const char* getCommand( const char* device, const char* action ); ///< Find what command is bound to the given event descriptor .
   bool    isInverted( const char* device, const char* action );
   F32   getScale( const char* device, const char* action );
   const char* getDeadZone( const char* device, const char* action );
   /// @}


   static bool        getKeyString(const U32 action, char* buffer);
   static bool        getDeviceName(const U32 deviceType, const U32 deviceInstance, char* buffer);
   static const char* buildActionString( const InputEvent* event );

   bool processAction(const InputEvent*);
   bool processGesture(const InputEvent*);
   bool processTouch(const InputEvent*);
   bool processButton(const InputEvent*);
   bool processMove(const InputEvent*);
   bool processMotion(const InputEvent*);
   bool processXInput(const InputEvent*);
   static bool checkBreakTable(const InputEvent*);   
   static bool handleEvent(const InputEvent*);
   static bool handleEventGlobal(const InputEvent*);

   static bool getDeviceTypeAndInstance(const char *device, U32 &deviceType, U32 &deviceInstance);

   DECLARE_CONOBJECT(ActionMap);
};

#endif // _ACTIONMAP_H_
