#ifndef __ZIPPER_H__
#define __ZIPPER_H__

#include <node.h>
#include <nan.h>
#include <node_object_wrap.h>

// stl
#include <string>
#include <vector>

// libzip
#include <zlib.h>
#include <errno.h>
#include <zip.h>
//#include <zipint.h>

using namespace v8;
using namespace node;

class Zipper: public node::ObjectWrap {
  public:
    static Persistent<Function> constructor;
    static void Initialize(Handle<Object> target);
    static NAN_METHOD(New);
    
    // Async
    static NAN_METHOD(addFile);
    static void _AddFile(uv_work_t *req);
    static void _AfterAddFile(uv_work_t *req);
    
    Zipper(std::string const& file_name);

  private:
    ~Zipper();
    std::string const file_name_;
    struct zip *archive_;
};

#endif
