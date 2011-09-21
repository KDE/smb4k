/***************************************************************************
    smb4kmounthelper  -  The helper that mounts and unmounts shares.
                             -------------------
    begin                : Sa Okt 16 2010
    copyright            : (C) 2010 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QProcessEnvironment>
#include <QUrl>
#include <QDebug>

// KDE includes
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmountpoint.h>

// application specific includes
#include "smb4kmounthelper.h"

KDE4_AUTH_HELPER_MAIN( "de.berlios.smb4k.mounthelper", Smb4KMountHelper )


ActionReply Smb4KMountHelper::mount( const QVariantMap &args )
{
  ActionReply reply;
  reply.addData( "share_url", args["share_url"] );
  reply.addData( "share_workgroup", args["share_workgroup"] );
  reply.addData( "share_comment", args["share_comment"] );
  reply.addData( "share_ip", args["share_ip"] );
  reply.addData( "share_mountpoint", args["share_mountpoint"] );
  reply.addData( "key", args["key"] );

  KProcess proc( this );
  proc.setOutputChannelMode( KProcess::SeparateChannels );
  proc.setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
  proc.setEnv( "PASSWD", args["share_url"].toUrl().password(), true );
  // The $HOME environment variable is needed under FreeBSD to
  // point the mount process to the right ~/.nsmbrc file. Under
  // Linux it is not needed.
  proc.setEnv( "HOME", args["home_dir"].toString() );
  proc.setProgram( args["mount_command"].toStringList() );

  // Run the mount process.
  proc.start();

  if ( proc.waitForStarted( -1 ) )
  {
    // We want to be able to terminate the process from outside.
    // Thus, we implement a loop that checks periodically, if we
    // need to kill the process.
    bool user_kill = false;

    while ( !proc.waitForFinished( 10 ) )
    {
      if ( HelperSupport::isStopped() )
      {
        proc.kill();
        user_kill = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }

    if ( proc.exitStatus() == KProcess::CrashExit )
    {
      if ( !user_kill )
      {
        reply.setErrorCode( ActionReply::HelperError );
        reply.setErrorDescription( i18n( "The mount process crashed." ) );
        return reply;
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Check if there is output on stderr.
      QString stderr = QString::fromUtf8( proc.readAllStandardError() );
      reply.addData( "stderr", stderr );

      // Check if there is output on stdout.
      QString stdout = QString::fromUtf8( proc.readAllStandardOutput() );
      reply.addData( "stdout", stdout );
    }
  }
  else
  {
    reply.setErrorCode( ActionReply::HelperError );
    reply.setErrorDescription( i18n( "The mount process could not be started." ) );
    return reply;
  }

  return reply;
}



ActionReply Smb4KMountHelper::unmount( const QVariantMap &args )
{
  ActionReply reply;
  reply.addData( "share_url", args["share_url"] );
  reply.addData( "share_mountpoint", args["share_mountpoint"] );
  reply.addData( "key", args["key"] );

  // Check if the mountpoint is valid and the file system is
  // also correct.
  bool mountpoint_ok = false;
  KMountPoint::List mountpoints = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );

  for( int j = 0; j < mountpoints.size(); j++ )
  {
#ifndef Q_OS_FREEBSD
    if ( QString::compare( args["share_mountpoint"].toString(),
                           mountpoints.at( j )->mountPoint(), Qt::CaseInsensitive ) == 0 &&
         QString::compare( mountpoints.at( j )->mountType(), "cifs", Qt::CaseInsensitive ) == 0 )
#else
    if ( QString::compare( args["share_mountpoint"].toString(),
                           mountpoints.at( j )->mountPoint(), Qt::CaseInsensitive ) == 0 &&
         QString::compare( mountpoints.at( j )->mountType(), "smbfs", Qt::CaseInsensitive ) == 0 )
#endif
    {
      mountpoint_ok = true;
      break;
    }
    else
    {
      continue;
    }
  }

  if ( !mountpoint_ok )
  {
    reply.setErrorCode( ActionReply::HelperError );
    reply.setErrorDescription( i18n( "The mountpoint is invalid." ) );
    return reply;
  }
  else
  {
    // Do nothing
  }

  KProcess proc( this );
  proc.setOutputChannelMode( KProcess::SeparateChannels );
  proc.setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
  proc.setProgram( args["umount_command"].toStringList() );

  // Run the unmount process.
  proc.start();

  if ( proc.waitForStarted( -1 ) )
  {
    // We want to be able to terminate the process from outside.
    // Thus, we implement a loop that checks periodically, if we
    // need to kill the process.
    bool user_kill = false;

    while ( !proc.waitForFinished( 10 ) )
    {
      if ( HelperSupport::isStopped() )
      {
        proc.kill();
        user_kill = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }

    if ( proc.exitStatus() == KProcess::CrashExit )
    {
      if ( !user_kill )
      {
        reply.setErrorCode( ActionReply::HelperError );
        reply.setErrorDescription( i18n( "The unmount process crashed." ) );
        return reply;
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Check if there is output on stderr.
      QString stderr = QString::fromUtf8( proc.readAllStandardError() );
      reply.addData( "stderr", stderr );

      // Check if there is output on stdout.
      QString stdout = QString::fromUtf8( proc.readAllStandardOutput() );
      reply.addData( "stdout", stdout );
    }
  }
  else
  {
    reply.setErrorCode( ActionReply::HelperError );
    reply.setErrorDescription( i18n( "The unmount process could not be started." ) );
    return reply;
  }

  return reply;
}

#include "smb4kmounthelper.moc"
