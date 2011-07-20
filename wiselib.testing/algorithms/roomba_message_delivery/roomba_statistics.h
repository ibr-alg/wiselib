/*
    Provides statistics output for roomba robot. Statistics
    should be computed at the event of received messages.
    Copyright (C) 2010  Andreas Cord-Landwehr

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef ROOMBA_STATISTICS_H
#define ROOMBA_STATISTICS_H

#include <string.h>
#include <fstream>
#include <iostream>
#include <boost/concept_check.hpp>

class RoombaStatistics
{

public:
    RoombaStatistics( std::string statistics_filename = "output.txt" )
	{
		set_statistics_filename( statistics_filename );
		statistics_stream_.open( statistics_filename_.c_str() );
		statistics_stream_ << "Time;Buffer Size;Action\n";
		statistics_stream_.flush();
	}

    virtual ~RoombaStatistics()
    {}

	/** \brief prints statistics data to statistics file
	 *
	 * \param int time is the time point the event occurs, by local time
	 * \param int buffer_size is the current size of the buffer
	 * \param int action how the number of messages changed (+1,0,-1)
	 * \return void
	 */
	void print_statistics( int time, int buffer_size, int action )
	{
		statistics_stream_ << time << ";" << buffer_size << ";" << action << "\n";
		statistics_stream_.flush();
	}

	/** \brief print comment to statistics file
	 * \param comment is the to be printed comment
	 * \return void
	 */
	void print_statistics_comment(std::string comment)
	{
		statistics_stream_ << "# " << comment << "\n";
		statistics_stream_.flush();
	}

	/**
	 * Closes the stream
	 */
	void close_statistics()
	{
		statistics_stream_.close();
	}

	/*
	 * Setter and getter methods
	 */
    void set_statistics_filename( std::string statistics_filename )
    {
		statistics_filename_ = statistics_filename;
    }

    std::string statistics_filename( )
    {
		return statistics_filename_;
	}

private:
	std::string statistics_filename_;
	std::ofstream statistics_stream_;
};

#endif // ROOMBA_STATISTICS_H
