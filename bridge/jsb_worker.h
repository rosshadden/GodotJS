#ifndef GODOTJS_WORKER_H
#define GODOTJS_WORKER_H
#include "jsb_bridge_pch.h"
#include "jsb_buffer.h"

#if !JSB_WITH_WEB && !JSB_WITH_JAVASCRIPTCORE
namespace jsb
{
    enum class FinalizationType : uint8_t;

    typedef internal::Index32 WorkerID;
    // typedef BinaryMutex WorkerLock;
    typedef Mutex WorkerLock;
    class Environment;
    class WorkerImpl;
    typedef std::shared_ptr<WorkerImpl> WorkerImplPtr;

    class Worker
    {
    private:
        friend class WorkerImpl;
        friend class JSWorkerModuleLoader;

        WorkerID id_ = {};

        static WorkerLock lock_;
        static internal::SArray<WorkerImplPtr, WorkerID> worker_list_;
        static HashMap<Thread::ID, WorkerID> workers_;

    public:
        static void register_(const v8::Local<v8::Context>& p_context, const v8::Local<v8::Object>& p_self);

        // release all workers, call from main thread (GodotJSScriptLanguage::finish)
        static void finish();

        static void on_thread_enter();
        static void on_thread_exit();

    private:
        static void finalizer(Environment*, void* pointer, FinalizationType /* p_finalize */);
        static void constructor(const v8::FunctionCallbackInfo<v8::Value>& info);

        static void terminate(const v8::FunctionCallbackInfo<v8::Value>& info);
        static void post_message(const v8::FunctionCallbackInfo<v8::Value>& info);
        static void _placeholder(const v8::FunctionCallbackInfo<v8::Value>& info);

        static WorkerID create(Environment* p_master, const String& p_path, NativeObjectID p_handle);

        // check if a worker valid
        static bool is_valid(WorkerID p_id);

        static bool try_get_worker(WorkerID p_id, NativeObjectID& o_handle, void*& o_token_ptr);

        // terminate a worker
        static bool terminate(WorkerID p_id);

        // master -> worker
        static void on_receive(WorkerID p_id, Buffer&& p_buffer);
    };
}
#endif

#endif
