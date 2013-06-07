#!/bin/sh

# Script to get a git branch and make a .tar.gz on a shared disk, for use by
# autobuild scripts on our build machines.
#
# Should be run on from a crontab on a machine that has access to a git
# clone, e.g.
#
# 10 1 * * * /cowbell1/home/ben/imp/tools/auto-build.sh develop

if [ $# -ne 1 ]; then
  echo "Usage: $0 branch"
  exit 1
fi

GIT_TOP=/cowbell1/git
GIT_DIR=${GIT_TOP}/imp/.git
export GIT_DIR

BRANCH=$1

TMPDIR=/var/tmp/imp-build-$$
IMPTOP=/salilab/diva1/home/imp/$BRANCH
mkdir -p ${IMPTOP}

# Make sure VERSION reflects the head of the specified remote branch
cd ${GIT_TOP}/imp
./setup_git.py  >& /dev/null
git checkout ${BRANCH} -f -q >& /tmp/$$.out
# Squash chatty output from git checkout
grep -v "Version=" /tmp/$$.out
# Make sure we're up to date with the remote
git merge --ff-only -q origin/${BRANCH} || exit 1
git checkout ${BRANCH} -f -q >& /tmp/$$.out
grep -v "Version=" /tmp/$$.out
rm -f /tmp/$$.out

# Get top-most revision
rev=`git rev-parse HEAD`
shortrev=`git rev-parse --short HEAD`

# Get old revision
oldrev_file=${IMPTOP}/.SVN-new/build/imp-gitrev
if [ -f "${oldrev_file}" ]; then
  oldrev=`cat ${oldrev_file}`
fi

# For non-develop builds, skip if the revision hasn't changed
if [ ${BRANCH} != "develop" -a "${oldrev}" = "${rev}" ]; then
  exit 0
fi

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

# Get IMP code from git
tar -C ${GIT_TOP} --exclude .git -cf - imp | tar -xf -

# Get date and revision-specific install directories
SORTDATE=`date -u "+%Y%m%d"`
DATE=`date -u +'%Y/%m/%d'`
IMPINSTALL=${IMPTOP}/${SORTDATE}-${shortrev}
# Make sure VERSION is acceptable as a version number (no spaces or /, -)
IMPVERSION="`cat imp/VERSION | sed -e 's/[ /-]/./g'`"
if [ ${BRANCH} = "develop" ]; then
  # For nightly builds, prepend the date so the packages are upgradeable
  IMPVERSION="${SORTDATE}.${IMPVERSION}"
fi
IMPSRCTGZ=${IMPINSTALL}/build/sources/imp-${IMPVERSION}.tar.gz
rm -rf ${IMPINSTALL}
mkdir -p ${IMPINSTALL}/build/sources ${IMPINSTALL}/build/logs

# Make link so build system can find the install location
rm -f ${IMPTOP}/.SVN-new
ln -s ${IMPINSTALL} ${IMPTOP}/.SVN-new

# Add build date to nightly docs
if [ ${BRANCH} = "develop" ]; then
  IMPVER="`sed -e 's/ /:/g' < imp/VERSION`"
  (cd imp/tools/build/doxygen_templates && sed -e "s#^PROJECT_NUMBER.*#PROJECT_NUMBER = ${IMPVER}, ${DATE}#" < Doxyfile.in > .dox && mv .dox Doxyfile.in)
fi

# Write out version files
verfile="${IMPINSTALL}/build/imp-version"
revfile="${IMPINSTALL}/build/imp-gitrev"
mkdir -p "${IMPINSTALL}/build"
echo "${IMPVERSION}" > $verfile
echo "${rev}" > $revfile

# Write out log from previous build to this one
logfile="${IMPINSTALL}/build/imp-gitlog"
if [ -n "${oldrev}" ]; then
  git log ${oldrev}..${rev} --format="%H%x00%an%x00%ae%x00%s" > ${logfile}
fi

# Write out list of all components
compfile="${IMPINSTALL}/build/imp-components"
python <<END
import sys
sys.path.insert(0, 'imp/tools/build')
import tools

f = open('$compfile', 'w')
for m in tools.compute_sorted_order('imp', ''):
    print >> f, "module\t" + m
for a in tools.get_applications('imp'):
    print >> f, "application\t" + a[0]
END

# Write out a tarball:
mv imp imp-${IMPVERSION} && tar -czf ${IMPSRCTGZ} imp-${IMPVERSION}

# Cleanup
cd /
rm -rf ${TMPDIR}
