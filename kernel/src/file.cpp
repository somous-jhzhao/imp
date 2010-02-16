/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/file.h>

IMP_BEGIN_NAMESPACE

namespace {
template <class BaseStream, class FileStream>
struct LazyFileStorage: public internal::IOStorage<BaseStream> {
  std::string name_;
  bool open_;
  FileStream stream_;
  LazyFileStorage(std::string name): name_(name), open_(false){}
  BaseStream& get_stream() {
    if (!open_) {
      stream_.open(name_.c_str());
      if (!stream_) {
        IMP_THROW("Unabe to open file " << name_,
                  IOException);
      }
      open_=true;
    }
    return stream_;
  }
};

template <class BaseStream, class FileStream>
struct FileStorage: public internal::IOStorage<BaseStream> {
  FileStream stream_;
  FileStorage(std::string name): stream_(name.c_str()){
    if (!stream_) {
      IMP_THROW("Unabe to open file " << name,
                IOException);
    }
  }
  BaseStream& get_stream() {
    return stream_;
  }
};

template <class BaseStream>
struct StreamStorage: public internal::IOStorage<BaseStream> {
  BaseStream &stream_;
  StreamStorage(BaseStream &stream): stream_(stream){}
  BaseStream& get_stream() {
    return stream_;
  }
};

template <class BaseStream>
struct OwnedStreamStorage: public internal::IOStorage<BaseStream>{
  BaseStream &stream_;
  internal::OwnerPointer<Object> ref_;
  OwnedStreamStorage(BaseStream &stream,
                     Object*o): stream_(stream),
                                ref_(o){}
  BaseStream& get_stream() {
    return stream_;
  }
};

}

TextOutput::TextOutput(const char *c):
  out_(new LazyFileStorage<std::ostream, std::ofstream>(c)){}
TextOutput::TextOutput(std::string c):
  out_(new LazyFileStorage<std::ostream, std::ofstream>(c)){}
TextOutput::TextOutput(std::ostream &in):
  out_(new StreamStorage<std::ostream>(in)){}
TextOutput::TextOutput(TextProxy<std::ostream> out):
  out_(new OwnedStreamStorage<std::ostream>(*out.str_,
                                                      out.ptr_)){}
TextOutput::TextOutput(int) {
  IMP_THROW("Wrong argument type", IOException);
}
TextOutput::TextOutput(double) {
  IMP_THROW("Wrong argument type", IOException);
}

TextInput::TextInput(const char *c):
  in_(new FileStorage<std::istream, std::ifstream>(c)){}
TextInput::TextInput(std::string c):
  in_(new FileStorage<std::istream, std::ifstream>(c)){}
TextInput::TextInput(std::istream &in):
  in_(new StreamStorage<std::istream>(in)){}
TextInput::TextInput(TextProxy<std::istream> out):
  in_(new OwnedStreamStorage<std::istream>(*out.str_,
                                           out.ptr_)){}
TextInput::TextInput(int) {
 IMP_THROW("Wrong argument type", IOException);
}
TextInput::TextInput(double) {
  IMP_THROW("Wrong argument type", IOException);
}




namespace {
  void testf(TextInput in) {
  }
  void testcall() {
    testf(std::string("filename"));

    std::ifstream inf("infile");
    testf(inf);
  }
}

IMP_END_NAMESPACE
