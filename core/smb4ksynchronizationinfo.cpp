/***************************************************************************
    smb4ksynchronizationinfo  -  This is a container that holds
    information about progress of the synchronization
                             -------------------
    begin                : So Mai 20 2007
    copyright            : (C) 2007 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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

// application specific includes
#include "smb4ksynchronizationinfo.h"


Smb4KSynchronizationInfo::Smb4KSynchronizationInfo()
: m_source( QString() ), m_destination( QString() ), m_text( QString() ),
  m_current_progress( -1 ), m_total_progress( -1 ), m_total_files( -1 ),
  m_processed_files( -1 ), m_rate( QString() )
{
}


Smb4KSynchronizationInfo::Smb4KSynchronizationInfo( const Smb4KSynchronizationInfo &info )
: m_source( info.sourcePath() ), m_destination( info.destinationPath() ), m_text( info.text() ),
  m_current_progress( info.currentProgress() ), m_total_progress( info.totalProgress() ),
  m_total_files( info.totalFileNumber() ), m_processed_files( info.processedFileNumber() ),
  m_rate( info.transferRate() )
{
}


Smb4KSynchronizationInfo::~Smb4KSynchronizationInfo()
{
}


void Smb4KSynchronizationInfo::setSourcePath( const QString &source )
{
  m_source = source;
}


void Smb4KSynchronizationInfo::setDestinationPath( const QString &destination )
{
  m_destination = destination;
}


void Smb4KSynchronizationInfo::setText( const QString &text )
{
  m_text = text;
}


void Smb4KSynchronizationInfo::setCurrentProgress( int percent )
{
  m_current_progress = percent;
}


void Smb4KSynchronizationInfo::setTotalProgress( int percent )
{
  m_total_progress = percent;
}


void Smb4KSynchronizationInfo::setTotalFileNumber( int total )
{
  m_total_files = total;
}


void Smb4KSynchronizationInfo::setProcessedFileNumber( int processed )
{
  m_processed_files = processed;
}


void Smb4KSynchronizationInfo::setTransferRate( const QString &rate )
{
  m_rate = rate;
}


bool Smb4KSynchronizationInfo::equals( Smb4KSynchronizationInfo *info ) const
{
  if ( QString::compare( m_source, info->sourcePath() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_destination, info->destinationPath() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_text, info->text() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_current_progress != info->currentProgress() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_total_progress != info->totalProgress() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_total_files != info->totalFileNumber() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_processed_files != info->processedFileNumber() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_rate != info->transferRate() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  return true;
}
