#include "zipper.hpp"

// stl
#include <sstream>
#include <vector>
#include <cstring>
#include <algorithm>
//#include <iostream>

#include <node_buffer.h>
#include <node_version.h>


#define TOSTR(obj) (*String::Utf8Value((obj)->ToString()))

Persistent<Function> Zipper::constructor;

void Zipper::Initialize(Handle<Object> target) {

    NanScope();
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(Zipper::New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(NanNew<String>("Zipper"));

    // functions
    NODE_SET_PROTOTYPE_METHOD(tpl, "addFile", addFile);

    NanAssignPersistent(constructor, tpl->GetFunction());
    target->Set(NanNew<String>("Zipper"),tpl->GetFunction());
}

Zipper::Zipper(std::string const& file_name) :
  ObjectWrap(),
  file_name_(file_name),
  archive_() {}

Zipper::~Zipper() {
    zip_close(archive_);
}

NAN_METHOD(Zipper::New)
{
    NanScope();

    if (!args.IsConstructCall())
        return NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");

    if (args.Length() != 1 || !args[0]->IsString())
        return NanThrowTypeError("first argument must be a path to a zipfile");

    std::string input_file = TOSTR(args[0]);
    struct zip *za;
    int err;
    char errstr[1024];
    if ((za=zip_open(input_file.c_str(), ZIP_CREATE, &err)) == NULL) {
        zip_error_to_str(errstr, sizeof(errstr), err, errno);
        std::stringstream s;
        s << "cannot open file: " << input_file << " error: " << errstr << "\n";
        return NanThrowError(s.str().c_str());
    }

    Zipper* zf = new Zipper(input_file);
    zf->archive_ = za;
    zf->Wrap(args.This());
    NanReturnValue(args.This());

}


typedef struct {
    Zipper* zf;
    struct zip *za;
    std::string name;
    std::string path;
    bool error;
    std::string error_name;
    std::vector<unsigned char> data;
    Persistent<Function> cb;
} closure_t;


NAN_METHOD(Zipper::addFile)
{
    NanScope();

    if (args.Length() < 3)
        return NanThrowTypeError("requires three arguments, the path of a file, a filename and a callback");
    
    // first arg must be path
    if(!args[0]->IsString())
        return NanThrowTypeError("first argument must be a file path to add to the zip");
    
    // second arg must be name
    if(!args[1]->IsString())
        return NanThrowTypeError("second argument must be a file name to add to the zip");
    
    // last arg must be function callback
    if (!args[args.Length()-1]->IsFunction())
        return NanThrowTypeError("last argument must be a callback function");
  
    std::string path = TOSTR(args[0]);
    std::string name = TOSTR(args[1]);
  
    Zipper* zf = ObjectWrap::Unwrap<Zipper>(args.This());
    zf->Ref();

    uv_work_t *req = new uv_work_t();
    closure_t *closure = new closure_t();

    // libzip is not threadsafe so we cannot use the zf->archive_
    // instead we open a new zip archive for each thread
    struct zip *za;
    int err;
    char errstr[1024];
    if ((za=zip_open(zf->file_name_.c_str() , ZIP_CREATE, &err)) == NULL) {
        zip_error_to_str(errstr, sizeof(errstr), err, errno);
        std::stringstream s;
        s << "cannot open file: " << zf->file_name_ << " error: " << errstr << "\n";
        zip_close(za);
        return NanThrowError(s.str().c_str());
    }

    closure->zf = zf;
    closure->za = za;
    closure->error = false;
    closure->path = path;
    closure->name = name;
    NanAssignPersistent(closure->cb, args[args.Length()-1].As<Function>());
    req->data = closure;

    uv_queue_work(uv_default_loop(), req, _AddFile, (uv_after_work_cb)_AfterAddFile);
    NanReturnUndefined();
}


void Zipper::_AddFile(uv_work_t *req)
{
    closure_t *closure = static_cast<closure_t *>(req->data);

    struct zip_source *source = zip_source_file(closure->za, closure->path.c_str(), 0, 0);
    if (zip_add(closure->za, closure->name.c_str(), source) < 0) {
        std::stringstream s;
        s << "Cannot prepare file for add to zip: '" << closure->path << "'\n";
        closure->error = true;
        closure->error_name = s.str();
        zip_source_free(source);
    }

    if (zip_close(closure->za) < 0) {
        std::stringstream s;
        s << "Cannot add file to zip: '" << closure->path << "' (" << zip_strerror(closure->za) << ")\n";
        closure->error = true;
        closure->error_name = s.str();
    }
}


void Zipper::_AfterAddFile(uv_work_t *req)
{
    NanScope();

    closure_t *closure = static_cast<closure_t *>(req->data);

    TryCatch try_catch;
  
    if (closure->error) {
        Local<Value> argv[1] = { Exception::Error(NanNew<String>(closure->error_name.c_str())) };
        NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(closure->cb), 1, argv);
    } else {
        Local<Value> argv[1] = { NanNull() };
        NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(closure->cb), 1, argv);
    }
    
    closure->zf->Unref();
    NanDisposePersistent(closure->cb);
    delete closure;
    delete req;

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
      //try_catch.ReThrow();
    }
}
