
import scons_tools.module
import scons_tools.data
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re

def _add_use_or_no(env, h, name):
    nd= name.replace("_USE_","_NO_")
    if nd==name:
        nd= name.replace("_NO_", "_USE_")
    print >> h, "\n",
    print >> h, "#  if defined("+nd+") || defined("+name+")"
    print >> h, '#    error "Do not define macro '+name+' directly."'
    print >> h, "#  endif"
    print >> h, "#  define "+name

def _add_use(env, h, nm):
    curn= scons_tools.module._get_module_name(env)
    _add_use_or_no(env, h, "IMP_"+curn.upper()+"_USE_"+nm.upper())
def _add_no_use(env, h, nm):
    curn= scons_tools.module._get_module_name(env)
    _add_use_or_no(env, h, "IMP_"+curn.upper()+"_NO_"+nm.upper())
def _add_version(env, h, nm, version, versionheader, versioncpp):
    print >> h, "#ifndef SWIG"
    print >> h, "\n#include <"+versionheader+">\n"
    if type(version)==type([]):
        if not type(versioncpp) == type([]):
            scons_tools.utility.report_error(env, "Version list expected")
        test = " && ".join([x[0]+" != " + x[1] for x in zip(versioncpp, version)])
    else:
        test=versioncpp +" != "+version
    print >> h, "#  if "+test
    print >> h, '#    error "'+nm+ 'versions does not match expected version. Please rerun"\\'\
    '"configuration tests by running scons with --config=force."'
    print >> h, "#  endif"
    print >> h, "#endif"


