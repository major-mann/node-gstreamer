
#ifndef NSTR_LOB
#define NSTR_LOB

#include <node_object_wrap.h>
#include "../helper.h"
#include <map>
#include <vector>

using namespace v8;
namespace nstr {

	template <class T, class CT>
	class LinkedObjectBase : public node::ObjectWrap {
		public:
			typedef void (*ref_handler) (CT* obj);
            typedef void (*unref_handler) (CT* obj);

            /** Initializes the class to be managed as a javasxcript class instead of singleton */
            static void Init(Handle<Object> exports, const char* typeName) {
                Isolate* isolate = Isolate::GetCurrent();
                Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
                tpl->SetClassName(String::NewFromUtf8(isolate, typeName));
                tpl->InstanceTemplate()->SetInternalFieldCount(1);
                ConstructorTemplate.Reset(isolate, tpl);
                Helper::SetProp(exports, typeName, tpl->GetFunction());
            }

            /** Returns the v8 instance that represents this object */
            Handle<Object> GetV8Instance(){
                HandleScope scope(Isolate::GetCurrent());
                Local<Object> v8i = handle();
                if (v8i.IsEmpty()) {
                    PrepareV8Instance();
                    v8i = handle();
                }
                return v8i;
            };

            /** Returns the currently stored object */
            CT* GetObject() {
            	return Obj;
            }

            /** Returns the currently stored object */
            const CT* GetConstObject() {
                if (CObj == NULL) {
                    return Obj;
                } else {
                    return CObj;
                }
            }

            void AddHandler(const char* name, Handle<Function> func) {
                EventHandler* eh = new EventHandler();
                eh->func.Reset(Isolate::GetCurrent(), func);

                typename HandlerMap::iterator it = Handlers.find(name);

                if (it == Handlers.end()) {
                    //Insert new...
                    std::vector<EventHandler*>* handlers = new std::vector<EventHandler*>();
                    handlers->push_back(eh);
                    char* nm = strdup(name);
                    Handlers.insert(std::make_pair(nm, handlers));
                } else {    
                    //Add to vector
                    std::vector<EventHandler*>* handlers = it->second;
                    handlers->push_back(eh);
                }
            }
            bool RemoveHandler(const char* name, Handle<Function> func) {
                bool result = false;
                typename HandlerMap::iterator it = Handlers.find(name);
                if (it != Handlers.end()) {
                    std::vector<EventHandler*>* handlers = it->second;
                    typename std::vector<EventHandler*>::iterator hit = handlers->begin();
                    while (hit != handlers->end()) {
                        EventHandler* eh = *hit;
                        Local<Function> handler = Local<Function>::New(Isolate::GetCurrent(), eh->func);
                        if (handler->Equals(func)) {
                            handlers->erase(hit);
                            eh->func.Reset();
                            delete eh;
                            
                            if (handlers->size() == 0) {
                                const char* oldName = it->first;
                                Handlers.erase(it);
                                delete oldName;
                                delete handlers;
                            }
                            result = true;
                            break;
                        }
                        ++hit;
                    }
                }
                return result;
            }
            void Emit(const char* name, int argc, Handle<Value> argv[]) {
                typename HandlerMap::iterator it = Handlers.begin();
                it = Handlers.find(name);

                if (it != Handlers.end()) {
                    std::vector<EventHandler*>* handlers = it->second;
                    typename std::vector<EventHandler*>::iterator hit = handlers->begin();
                    while (hit != handlers->end()) {
                        EventHandler* handler = *hit;
                        Local<Function> func = Local<Function>::New(Isolate::GetCurrent(), handler->func);
                        Local<Object> v8i = GetV8Instance();
                        func->Call(v8i, argc, argv);
                        ++hit;
                    }
                }
            }

		protected:

            /** static create function which can do a cache lookup for our object from the given type */
            static T* Create(CT* obj) {
                typename std::map<intptr_t, T*>::iterator it = ReverseCachedReferences.find((intptr_t)obj);
                if (it == ReverseCachedReferences.end()) {
                    return new T(obj);
                } else {
                    return it->second;
                }
            }

