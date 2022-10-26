#!/usr/bin/sh

vc7="Visual Studio .NET 2003" 
vc8="Visual Studio 8"
csc8="C:/WINDOWS/Microsoft.NET/Framework/v3.5"
vc9="Visual Studio 9.0"
rc8="C:/Program Files/Microsoft SDKs/Windows/v6.0A"
rc9="C:/Program Files/Microsoft SDKs/Windows/v7.0"
tpublic="\\\\bluejungle.com\\share\\public"

if [ $# -ne 1 ] ; then
  echo
  echo "Usage: $0, Enforcer_directory (in <DRIVE>: format)"
  echo "     eg. $0 l:/p4/2.0/Enforcers"
  echo
else
  enforcer=$1;
  export NLENFORCERSDIR=$enforcer;
  export NLBUILDROOT=$enforcer;
  export NLPLATFORMSDIR=$NLENFORCERSDIR/platforms
  if [ $EXTERNALSDIR ]; then
	  echo
	  echo "Externals directory already set to," $EXTERNALSDIR ", not resetting."
	  echo
  else
	  export EXTERNALSDIR=$NLENFORCERSDIR/../external
  fi;

  export JAVAROOT=`find "$enforcer/.." -maxdepth 2 -type d -name "main"`
  echo $JAVAROOT
  alias cde='cd $NLENFORCERSDIR'
  alias cdp='cd $NLPLATFORMSDIR'
  # Note ANT_HOME needs to be in DRIVE letter, but PATH for ant needs to be in /cygdrive
  export ANT_HOME=t:/bintool/buildtool/CruiseControl/apache-ant-1.7.0/;
  export PATH="/cygdrive/t/bintool/buildtool/CruiseControl/apache-ant-1.7.0/bin:$PATH";

  # Adding to the PATH
  prod=`ls $NLENFORCERSDIR/prod`
  for d in $prod ; do
    if [ "`stat -c %F $NLENFORCERSDIR/prod/$d`" == "directory" ]; then
      d=`cygpath -u "$NLENFORCERSDIR/prod/$d/build.output/lib_win32"`
      echo "Adding Lib path $d"
      export PATH="$d:$PATH"
    fi
  done
  echo

  # Trying to map the W: to the Visual Studio 7 ...
  drive=`subst | grep "$vc7" `; 
  drive=`echo $drive | cut -c1` 
  if [ .$drive == ".W" ] ; then 
    echo "You seem to have W drive for your Visual Studio 7, so it should be fine"
  else
    subst /d w: > /dev/null

    # Searching the environment VS71COMNTOOLS to see if we have some clue
    found=0;

    p=`echo $VS71COMNTOOLS | sed "s/\\\\\\Common7.*$//"`
    if [ -n "$p" ]; then
      echo "Creating W: drive for you"
      subst w: "$p";
      found=1;
      subst | grep "$vc7"
    fi

    if [ $found -eq 0 ]; then
      echo "I cannot find your Visual Studio 7 path."
      echo "Please try to map your W: drive to Visual Studio 7"
      echo "   e.g.  subst w: \"f:\Program Files\Microsoft Visual Studio .NET 2003\""
    fi
  fi 

  echo

  # Trying to map the V: to the Visual Studio 8 ...
  drive=`subst | grep "$vc8" `; 
  drive=`echo $drive | cut -c1` 
  if [ .$drive == ".V" ] ; then 
    echo "You seem to have V drive for your Visual Studio 8, so it should be fine"
  else
    subst /d v: > /dev/null

    # Searching the environment VS80COMNTOOLS to see if we have some clue
    found=0;

    p=`echo $VS80COMNTOOLS | sed "s/\\\\\\Common7.*$//"`
    if [ -n "$p" ]; then
      echo "Creating V: drive for you"
      subst v: "$p";
      found=1;
      subst | grep "$vc8"
    fi

    if [ $found -eq 0 ]; then
      echo "I cannot find your Visual Studio 8 path."
      echo "Please try to map your V: drive to Visual Studio 8"
      echo "   e.g.  subst v: \"f:\Program Files\Microsoft Visual Studio 8\""
    fi
  fi

  echo

  # Trying to map the X: to the Visual Studio 9 ...
  drive=`subst | grep "$vc9" `; 
  drive=`echo $drive | cut -c1` 
  if [ .$drive == ".X" ] ; then 
    echo "You seem to have X drive for your Visual Studio 9, so it should be fine"
  else
    subst /d x: > /dev/null

    # Searching the environment VS90COMNTOOLS to see if we have some clue
    found=0;

    p=`echo $VS90COMNTOOLS | sed "s/\\\\\\Common7.*$//"`
    if [ -n "$p" ]; then
      echo "Creating X: drive for you"
      subst x: "$p";
      found=1;
      subst | grep "$vc9"
    fi

    if [ $found -eq 0 ]; then
      echo "I cannot find your Visual Studio 9 path."
      echo "Please try to map your X: drive to Visual Studio 9"
      echo "   e.g.  subst x: \"f:\Program Files\Microsoft Visual Studio 9\""
    fi
  fi

  #Trying to map Y: to CSC compiler ...
  drive=`subst | grep "v3.5"`;
  drive=`echo $drive | cut -c1`
  if [ .$drive == ".Y" ] ; then 
     echo "You seem to have Y drive for your Visual Studio 8 CSC compiler, so it should be fine"
  else
    subst /d Y: > /dev/null
    echo "Creating Y: drive for you"
    subst Y: "$csc8";
    echo 
    subst | grep "v3.5"
  fi

  #Trying to map R: to Visual Studio Rc.exe compiler ...
  drive=`subst | grep "v6.0A"`;
  drive=`echo $drive | cut -c1`
  if [ .$drive == ".R" ] ; then
     echo "You seem to have R drive for your Visual Studio 8 RC.exe, so it should be fine"
  else
    subst /d R: > /dev/null
    echo "Creating R: drive for you"
    subst R: "$rc8";
    echo
    subst | grep "v6.0A"
  fi

  #Trying to map O: to Visual Studio Rc.exe compiler for v7 Platform SDK...
  drive=`subst | grep "v7.0"`;
  drive=`echo $drive | cut -c1`
  if [ .$drive == ".O" ] ; then
     echo "You seem to have O drive for your Platform 7 SDK RC.exe already defined"
  else
    subst /d O: > /dev/null
    echo "Creating O: drive for you"
    subst O: "$rc9";
    echo
    subst | grep "v7.0"
  fi
 
  #Trying to map t: to \\bluejungle.com\share\public for bintool\winddk...
  drive=`subst | grep "public"`;
  drive=`echo $drive | cut -c1`
  if [ .$drive == ".T" ] ; then
     echo "You seem to have T drive for your \\bluejungle.com\share\public defined"
  else
    subst /d T: > /dev/null
    echo "Creating T: drive for you"
    subst T: "$tpublic";
    echo
    subst | grep "public"
  fi

  # Add to path location of VS8 IDE runtime to allow command line build
  export PATH="$PATH:/cygdrive/x/Common7/IDE"
fi


 #Trying to map j: to CSC compiler ...
  drive=`subst | grep "nxt-bclient01"`;
  drive=`echo $drive | cut -c1`
  if [ .$drive == ".J" ] ; then 
     echo "You seem to have J drive"
  else
    subst /d J: > /dev/null
    echo "Creating J: drive for you"
#    subst J: "\\\\nxt-bclient01\\release\\1104-2-5.5.1\\edp\\bin";
    subst J: "\\\\nxt-bclient01\\hudson-jobs";
    echo 
    subst | grep "nxt-bclient01"
  fi
