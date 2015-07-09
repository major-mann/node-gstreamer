#ifndef NSTR_PAD_H
#define NSTR_PAD_H

#include "linked_object_base.h"
#include "pad_template.h"
#include "static_pad_template.h"
#include "caps.h"

//TODO: Events

namespace nstr {

      //Circular
      class Element;

      class Pad : public LinkedObjectBase<Pad, GstPad> {
            friend class LinkedObjectBase<Pad, GstPad>;
            public:

                  /** Creates a new pad from the supplied pad */
                  static Pad* Create(GstPad* pad);

                  /** Creates a new pad from the template and the given name */
                  static Pad* Create(GstPadTemplate* tpl, const char* name); //gst_pad_new_from_template

                  /** Creates a new pad from the static template and the given name */
                  static Pad* Create(GstStaticPadTemplate* stpl, const char* name); //gst_pad_new_from_static_template

                  /** Gets and returns a pad v8 instance's internal reference to the pad */
                  static Pad* Unwrap(Handle<Value> value);

                  /** Returns the name of the pad */
                  const char* Name(); //gst_pad_get_name

                  /** Returns the pad direction */
                  const char* Direction(); //gst_pad_get_direction

                  /** Checks whether the pad is linked */
                  bool Linked(); //gst_pad_is_linked

                  /** Returns whether the pad is active */
                  bool Active();

                  /** Returns the pad's element */
                  Element* Parent(); //gst_pad_get_parent_element

                  /** Returns the pads template */
                  PadTemplate* Template(); //gst_pad_get_pad_template

                  /** Checks whether this can link with the supplied pad */
                  bool CanLink(Pad* other); //gst_pad_can_link

                  /** Links with the supplied pad */
                  const char* Link(Pad* other); //gst_pad_link

                  /** Unlinks from the supplied pad */
                  bool Unlink(Pad* other); //gst_pad_unlink

                  /** Gets the allowed capabilities */
                  Caps* AllowedCaps(); //gst_pad_get_allowed_caps

                  /** Gets the template capabilities */
                  Caps* TemplateCaps(); //gst_pad_get_pad_template_caps

                  /** Checks whether the pad has the current capabilities */
                  bool HasCurrentCaps(); //gst_pad_has_current_caps

                  /** Returns the next peer */
                  Pad* Peer();

                  /** Returns the last value returned from the flow */
                  const char* LastFlowReturn(); //gst_pad_get_last_flow_return

                  /** Returns the current position */
                  long Position(GstFormat format); //gst_pad_query_position

                  /** Queries the pad duration */
                  long Duration(GstFormat format); //gst_pad_query_duration

                  /** Checks whether the supplied capabilities are accepted */
                  bool AcceptCaps(Caps* caps); //gst_pad_query_accept_caps

                  /** Queries the current caps */
                  Caps* QueryCaps(Caps* caps); //gst_pad_query_caps

                  /** Checks whether the pad was blocked at the last state request */
                  bool Blocked();

                  /** Checks whether the pad is currently blocking */
                  bool Blocking();
            protected:
                  /**
                  * Allows the caller to customize the contents of the v8 object
                  */
                  void InitializeV8Instance(Handle<Object> instance);
            private:
                  Pad(GstPad* pad);

                  static Pad* Instance(Local<Value> data);

                  static bool StringToFormat(const char* str, GstFormat* format);

                  static GstPadProbeReturn PadProbeCallback(GstPad* gpad, GstPadProbeInfo* info, gpointer userData);

                  //Defined the accessors
                  static void NameGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);
                  static void DirectionGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);
                  static void LinkedGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);
                  static void ActiveGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);
                  static void BlockedGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);
                  static void BlockingGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);

                  //Define the functions
                  static void Parent(const FunctionCallbackInfo<Value>& args);
                  static void Template(const FunctionCallbackInfo<Value>& args);
                  static void CanLink(const FunctionCallbackInfo<Value>& args);
                  static void Link(const FunctionCallbackInfo<Value>& args);
                  static void Unlink(const FunctionCallbackInfo<Value>& args);
                  static void AllowedCaps(const FunctionCallbackInfo<Value>& args);
                  static void CurrentCaps(const FunctionCallbackInfo<Value>& args);
                  static void TemplateCaps(const FunctionCallbackInfo<Value>& args);
                  static void HasCurrentCaps(const FunctionCallbackInfo<Value>& args);
                  static void Peer(const FunctionCallbackInfo<Value>& args);
                  static void LastFlowReturn(const FunctionCallbackInfo<Value>& args);
                  static void Position(const FunctionCallbackInfo<Value>& args);
                  static void Duration(const FunctionCallbackInfo<Value>& args);
                  static void AcceptCaps(const FunctionCallbackInfo<Value>& args);
                  static void QueryCaps(const FunctionCallbackInfo<Value>& args);
      };
}

#endif