            /** static create function which can do a cache lookup for our object from the given type */
            static T* Create(const CT* obj) {
                typename std::map<intptr_t, T*>::iterator it = ReverseCachedReferences.find((intptr_t)obj);
                if (it == ReverseCachedReferences.end()) {
                    return new T(obj);
                } else {
                    return it->second;
                }
            }

			LinkedObjectBase(CT* obj, ref_handler ref, unref_handler unref) {
                if (obj == NULL) {
                    printf("%s\n", "Critical application error! Supplied object is NULL");
                    Helper::PrintStack();
                    exit(-1);
                    return;
                }
                RefCall = ref;
                UnrefCall = unref;
                Obj = NULL;
				SetObject(obj);
			}

            LinkedObjectBase(const CT* obj) {
                RefCall = 0;
                UnrefCall = 0;
                Obj = NULL;
                CObj = NULL;
                SetObject(obj);
            }


			virtual ~LinkedObjectBase() {
				typename std::map<intptr_t, CT*>::iterator ctit = CachedReferences.find((intptr_t)this);
				if (ctit != CachedReferences.end()) {
					typename std::map<intptr_t, T*>::iterator rctit = ReverseCachedReferences.find((intptr_t)ctit->second);
					CachedReferences.erase(ctit);
					if (rctit != ReverseCachedReferences.end()) {
						ReverseCachedReferences.erase(rctit);
					}
				}
                DoUnref(Obj);
				Obj = NULL;
			}

			/** Updates the underlying object the class represents */
            void SetObject(CT* obj) {
            	SetObject(obj, NULL, false);
            }

            void SetObject(const CT* obj) {
                SetObject(NULL, obj, false);
            }

            void SetObject(CT* obj, bool clear) {
                SetObject(obj, NULL, false);
            }

            void SetObject(const CT* obj, bool clear) {
                SetObject(NULL, obj, false);
            }

            virtual void OnObjectUpdated(CT* ov, CT* nv) {
                //This is here so derived classes can manage the change
            }

            virtual void OnObjectUpdated(const CT* ov, const CT* nv) {
                //This is here so derived classes can manage the change
            }

            /** Prepares the v8 instance */
            void PrepareV8Instance() {
                Local<Object> v8i = handle();
                if (v8i.IsEmpty()) {
                    if (ConstructorTemplate.IsEmpty()) {
                        Handle<Object> instance = CreateV8Instance();
                        SetV8Instance(instance);
                    } else {
                        Handle<Object> instance = New();
                        SetV8Instance(instance);
                    }
                }
            }

            /**
            * Creates and updates the V8Instance on this class according to whichever object type
            * is being dealt with
            */
            virtual Handle<Object> CreateV8Instance() {
                Isolate* isolate = Isolate::GetCurrent();
                Local<ObjectTemplate> tpl = ObjectTemplate::New(isolate);
                tpl->SetInternalFieldCount(1);
                Local<Object> instance = tpl->NewInstance();
                return instance;
            }

            /**
            * Allows the caller to customize what is happening
            */
            virtual void InitializeV8Instance(Handle<Object> instance) {
                //Note: In the first call from the constructor, the instance will be empty
                if (!instance.IsEmpty()) {
                    //Add the event emitter members
                    Handle<External> data = External::New(Isolate::GetCurrent(), this);
                    Helper::SetFunctionProp(instance, "on", On, data);
                    Helper::SetFunctionProp(instance, "off", Off, data);
                    Helper::SetFunctionProp(instance, "emit", Emit, data);
                }
            };
		private:
            /** Holds the event data */
            struct EventHandler {
                Persistent<Function> func;
            };

            /** The event handlers map type */
            typedef std::map<const char*, std::vector<EventHandler*>*, cmp_str> HandlerMap;

            /** The map of event handlers */
            HandlerMap Handlers;

