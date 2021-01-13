#!/bin/bash
if [[ "$SHELLOPTS" =~ "nounset" ]]; then
  _WAS_NOUNSET=1
else
  _WAS_NOUNSET=0
fi
function log_error() {
    echo -e "$(tput setaf 1 2>/dev/null)$1$(tput sgr0 2>/dev/null)"
}
function log_success() {
    echo -e "$(tput setaf 2 2>/dev/null)$1$(tput sgr0 2>/dev/null)"
}
function log_warn() {
    echo -e "$(tput setaf 3 2>/dev/null)$1$(tput sgr0 2>/dev/null)"
}
function log_info() {
    echo -e "$(tput setaf 4 2>/dev/null)$1$(tput sgr0 2>/dev/null)"
}

function link_if_necessary() {
  thedir=$1
  thelinkname=$2
  if [[ "$thedir" != "$thelinkname" ]]; then
    if [[ -L $thelinkname ]]; then
      if [[ "$(readlink -- "$thelinkname")" != $thedir ]]; then
        echo "$thelinkname is an existing link, but doesn't point to $thedir. Aborting."
        exit 1
      fi
    elif [[ -e $thelinkname ]]; then
      echo "$thelinkname exists, but is not a symlink and we want the destination to be $thedir."
    else
        ln -sv "$thedir" "$thelinkname"
    fi
  fi
}

function setgid_all() {
    # n.b. can't be recursive because g+s on files means something else
    # so we find all directories and individually chmod them:
    if [[ "$EUID" != 0 ]]; then
      find $1 -type d -exec sudo chmod g+rxs {} \;
    else
      find $1 -type d -exec chmod g+rxs {} \;
    fi
}

function _cached_fetch() {
  url=$1
  filename=$2
  dest=$PWD
  if [[ $filename == *levmar* ]]; then
    EXTRA_CURL_OPTS='-A "Mozilla/5.0"'
  else
    EXTRA_CURL_OPTS=''
  fi
  mkdir -p /opt/MagAOX/.cache
  if [[ ! -e $dest/$filename ]]; then
    if [[ ! -e /opt/MagAOX/.cache/$filename ]]; then
      curl $EXTRA_CURL_OPTS -L $url > /tmp/magaoxcache-$filename && \
        mv /tmp/magaoxcache-$filename /opt/MagAOX/.cache/$filename
      log_info "Downloaded to /opt/MagAOX/.cache/$filename"
    fi
    cp /opt/MagAOX/.cache/$filename $dest/$filename
    log_info "Copied to $dest/$filename"
  fi
}

if [[ -e /vagrant/windows_host.txt ]]; then
  VM_WINDOWS_HOST=1
else
  VM_WINDOWS_HOST=0
fi

function clone_or_update_and_cd() {
    orgname=$1
    reponame=$2
    parentdir=$3
    # Disable unbound var check because we do it ourselves:
    if [[ "$SHELLOPTS" =~ "nounset" ]]; then _WAS_NOUNSET=1; set +u; fi
    if [[ ! -z $4 ]]; then
      destdir=$4
    else
      destdir="$parentdir/$reponame"
    fi
    if [[ $_WAS_NOUNSET == 1 ]]; then set -u; fi
    # and re-enable.

    if [[ $MAGAOX_ROLE == vm && $VM_WINDOWS_HOST == 0 ]]; then
        mkdir -p /vagrant/vm/$reponame
        link_if_necessary /vagrant/vm/$reponame $destdir
    fi
    if [[ ! -d $parentdir/$reponame/.git ]]; then
      echo "Cloning new copy of $orgname/$reponame"
      CLONE_DEST=/tmp/${reponame}_$(date +"%s")
      git clone https://github.com/$orgname/$reponame.git $CLONE_DEST
      sudo rsync -av $CLONE_DEST/ $destdir/
      cd $destdir/
      log_success "Cloned new $destdir"
      rm -rf $CLONE_DEST
      log_success "Removed temporary clone at $CLONE_DEST"
    else
      cd $destdir
      git pull
      log_success "Updated $destdir"
    fi
    git config core.sharedRepository group
    if [[ $MAGAOX_ROLE != vm ]]; then
      sudo chown -R :magaox-dev $destdir
      sudo chmod -R g=rwX $destdir
      # n.b. can't be recursive because g+s on files means something else
      # so we find all directories and individually chmod them:
      sudo find $destdir -type d -exec chmod g+s {} \;
      log_success "Normalized permissions on $destdir"
    fi
}

DEFAULT_PASSWORD="extremeAO!"

function creategroup() {
  if [[ ! $(getent group $1) ]]; then
    sudo groupadd $1
    echo "Added group $1"
  else
    echo "Group $1 exists"
  fi
}

function createuser() {
  if getent passwd $1 > /dev/null 2>&1; then
    log_info "User account $1 exists"
  else
    sudo useradd -U $1
    echo -e "$DEFAULT_PASSWORD\n$DEFAULT_PASSWORD" | sudo passwd $1
    log_success "Created user account $1 with default password $DEFAULT_PASSWORD"
  fi
  sudo usermod -a -G magaox $1
  log_info "Added user $1 to group magaox"
  sudo mkdir -p /home/$1/.ssh
  sudo touch /home/$1/.ssh/authorized_keys
  sudo chmod -R u=rwx,g=,o= /home/$1/.ssh
  sudo chmod u=rw,g=,o= /home/$1/.ssh/authorized_keys
  sudo chown -R $1:magaox /home/$1
  sudo chsh $1 -s $(which bash)
  log_info "Append an ecdsa or ed25519 key to /home/$1/.ssh/authorized_keys to enable SSH login"
}
# We work around the buggy devtoolset /bin/sudo wrapper in provision.sh, but
# that means we have to explicitly enable it ourselves.
# (This crap again: https://bugzilla.redhat.com/show_bug.cgi?id=1319936)
# Also, we control whether to build with the devtoolset gcc7 compiler or the
# included gcc4 compiler with $BUILDING_KERNEL_STUFF. If that's set to 1,
# use the included gcc4 for consistency with the running kernel.
if [[ "$SHELLOPTS" =~ "nounset" ]]; then _WAS_NOUNSET=1; set +u; fi # Temporarily disable checking for unbound variables to set a default value
  if [[ -z $BUILDING_KERNEL_STUFF ]]; then BUILDING_KERNEL_STUFF=0; fi
if [[ $_WAS_NOUNSET == 1 ]]; then set -u; fi

if [[ $BUILDING_KERNEL_STUFF != 1 && -e /opt/rh/devtoolset-7/enable ]]; then
  if [[ "$SHELLOPTS" =~ "nounset" ]]; then _WAS_NOUNSET=1; set +u; fi
    source /opt/rh/devtoolset-7/enable
  if [[ $_WAS_NOUNSET == 1 ]]; then set -u; fi
fi
# root doesn't get /usr/local/bin on their path, so add it
# (why? https://serverfault.com/a/838552)
if [[ $PATH != "*/usr/local/bin*" ]]; then
    export PATH="/usr/local/bin:$PATH"
fi
if [[ $(which sudo) == *devtoolset* ]]; then
  REAL_SUDO=/usr/bin/sudo
else
  REAL_SUDO=$(which sudo)
fi
