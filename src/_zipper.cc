// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>

// zipper
#include "zipper.hpp"

using namespace node;
using namespace v8;

extern "C" {

  static void init (Local<Object> target)
  {

    Zipper::Initialize(target);

    // zipper version
    // Nan::Set(target, Nan::New<v8::String>("version").ToLocalChecked(),Nan::New<v8::String>("0.0.1"));
    
    // versions of deps
    Local<Object> versions = Nan::New<Object>();
    //Nan::Set(versions, Nan::New<String>("node").ToLocalChecked(), Nan::New<String>(NODE_VERSION+1)).ToLocalChecked();
    //Nan::Set(versions, Nan::New<String>("v8").ToLocalChecked(), Nan::New<String>(V8::GetVersion())).ToLocalChecked();
    Nan::Set(versions, Nan::New<String>("versions").ToLocalChecked(), versions);

  }

  NODE_MODULE(zipper, init);
}