			//Reference from class to stored object
			static std::map<intptr_t, CT*> CachedReferences;

            //Reference from class to const stored object
            static std::map<intptr_t, const CT*> ConstCachedReferences;

			//Stored object to class
			static std::map<intptr_t, T*> ReverseCachedReferences;

			//The constructor in the case this template is used as a Class type
			//	instead of object generator
			static Persistent<FunctionTemplate> ConstructorTemplate;

			//Memory management
			ref_handler RefCall;
			unref_handler UnrefCall;

			//The linked object
			CT* Obj;

            //The const linked object
            const CT* CObj;

            //TODO: Should this be persistent? My assumption would be no as we want v8 to tell us when the object
            //  is no longer needed. But it seems that something is going wrong down the line
			//The linked v8 object
			//Handle<Object> V8Instance;

			/** Updates the v8 instance */
            void SetV8Instance(Handle<Object> instance) {
                //V8Instance = instance;
                Wrap(instance);
                InitializeV8Instance(instance);
            }

            void DoRef(CT* obj) {
                if (RefCall != 0 && obj != NULL) {
                    RefCall(obj);
                }
            }

            void DoUnref(CT* obj) {
                if (UnrefCall != 0 && obj != NULL) {
                    UnrefCall(obj);
                }
            }

            void CacheInsert(CT* obj) {
                CachedReferences.insert(std::make_pair((intptr_t)this, obj));
                ReverseCacheInsert(obj);
            }

            void CacheInsert(const CT* obj) {
                ConstCachedReferences.insert(std::make_pair((intptr_t)this, obj));
                ReverseCacheInsert(obj);
            }

            void ReverseCacheInsert(const CT* obj) {
                ReverseCachedReferences.insert(std::make_pair((intptr_t)obj, (T*)this));
            }

            void RemoveFromCache(const CT* obj) {
                if (obj == NULL) {
                    return;
                }
                typename std::map<intptr_t, T*>::iterator tit = ReverseCachedReferences.find((intptr_t)obj);
                if (tit != ReverseCachedReferences.end()) {
                    ReverseCachedReferences.erase(tit);
                }
                typename std::map<intptr_t, CT*>::iterator ctit = CachedReferences.find((intptr_t)this);
                if (ctit != CachedReferences.end()) {
                    CachedReferences.erase(ctit);
                }
                typename std::map<intptr_t, const CT*>::iterator cctit = ConstCachedReferences.find((intptr_t)this);
                if (cctit != ConstCachedReferences.end()) {
                    ConstCachedReferences.erase(cctit);
                }
            }

            /** Sets the object */
            void SetObject(CT* obj, const CT* cobj, bool clear) {

                //Remove existing reference
                RemoveFromCache(Obj);
                RemoveFromCache(CObj);


                //Set new reference
                DoUnref(Obj);
                CT* oldObj = Obj;
                const CT* oldCObj = CObj;
                Obj = obj;
                CObj = cobj;
                if (obj == NULL) {
                    OnObjectUpdated(oldCObj, CObj);
                } else {
                    OnObjectUpdated(oldObj, Obj);
                }
                DoRef(Obj);

                if (obj != NULL) {
                    CacheInsert(obj);
                }
                if (cobj != NULL) {
                    CacheInsert(cobj);
                }

                if (clear) {
                    //TODO: Remove existing object properties from v8 instance
                }


                //Re-initialise
                InitializeV8Instance(handle());

            }

            static Handle<Object> New() {
                //TODO: This should call new instance without creating a new element...?
                //  Maybe pass some kind of baton across...

                Local<FunctionTemplate> tpl = Local<FunctionTemplate>::New(Isolate::GetCurrent(), ConstructorTemplate);
                Local<Function> cons = tpl->GetFunction();

                //We pass an external in to indicate that we want a new javascript instance,
                //  but not a new T* instance
                char* tmp = new char[0];
                Local<External> ext = External::New(Isolate::GetCurrent(), tmp);
                Handle<Value> argv[1];
                argv[0] = ext;
                Handle<Object> v8ins = cons->NewInstance(1, argv);
                return v8ins;
            }