def _action_config_h(target, source, env):
    """The IMPModuleConfig Builder generates a configuration header file
       used to mark classes and functions for export and to define namespaces,
       and a corresponding SWIG interface, e.g.
       env.IMPModuleConfig(('config.h', 'foo_config.i'), env.Value('foo'))
       generates a configuration header and interface for the 'foo' module."""
    vars= scons_tools.module._get_module_variables(env)
    #print "opening h at " +target[0].abspath + " for module %(module)s"%vars
    h = file(target[0].abspath, 'w')
    #print "Generating "+str(h)
    print >> h, """/*
 * \\file %(module_include_path)s/%(module)s_config.h
 * \\brief Provide macros to mark functions and classes as exported
 *        from a DLL/.so, and to set up namespaces
 *
 * When building the module, %(PREPROC)s_EXPORTS should be defined, and when
 * using the module externally, it should not be. Classes and functions
 * declared in the module's headers should then be marked with
 * %(PREPROC)sEXPORT if they are intended to be part of the API and
 * they are not defined entirely in a header.
 *
 * The Windows build environment requires applications to mark exports in
 * this way; we use the same markings to set the visibility of ELF symbols
 * if we have compiler support.
 *
 * All code in this module should live in the %(namespace)s namespace.
 * This is simply achieved by wrapping things with the
 * %(PREPROC)s_BEGIN_NAMESPACE and %(PREPROC)s_END_NAMESPACE macros.
 * There are similar macros for module code that is designed to be for
 * internal use only.
 *
 * This header is auto-generated by scons_tools/module/_config_h.py;
 * it should not be edited manually.
 *
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef %(PREPROC)s_CONFIG_H
#define %(PREPROC)s_CONFIG_H

#include <boost/static_assert.hpp>

#ifndef IMP_DOXYGEN

#  ifdef _MSC_VER

#    ifdef %(PREPROC)s_EXPORTS
#      define %(PREPROC)sEXPORT __declspec(dllexport)
#    else // EXPORTS
#      define %(PREPROC)sEXPORT __declspec(dllimport)
#    endif // EXPORTS

#  else // _MSC_VER

#    ifdef GCC_VISIBILITY
#      define %(PREPROC)sEXPORT __attribute__ ((visibility("default")))
#    else // GCC_VISIBILITY
#      define %(PREPROC)sEXPORT
#    endif // GCC_VISIBILITY
#  endif // _MSC_VER

#  if defined(_MSC_VER) && !defined(SWIG)
#    ifdef %(PREPROC)s_EXPORTS

#      define %(PREPROC)s_EXPORT_TEMPLATE(name)       \
          template class __declspec(dllexport) name

#    else //EXPORTS

#      define %(PREPROC)s_EXPORT_TEMPLATE(name)       \
          template class __declspec(dllimport) name

#    endif // EXPORTS

#  else // MSC and SWIG
#    define %(PREPROC)s_EXPORT_TEMPLATE(name) IMP_REQUIRE_SEMICOLON_NAMESPACE

#  endif // MSC and SWIG


""" % vars
    print >> h, "#  define %(PREPROC)s_BEGIN_NAMESPACE \\"%vars
    for comp in vars['namespace'].split("::"):
        print >> h, "namespace %s {\\" %comp
    print >> h
    print >> h, "#  define %(PREPROC)s_END_NAMESPACE \\"%vars
    for comp in vars['namespace'].split("::"):
        print >> h, "} /* namespace %s */ \\" %comp
    print >> h
    print >> h, """#  define %(PREPROC)s_BEGIN_INTERNAL_NAMESPACE \\
%(PREPROC)s_BEGIN_NAMESPACE \\
namespace internal {
""" %vars
    print >> h
    print >> h, """#  define %(PREPROC)s_END_INTERNAL_NAMESPACE \\
} /* namespace internal */ \\
%(PREPROC)s_END_NAMESPACE
""" %vars
    print >> h

    for d in scons_tools.module._get_module_direct_dependencies(env):
        #print "processing", d
        nm=scons_tools.dependency.get_dependency_string(d)
        _add_use(env, h, nm)
        if scons_tools.data.get(env).dependencies[d].version:
            _add_version(env, h, nm,
                         scons_tools.data.get(env).dependencies[d].version,
                         scons_tools.data.get(env).dependencies[d].versionheader,
                         scons_tools.data.get(env).dependencies[d].versioncpp)
        else:
            #print "no version for", d
            pass
    for d in scons_tools.module._get_module_unfound_dependencies(env):
        nm=scons_tools.dependency.get_dependency_string(d)
        _add_no_use(env, h, nm)
    for d in scons_tools.module._get_module_modules(env):
        nm="IMP_"+d.upper()
        _add_use(env, h, nm)
    for d in scons_tools.module._get_module_unfound_modules(env):
        nm="IMP_"+d.upper()
        _add_no_use(env, h, nm)

    for d in env['IMP_MODULE_CONFIG']:
        if type(d) == type([]):
            name=d[0]
            value=d[1]
        else:
            name=d
            value=None
        nd= name.replace("_USE_","_NO_")
        if nd==name:
            nd= name.replace("_NO_", "_USE_")
        if nd != name:
            print >> h, "#  ifdef "+nd
            print >> h, "/* Do not define IMP config macros directly */"
            print >> h, """#    error "Do not define macro """+nd+""" directly.\""""
            print >> h, "#  endif"
        print >> h, "#  ifdef "+name
        print >> h, "/* Do not define IMP config macros directly */"
        print >> h, """#    error "Do not define macro """+name+""" directly.\""""
        print >> h, "#  endif"
        if value is not None:
            print >> h, "#  define "+name+" "+value
        else:
            print >> h, "#  define "+name
    print >> h


    # This needs to be called get_module_version_info() to make it easy
    # to call from Objects (which have their own get_version_info() method
    print >> h, """

#  ifndef SWIG
#    include <IMP/internal/directories.h>
#    include <IMP/kernel_config.h>
#    include <string>

IMP_BEGIN_INTERNAL_NAMESPACE
IMPEXPORT std::string get_data_path(std::string module_name,
                                    std::string file_name);
IMPEXPORT std::string get_example_path(std::string module_name,
                                       std::string file_name);

IMP_END_INTERNAL_NAMESPACE
#  endif // SWIG
#endif // IMP_DOXYGEN

#include <string>

//  functions are defined explicitly for swig
#ifndef SWIG
namespace IMP {
  class VersionInfo;
}

%(PREPROC)s_BEGIN_NAMESPACE
/** \name Standard module methods
  All \imp modules have a set of standard methods to help get information
  about the module and about file associated with the modules.
  @{
  */
%(PREPROC)sEXPORT const VersionInfo& get_module_version_info();

inline std::string get_module_name() {
   return "%(namespace)s";
}

//! Return the full path to installed data
/** Each module has its own data directory, so be sure to use
    the version of this function in the correct module. To read
    the data file "data_library" that was placed in the \c data
    directory of module "mymodule", do something like
    \code
    std::ifstream in(IMP::mymodule::get_data_path("data_library"));
    \endcode
    This will ensure that the code works when \imp is installed or
    used via the \c tools/imppy.sh script.
*/
inline std::string get_data_path(std::string file_name) {
  return IMP::internal::get_data_path("%(module)s", file_name);
}

//! Return the path to installed example data for this module
/** Each module has its own example directory, so be sure to use
    the version of this function in the correct module.  For example
    to read the file \c example_protein.pdb located in the
    \c examples directory of the IMP::atom module, do
    \code
    IMP::atom::read_pdb(IMP::atom::get_example_path("example_protein.pdb", model));
    \endcode
    This will ensure that the code works when \imp is installed or
    used via the \c tools/imppy.sh script.
*/
inline std::string get_example_path(std::string file_name)  {
  return IMP::internal::get_example_path("%(module)s", file_name);
}
/** @} */


%(PREPROC)s_END_NAMESPACE

#endif // SWIG

#endif  /* %(PREPROC)s_CONFIG_H */""" % vars

def _print_config_h(target, source, env):
    vars= scons_tools.module._get_module_variables(env)
    print "Generating %(module)s_config.h"%vars

ConfigH = Builder(action=Action(_action_config_h,
                                _print_config_h))

def _action_config_cpp(target, source, env):
    vars= scons_tools.module._get_module_variables(env)
    vars['version']= source[0].get_contents()
    cpp = file(target[0].abspath, 'w')

    print >> cpp, """/**
 *  \\file %(module_include_path)s/config.cpp
 *  \\brief %(module)s module version information.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
""" % vars

    print >> cpp, """#include <%(module_include_path)s/%(module)s_config.h>
#include <IMP/VersionInfo.h>
"""  % vars


    print >> cpp, "%(PREPROC)s_BEGIN_NAMESPACE\n" % vars


    print >> cpp, """
const VersionInfo& get_module_version_info() {
    static VersionInfo vi("%(module)s", "%(version)s");
    return vi;
}
""" %vars
    if vars['module']=="kernel":
        print >> cpp, """
namespace internal {
 std::string imp_data_path="%s";
 std::string imp_example_path="%s";
}
"""%(source[1].get_contents(), source[2].get_contents())

    print >> cpp, "\n%(PREPROC)s_END_NAMESPACE" % vars



def _print_config_cpp(target, source, env):
    vars= scons_tools.module._get_module_variables(env)
    print "Generating %(module)s_config.cpp"%vars


ConfigCPP = Builder(action=Action(_action_config_cpp,
                                  _print_config_cpp))
