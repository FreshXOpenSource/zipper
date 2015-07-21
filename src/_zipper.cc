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

  static void init (Handle<Object> target)
  {

    Zipper::Initialize(target);

    // zipper version
    target->Set(NanNew<String>("version"), NanNew<String>("0.0.1"));

    // versions of deps
    Local<Object> versions = NanNew<Object>();
    versions->Set(NanNew<String>("node"), NanNew<String>(NODE_VERSION+1));
    versions->Set(NanNew<String>("v8"), NanNew<String>(V8::GetVersion()));
    target->Set(NanNew<String>("versions"), versions);

  }

  NODE_MODULE(zipper, init);
}