            static void New(const FunctionCallbackInfo<Value>& args) {
                Isolate* isolate = Isolate::GetCurrent();
                HandleScope scope(isolate);

                if (args.IsConstructCall()) {

                    if (args.Length() > 0 && args[0]->IsExternal()) {
                        //TODO: Some kind of check on the external?
                        delete (char*)Local<External>::Cast(args[0])->Value();
                        args.GetReturnValue().Set(args.This());
                    } else {
                        //We expect that types that inherit from this provide a
                        //  function that can be used to creat from args
                        LinkedObjectBase<T, CT>* instance = T::Create(args);

                        //Note: We expect the create function to throw a relevant
                        //  v8 error if it is unable to create the type
                        if (instance != NULL) {
                            instance->SetV8Instance(args.This());
                            args.GetReturnValue().Set(args.This());
                        }
                    }
                } else {

                    // Invoked as plain function `MyObject(...)`, turn into construct call.
                    const int argc = 1;
                    //TODO: This should pass all the args
                    Local<Value> argv[argc] = { args[0] };
                    Local<FunctionTemplate> tpl = Local<FunctionTemplate>::New(isolate, ConstructorTemplate);
                    Local<Function> cons = tpl->GetFunction();        
                    args.GetReturnValue().Set(cons->NewInstance(argc, argv));
                }
            }

            static LinkedObjectBase<T, CT>* Instance(Handle<Value> value) {
                Handle<External> ext = Handle<External>::Cast(value);
                return (LinkedObjectBase<T, CT>*)ext->Value();
            }

            static void On(const FunctionCallbackInfo<Value>& args) {
                //on(name, handler)
                if (args.Length() > 1 && args[0]->IsString() && args[1]->IsFunction()) {
                    LinkedObjectBase<T, CT>* self = Instance(args.Data());
                    String::Utf8Value name(args[0]);
                    Handle<Function> func = Handle<Function>::Cast(args[1]);
                    self->AddHandler(*name, func);
                } else {
                    Helper::ThrowV8Error("on requires name and handler parameters");
                }
            }

            static void Off(const FunctionCallbackInfo<Value>& args) {
                //off(name, handler)
                if (args.Length() > 1 && args[0]->IsString() && args[1]->IsFunction()) {
                    LinkedObjectBase<T, CT>* self = Instance(args.Data());
                    String::Utf8Value name(args[0]);
                    Handle<Function> func = Handle<Function>::Cast(args[1]);
                    self->RemoveHandler(*name, func);
                } else {
                    Helper::ThrowV8Error("off requires name and handler parameters");
                }
            }

            static void Emit(const FunctionCallbackInfo<Value>& args) {
                //emit(name, arg1, arg2, argN)
                if (args.Length() > 0 && args[0]->IsString()) {
                    LinkedObjectBase<T, CT>* self = Instance(args.Data());
                    String::Utf8Value name(args[0]);

                    int len = args.Length() - 1;
                    Handle<Value>* pargv = new Handle<Value>[len];
                    Handle<Value> argv = *pargv;
                    self->Emit(*name, len, &argv);
                    delete [] pargv;
                } else {
                    Helper::ThrowV8Error("emit requires name parameter");
                }
            }
	};

    template <class T, class CT>
    std::map<intptr_t, CT*> LinkedObjectBase<T, CT>::CachedReferences;

    template <class T, class CT>
    std::map<intptr_t, const CT*> LinkedObjectBase<T, CT>::ConstCachedReferences;

    template <class T, class CT>
    std::map<intptr_t, T*> LinkedObjectBase<T, CT>::ReverseCachedReferences;

    template <class T, class CT>
    Persistent<FunctionTemplate> LinkedObjectBase<T, CT>::ConstructorTemplate;

}

#endif