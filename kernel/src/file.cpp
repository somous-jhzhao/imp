/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/file.h>
#include <IMP/internal/directories.h>
#include <cstdlib>
//#include <unistd.h>
#include <boost/scoped_array.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif
IMP_BEGIN_NAMESPACE

namespace {
template <class BaseStream, class FileStream>
struct LazyFileStorage: public internal::IOStorage<BaseStream> {
  typedef internal::IOStorage<BaseStream> P;
  std::string name_;
  bool open_;
  FileStream stream_;
  LazyFileStorage(std::string name): P(name),
                                     open_(false){}
  BaseStream& get_stream() {
    if (!open_) {
      stream_.open(P::get_name().c_str());
      if (!stream_) {
        IMP_THROW("Unable to open file " << P::get_name(),
                  IOException);
      }
      open_=true;
    }
    return stream_;
  }
};

template <class BaseStream, class FileStream>
struct FileStorage: public internal::IOStorage<BaseStream> {
  typedef internal::IOStorage<BaseStream> P;
  FileStream stream_;
  FileStorage(std::string name): P(name), stream_(name.c_str()){
    if (!stream_) {
      IMP_THROW("Unable to open file " << name,
                IOException);
    }
  }
  BaseStream& get_stream() {
    return stream_;
  }
};

template <class BaseStream>
struct StreamStorage: public internal::IOStorage<BaseStream> {
  typedef internal::IOStorage<BaseStream> P;
  BaseStream &stream_;
  StreamStorage(BaseStream &stream, std::string name): P(name),
                                                       stream_(stream){}
  BaseStream& get_stream() {
    return stream_;
  }
};

template <class BaseStream>
struct OwnedStreamStorage: public internal::IOStorage<BaseStream>{
  typedef internal::IOStorage<BaseStream> P;
  BaseStream &stream_;
  internal::OwnerPointer<Object> ref_;
  OwnedStreamStorage(BaseStream &stream,
                     Object*o): P("python stream"),
                                stream_(stream),
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
TextOutput::TextOutput(std::ostream &in, std::string name):
  out_(new StreamStorage<std::ostream>(in, name)){}
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
  in_(new LazyFileStorage<std::istream, std::ifstream>(c)){}
TextInput::TextInput(std::string c):
  in_(new LazyFileStorage<std::istream, std::ifstream>(c)){}
TextInput::TextInput(std::istream &in, std::string name):
  in_(new StreamStorage<std::istream>(in, name)){}
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
  void testf(TextInput) {
  }
  void testcall() {
    testf(std::string("filename"));

    std::ifstream inf("infile");
    testf(inf);
  }
}


TextOutput create_temporary_file(std::string prefix,
                                 std::string suffix) {
  char *env = getenv("IMP_BUILD_ROOT");
  std::string imp_tmp;
  if (env) {
    imp_tmp= internal::get_concatenated_path(env, "build/tmp");
  }
#if defined _MSC_VER
  std::string tpathstr;
  if (imp_tmp.empty()) {
    TCHAR tpath[MAX_PATH];
    DWORD dwRetVal = GetTempPath(MAX_PATH,tpath);
    if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
      IMP_THROW("Unable to find temporary path", IOException);
    }
    tpathstr=tpath;
  } else {
    tpathstr= imp_tmp;
  }
  char filename[MAX_PATH];
  if (GetTempFileName(tpathstr.c_str(), prefix.c_str(), 0, filename)==0) {
     IMP_THROW("Unable to create temp file in " << tpathstr, IOException);
  }
  return TextOutput(std::string(filename)+suffix);
#else
  std::string pathprefix;
  if (imp_tmp.empty()) {
    pathprefix="/tmp";
  } else {
    pathprefix=imp_tmp;
  }
  std::string templ=internal::get_concatenated_path(pathprefix,
                                                    prefix+".XXXXXX");
  boost::scoped_array<char> filename;
  filename.reset(new char[templ.size()+suffix.size()+1]);
  std::copy(templ.begin(), templ.end(), filename.get());
#ifdef __APPLE__
  std::copy(suffix.begin(), suffix.end(), filename.get()+templ.size());
  filename[templ.size()+ suffix.size()]='\0';
  int fd = mkstemps(filename.get(), suffix.size());
  if (fd == -1) {
    IMP_THROW("Unable to create temporary file: " << filename.get(),
              IOException);
  }
  close(fd);
#else
  filename[templ.size()]='\0';
  int fd = mkstemp(filename.get());
  if (fd == -1) {
    IMP_THROW("Unable to create temporary file: " << filename.get(),
              IOException);
  }
  close(fd);
  std::copy(suffix.begin(), suffix.end(), filename.get()+templ.size());
  filename[templ.size()+ suffix.size()]='\0';
#endif
  return TextOutput(filename.get());
#endif
}

namespace {

}

std::string get_relative_path(std::string base,
                                   std::string relative) {
  std::string dir= internal::get_directory_path(base);
  return internal::get_concatenated_path(dir, relative);
}

IMP_END_NAMESPACE
