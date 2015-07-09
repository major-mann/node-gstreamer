//Note: Large parts of this are from https://github.com/mapbox/node-sqlite3

#ifndef NSTR_ASYNC
#define NSTR_ASYNC

#include <uv.h>
#include "../threading.h"

namespace nstr {


    // Generic uv_async handler. From https://github.com/mapbox/node-sqlite3/blob/master/src/async.h
    template <class Item, class Parent>
    class Async {
        typedef void (*Callback)(Parent* parent, Item* item);

        protected:
            uv_async_t watcher;
            NSTR_MUTEX_t
            std::vector<Item*> data;
            Callback callback;
        public:
            Parent* parent;

        public:
            Async(Parent* parent_, Callback cb_)
                : callback(cb_), parent(parent_) {
                watcher.data = this;
                NSTR_MUTEX_INIT
                uv_async_init(uv_default_loop(), &watcher, reinterpret_cast<uv_async_cb>(listener));
            }

            static void listener(uv_async_t* handle, int status) {
                Async* async = static_cast<Async*>(handle->data);
                std::vector<Item*> rows;
                NSTR_MUTEX_LOCK(&async->mutex)
                rows.swap(async->data);
                NSTR_MUTEX_UNLOCK(&async->mutex)
                for (unsigned int i = 0, size = rows.size(); i < size; i++) {
#if NODE_VERSION_AT_LEAST(0, 7, 9)
                    uv_unref((uv_handle_t *)&async->watcher);
#else
                    uv_unref(uv_default_loop());
#endif
                    async->callback(async->parent, rows[i]);
                }
            }

            static void close(uv_handle_t* handle) {
                assert(handle != NULL);
                assert(handle->data != NULL);
                Async* async = static_cast<Async*>(handle->data);
                delete async;
            }

            void finish() {
                // Need to call the listener again to ensure all items have been
                // processed. Is this a bug in uv_async? Feels like uv_close
                // should handle that.
                listener(&watcher, 0);
                uv_close((uv_handle_t*)&watcher, close);
            }

            void add(Item* item) {
                // Make sure node runs long enough to deliver the messages.
        #if NODE_VERSION_AT_LEAST(0, 7, 9)
                uv_ref((uv_handle_t *)&watcher);
        #else
                uv_ref(uv_default_loop());
        #endif
                NSTR_MUTEX_LOCK(&mutex);
                data.push_back(item);
                NSTR_MUTEX_UNLOCK(&mutex)
            }

            void send() {
                uv_async_send(&watcher);
            }

            void send(Item* item) {
                add(item);
                send();
            }

            ~Async() {
                NSTR_MUTEX_DESTROY
            }
    };
    
    template <class TData, class TResult>
    class AsyncRunner {
        public:
            typedef TResult* (*WorkerCallback)(TData* data);
            typedef void (*CompleteCallback)(TData* data, TResult* result);

            static void Run(WorkerCallback worker, CompleteCallback complete, TData* data) {
                uv_work_t* baton = new uv_work_t();
                baton->data = new AsyncRunner(worker, complete, data);
                uv_queue_work(uv_default_loop(), baton, Callback, CompleteCb);
            }
        private:
            WorkerCallback Worker;
            CompleteCallback Complete;
            TData* Data;
            TResult* Result;

            AsyncRunner(WorkerCallback worker, CompleteCallback complete, TData* data) {
                Worker = worker;
                Complete = complete;
                Data = data;
            }

            static void Callback(uv_work_t* baton) {
                AsyncRunner* async = (AsyncRunner*)baton->data;
                async->Result = async->Worker(async->Data);
            }

            static void CompleteCb(uv_work_t* baton, int status) {
                AsyncRunner* async = (AsyncRunner*)baton->data;
                async->Complete(async->Data, async->Result);
                delete async;
                delete baton;
            }
    };
};

#endif