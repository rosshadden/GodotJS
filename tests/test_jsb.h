#ifndef GODOTJS_TESTS_JSB_H
#define GODOTJS_TESTS_JSB_H

#include "jsb_test_helpers.h"

namespace jsb::tests
{
#if JSB_WITH_QUICKJS
    struct Bindings
    {
        static void constructor(const v8::FunctionCallbackInfo<v8::Value>& info)
        {

        }

        static JSValue magic_call(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic)
        {
            CHECK(magic == 1);
            return JS_UNDEFINED;
        }

        static void print_exception(const impl::TryCatch& try_catch)
        {
            if (try_catch.has_caught())
            {
                String message;
                try_catch.get_message(&message);
                MESSAGE("JS Exception: ", message);
            }
        }
    };

    TEST_CASE("[jsb] quickjs.minimal")
    {
        JSRuntime* rt = JS_NewRuntime();
        JSContext* ctx = JS_NewContext(rt);
        {
            const JSValue this_obj = JS_NewObject(ctx);
            const JSValue func = JS_NewCFunctionMagic(ctx, Bindings::magic_call, "magic_call", 0, JS_CFUNC_generic_magic, 1);
            const JSAtom prop = JS_NewAtom(ctx, "prop");

            CHECK(JS_IsFunction(ctx, func));
            CHECK(prop != JS_ATOM_NULL);
            CHECK(impl::QuickJS::_RefCount(func) == 1);
            constexpr int flags = JS_PROP_HAS_ENUMERABLE | JS_PROP_HAS_CONFIGURABLE | JS_PROP_HAS_GET;
            CHECK(JS_DefineProperty(ctx, this_obj, prop, JS_UNDEFINED, func, JS_UNDEFINED, flags) == 1);
            CHECK(impl::QuickJS::_RefCount(func) == 2);

            JS_FreeValue(ctx, func);
            JS_FreeAtom(ctx, prop);
            JS_FreeValue(ctx, this_obj);
        }
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
    }

    TEST_CASE("[jsb] quickjs.isolate")
    {
        v8::Isolate::CreateParams params;
        v8::Isolate* isolate = v8::Isolate::New(params);
        {
            v8::HandleScope scope_0(isolate);
            v8::Local<v8::Context> context = v8::Context::New(isolate);
            v8::Local<v8::Symbol> symbol = v8::Symbol::New(isolate);

            {
                v8::HandleScope scope_1(isolate);
                impl::ClassBuilder builder = impl::ClassBuilder::New<IF_ObjectFieldCount>(isolate, "Base", Bindings::constructor, 0);
                const impl::Class class1 = builder.Build(context);
                context->Global()->Set(context, impl::Helper::new_string(isolate, "Base"), class1.Get(isolate));
            }

            {
                static constexpr char source[] = R"--((function() {
    let a = 1+1;
    return a;
}))--";
                impl::TryCatch try_catch(isolate);
                v8::MaybeLocal<v8::Value> eval = impl::Helper::compile_run(context, source, ::std::size(source) - 1, "testcase.js");
                Bindings::print_exception(try_catch);
                CHECK(!eval.IsEmpty());
                CHECK(eval.ToLocalChecked()->IsFunction());
                v8::Local<v8::Function> func = eval.ToLocalChecked().As<v8::Function>();
                v8::MaybeLocal<v8::Value> rval = func->Call(context, v8::Undefined(isolate), 0, nullptr);
                CHECK(!rval.IsEmpty());
                CHECK(rval.ToLocalChecked()->IsNumber());
                const int32_t rval_v = rval.ToLocalChecked().As<v8::Int32>()->Value();
                CHECK(rval_v == 2);
            }
        }
        isolate->Dispose();
    }
#endif

    TEST_CASE("[jsb] Node new/free")
    {
        GodotJSScriptLanguageIniter initer;

        Error err;
        GodotJSScriptLanguage::get_singleton()->eval_source(R"--(
let gd = require("godot");
let node = new gd.Node();
console.assert(gd.is_instance_valid(node));
node.free();
console.assert(!gd.is_instance_valid(node));
)--", err);
    }

    //TODO enable this test case after bug fixed
#if JSB_WITH_V8
    TEST_CASE("[jsb] Scripts: test_01")
    {
        GodotJSScriptLanguageIniter initer;

        Error err;
        GodotJSScriptLanguage::get_singleton()->eval_source(R"--(
let gd = require("godot");
let mod = require("test_01");
console.assert(typeof mod === "object");
console.assert(mod.call_me() == 123);
console.log(typeof mod.default === "function");
console.log(mod.default);
let inst = new mod.default();
console.assert(gd.is_instance_valid(inst));
console.assert(inst instanceof gd.Node);
inst.free();
console.assert(!gd.is_instance_valid(inst));

)--", err);
    }
#endif
}

#endif

