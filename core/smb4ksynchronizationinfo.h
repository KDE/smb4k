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

#ifndef SMB4KSYNCHRONIZATIONINFO_H
#define SMB4KSYNCHRONIZATIONINFO_H

// Qt includes
#include <QString>

// KDE includes
#include <kdemacros.h>

class KDE_EXPORT Smb4KSynchronizationInfo
{
  public:
    /**
     * The constructor. It takes no arguments and you should use the
     * setXYZ() functions to set the necessary values.
     */
    Smb4KSynchronizationInfo();

    /**
     * The copy constructor.
     *
     * @param info        The synchronization info that is to be copied
     */
    Smb4KSynchronizationInfo( const Smb4KSynchronizationInfo &info );

    /**
     * The destructor.
     */
    ~Smb4KSynchronizationInfo();

    /**
     * Set the path to the source directory of the synchronization.
     *
     * @param source      The path to the source directory.
     */
    void setSourcePath( const QString &source );

    /**
     * Return the path to the source directory of the synchronization.
     *
     * @returns the path to the source directory,
     */
    const QString &sourcePath() const { return m_source; }

    /**
     * Set the path to the destination directory of the synchronization.
     *
     * @param destination The path to the destination directory.
     */
    void setDestinationPath( const QString &destination );

    /**
     * Return the path to the destination directory of the synchronization.
     *
     * @returns the path to the destination directory.
     */
    const QString &destinationPath() const { return m_destination; }

    /**
     * Set the text that's provided in rsync's output. This may either be a
     * file name or some information about the progress
     *
     * @param text        The text
     */
    void setText( const QString &text );

    /**
     * Return the name of the file that is currently processed. This may
     * be empty if no information about the file name is available.
     *
     * @returns the name of the file that is currently processed or an empty
     * string
     */
    const QString &text () const { return m_text; }

    /**
     * Set the progress of the file that's currently processed.
     *
     * @param percent     The progress in percent
     */
    void setCurrentProgress( int percent );

    /**
     * Return the progress of the current file transfer. If no
     * information is available, -1 is returned.
     *
     * @returns the progress of the current file transfer or -1.
     */
    int currentProgress() const { return m_current_progress; }

    /**
     * Set the total progress of synchronization process.
     *
     * @param percent     The progress in percent
     */
    void setTotalProgress( int percent );

    /**
     * Return the total progress of synchronization process. If no
     * information is available, -1 is returned.
     *
     * @returns the total progress of the synchronization or -1.
     */
    int totalProgress() const { return m_total_progress; }

    /**
     * Set the total number of files that have been considered for the
     * synchronization.
     *
     * @param total       The total number of files
     */
    void setTotalFileNumber( int total );

    /**
     * Return the total number of files that were considered for synchronization.
     * If no information is available, -1 is returned.
     *
     * @returns the total number of files or -1.
     */
    int totalFileNumber() const { return m_total_files; }

    /**
     * Set the number of files that have already been processed during the
     * synchronization.
     *
     * @param processed   The number of files that have been processed
     */
    void setProcessedFileNumber( int processed );

    /**
     * Return the number of files that have already been processed during the
     * synchronization. If no information is available, -1 is returned.
     *
     * @returns the number of processed files or -1.
     */
    int processedFileNumber() const { return m_processed_files; }

    /**
     * Set the transfer rate. This should be a string that already contains
     * all information, i.e. the string should look like this: 100 kB/s.
     *
     * @param rate        The rate string (e.g. 100 kB/s)
     */
    void setTransferRate( const QString &rate );

    /**
     * Return the transfer rate. This is a string that already contains all
     * information, so that you can just put it into your widget without any
     * modification. It may also be empty if no information about the rate is
     * available.
     *
     * @returns The rate or an empty string.
     */
    const QString &transferRate() const { return m_rate; }

    /**
     * Returns TRUE if @p info has equal entries as this object. Otherwise FALSE
     * is returned.
     *
     * @returns TRUE is @p info equals this object.
     */
    bool equals( Smb4KSynchronizationInfo *info ) const;
    
    /**
     * Operator to check if two infos are equal.
     */
    bool operator==( Smb4KSynchronizationInfo info ) { return equals( &info ); }

  private:
    /**
     * Source path
     */
    QString m_source;

    /**
     * Destination path
     */
    QString m_destination;

    /**
     * The text
     */
    QString m_text;

    /**
     * The individual progress
     */
    int m_current_progress;

    /**
     * The total progress
     */
    int m_total_progress;

    /**
     * The total file number
     */
    int m_total_files;

    /**
     * The number of processed files
     */
    int m_processed_files;

    /**
     * The rate string
     */
    QString m_rate;
};

#endif
